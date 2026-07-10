package dwvc

// xfer.go — XferStream and the xfer_in/xfer_out subsystem.
//
// Implements serialization/deserialization of Vc values using the exact
// wire format produced by the C++ vcxstream/vc code.
//
// Wire format summary (all numbers are ASCII):
//   Every Vc on the wire: [type_tag(2)][payload...]
//
//   VCNil(04):    no payload
//   VCInt(01):    [len_digits_count(2)][signed_digits]
//   VCDouble(03): [string_len_count(2)][float_g_string]
//   VCString(02): [len_digits_count(2)][string_bytes]
//   VCCHit(17):   [chit_index_encoded_as_long]
//
//   Decomposables (vector=09, map=08, set=07, bag=11, list=10, tree=31):
//     [num_elems_encoded][elem0][elem1]...
//     Map/Tree: interleaved key, value, key, value...

import (
	"math"
	"reflect"
	"strconv"
)

// XferStream manages buffered I/O for vc serialization.
// Analog of the C++ vcxstream class.
type XferStream struct {
	// Read buffer — entire input is loaded for FIXED-style access.
	rbuf []byte
	rpos int

	// Write buffer — grows as needed.
	wbuf []byte
	wpos int

	// Chit table for DAG/cycle support.
	// Non-atomic objects are registered here before their contents
	// are read/written, allowing forward references.
	chitTable []vcRep

	// visited tracks objects during serialization.
	// Maps rep pointer identity → chit index.
	visited map[uintptr]int

	// Constraints for deserialization hardening.
	MaxMemory     int64
	MaxDepth      int
	MaxElements   int
	MaxElementLen int

	// Runtime tracking
	memoryTally int64
	depthLimit  int // countdown counter matching C++ max_depth

	allowSelfRef bool
}

// NewXferStreamForRead creates a stream for deserialization from a byte slice.
func NewXferStreamForRead(data []byte) *XferStream {
	return &XferStream{
		rbuf:          data,
		MaxMemory:     math.MaxInt64,
		MaxDepth:      math.MaxInt64,
		MaxElements:   math.MaxInt64,
		MaxElementLen: math.MaxInt64,
		depthLimit:    math.MaxInt64,
		allowSelfRef:  true,
	}
}

// NewXferStreamForWrite creates a stream for serialization.
func NewXferStreamForWrite() *XferStream {
	return &XferStream{
		wbuf:          make([]byte, 2048),
		MaxMemory:     math.MaxInt64,
		MaxDepth:      math.MaxInt64,
		MaxElements:   math.MaxInt64,
		MaxElementLen: math.MaxInt64,
		allowSelfRef:  true,
	}
}

// SetConstraints configures deserialization limits.
func (xs *XferStream) SetConstraints(maxMem, maxDepth, maxElems, maxElemLen int64) {
	if maxMem > 0 {
		xs.MaxMemory = maxMem
	}
	if maxDepth > 0 {
		xs.MaxDepth = int(maxDepth)
		xs.depthLimit = int(maxDepth)
	}
	if maxElems > 0 {
		xs.MaxElements = int(maxElems)
	}
	if maxElemLen > 0 {
		xs.MaxElementLen = int(maxElemLen)
	}
}

// WBuf returns the written bytes.
func (xs *XferStream) WBuf() []byte {
	return xs.wbuf[:xs.wpos]
}

// --- Buffer management ---

func (xs *XferStream) outWant(n int) ([]byte, error) {
	if n <= 0 {
		return nil, nil
	}
	if xs.wpos+n > len(xs.wbuf) {
		newLen := xs.wpos + n
		if newLen < len(xs.wbuf)*2 {
			newLen = len(xs.wbuf) * 2
		}
		newBuf := make([]byte, newLen)
		copy(newBuf, xs.wbuf[:xs.wpos])
		xs.wbuf = newBuf
	}
	ret := xs.wbuf[xs.wpos : xs.wpos+n]
	xs.wpos += n
	return ret, nil
}

func (xs *XferStream) inWant(n int) ([]byte, error) {
	if n <= 0 {
		return nil, nil
	}
	if xs.rpos+n > len(xs.rbuf) {
		return nil, ErrDev
	}
	ret := xs.rbuf[xs.rpos : xs.rpos+n]
	xs.rpos += n
	return ret, nil
}

// --- Chit table ---

func repID(r vcRep) uintptr {
	return reflect.ValueOf(r).Pointer()
}

func (xs *XferStream) chitAppend(r vcRep) int {
	idx := len(xs.chitTable)
	xs.chitTable = append(xs.chitTable, r)
	return idx
}

func (xs *XferStream) chitGet(idx int) (vcRep, error) {
	if idx < 0 || idx >= len(xs.chitTable) {
		return nil, ErrParse
	}
	return xs.chitTable[idx], nil
}

func (xs *XferStream) markVisited(r vcRep) {
	if xs.visited == nil {
		xs.visited = make(map[uintptr]int)
	}
	xs.visited[repID(r)] = len(xs.chitTable)
}

// --- Top-level Vc serialization ---

// XferOut serializes a Vc to the stream. Returns bytes written.
// Matches C++ vc::xfer_out.
func (v *Vc) XferOut(xs *XferStream) (int, error) {
	if v.rep == nil {
		buf, err := xs.outWant(EncodedTypeLen)
		if err != nil {
			return 0, ErrDev
		}
		EncodeType(buf, VCNil)
		return EncodedTypeLen, nil
	}

	id := repID(v.rep)
	if idx, ok := xs.visited[id]; ok {
		return xs.emitCHit(idx)
	}

	xs.markVisited(v.rep)

	t := v.rep.vcType()
	buf, err := xs.outWant(EncodedTypeLen)
	if err != nil {
		return 0, ErrDev
	}
	EncodeType(buf, t)

	if !v.rep.isAtomic() {
		xs.chitAppend(v.rep)
	}

	n, err := v.rep.xferOut(xs)
	if err != nil {
		return 0, err
	}
	return EncodedTypeLen + n, nil
}

func (xs *XferStream) emitCHit(idx int) (int, error) {
	if !xs.allowSelfRef {
		return 0, ErrParse
	}
	buf, err := xs.outWant(EncodedTypeLen)
	if err != nil {
		return 0, ErrDev
	}
	EncodeType(buf, VCCHit)

	idxBytes, idxLen := EncodeLong(int64(idx))
	idxBuf, err := xs.outWant(idxLen)
	if err != nil {
		return 0, ErrDev
	}
	copy(idxBuf, idxBytes)

	return EncodedTypeLen + idxLen, nil
}

// XferIn deserializes a Vc from the stream. Returns Vc and bytes consumed.
// Matches C++ vc::xfer_in / vc::real_xfer_in.
func (xs *XferStream) XferIn() (Vc, int, error) {
	// Depth limit check matching C++ max_depth countdown.
	// max_depth starts at MaxDepth, decrements each call, error at -1.
	if xs.depthLimit == -1 {
		return VcNil(), 0, ErrDepth
	}
	xs.depthLimit--
	ret, n, err := xs.realXferIn()
	xs.depthLimit++

	if err != nil {
		return VcNil(), n, err
	}
	if n > 0 && xs.memoryTally >= xs.MaxMemory {
		return VcNil(), n, ErrOOM
	}
	return ret, n, nil
}

func (xs *XferStream) realXferIn() (Vc, int, error) {
	tp, err := xs.inWant(EncodedTypeLen)
	if err != nil {
		return VcNil(), 0, ErrDev
	}
	t, err := DecodeType(tp)
	if err != nil {
		return VcNil(), 0, ErrParse
	}

	total := EncodedTypeLen

	switch t {
	case VCNil:
		return VcNil(), total, nil

	case VCCHit:
		return xs.readCHit(total)

	case VCInt:
		v, n, err := vcIntXferIn(xs)
		return v, total + n, err

	case VCDouble:
		v, n, err := vcDoubleXferIn(xs)
		return v, total + n, err

	case VCString:
		v, n, err := vcStringXferIn(xs)
		return v, total + n, err

	case VCVector:
		rep := &vcVector{}
		xs.chitAppend(rep)
		elems, n, err := decomposeXferIn(xs)
		if err != nil {
			return VcNil(), total + n, err
		}
		rep.elems = elems
		return Vc{rep: rep}, total + n, nil

	case VCSet:
		rep := &vcSet{}
		xs.chitAppend(rep)
		elems, n, err := decomposeXferIn(xs)
		if err != nil {
			return VcNil(), total + n, err
		}
		rep.elems = elems
		return Vc{rep: rep}, total + n, nil

	case VCBag:
		rep := &vcBag{}
		xs.chitAppend(rep)
		elems, n, err := decomposeXferIn(xs)
		if err != nil {
			return VcNil(), total + n, err
		}
		rep.elems = elems
		return Vc{rep: rep}, total + n, nil

	case VCList:
		rep := &vcList{}
		xs.chitAppend(rep)
		elems, n, err := decomposeXferIn(xs)
		if err != nil {
			return VcNil(), total + n, err
		}
		rep.elems = elems
		return Vc{rep: rep}, total + n, nil

	case VCMap:
		rep := &vcMap{}
		xs.chitAppend(rep)
		kvs, n, err := mapXferIn(xs)
		if err != nil {
			return VcNil(), total + n, err
		}
		rep.kvs = kvs
		return Vc{rep: rep}, total + n, nil

	case VCTree:
		rep := &vcTree{}
		xs.chitAppend(rep)
		kvs, n, err := mapXferIn(xs)
		if err != nil {
			return VcNil(), total + n, err
		}
		rep.kvs = kvs
		return Vc{rep: rep}, total + n, nil

	default:
		return VcNil(), 0, ErrParse
	}
}

func (xs *XferStream) readCHit(total int) (Vc, int, error) {
	lp, err := xs.inWant(EncodedLenLen)
	if err != nil {
		return VcNil(), total, ErrDev
	}
	digitCount, err := DecodeLen(lp)
	if err != nil {
		return VcNil(), total, ErrParse
	}
	if digitCount <= 0 {
		return VcNil(), total, ErrParse
	}
	dp, err := xs.inWant(digitCount)
	if err != nil {
		return VcNil(), total, ErrDev
	}
	idx, err := DecodeLong(dp, digitCount)
	if err != nil {
		return VcNil(), total, ErrParse
	}
	r, err := xs.chitGet(int(idx))
	if err != nil {
		return VcNil(), total, ErrParse
	}
	total += EncodedLenLen + digitCount
	return Vc{rep: r}, total, nil
}

// --- Type-specific xfer functions ---

func vcIntXferOut(xs *XferStream, i int64) (int, error) {
	s := CompatLtoa(i)
	nchars := len(s)
	countField := pltoa(int64(nchars), 2)
	total := EncodedLenLen + nchars
	buf, err := xs.outWant(total)
	if err != nil {
		return 0, ErrDev
	}
	buf[0] = countField[0]
	buf[1] = countField[1]
	copy(buf[2:], s)
	return total, nil
}

func vcIntXferIn(xs *XferStream) (Vc, int, error) {
	lp, err := xs.inWant(EncodedLenLen)
	if err != nil {
		return VcNil(), 0, ErrDev
	}
	digitCount, err := DecodeLen(lp)
	if err != nil {
		return VcNil(), 0, ErrParse
	}
	if digitCount <= 0 || digitCount > xs.MaxElementLen {
		return VcNil(), 0, ErrElemLen
	}
	dp, err := xs.inWant(digitCount)
	if err != nil {
		return VcNil(), 0, ErrDev
	}
	val, err := DecodeLong2(dp, digitCount)
	if err != nil {
		return VcNil(), 0, ErrParse
	}
	n := EncodedLenLen + digitCount
	xs.memoryTally += int64(n)
	return VcInt(val), n, nil
}

func vcDoubleXferOut(xs *XferStream, d float64) (int, error) {
	s := EncodeDouble(d)
	// Use encode_long format for the float string length, matching C++ exactly.
	idxBytes, idxLen := EncodeLong(int64(len(s)))
	total := idxLen + len(s)
	buf, err := xs.outWant(total)
	if err != nil {
		return 0, ErrDev
	}
	copy(buf, idxBytes)
	copy(buf[idxLen:], s)
	return total, nil
}

func vcDoubleXferIn(xs *XferStream) (Vc, int, error) {
	// Read encode_long format: [2-digit count of digits][digits of flen]
	lp, err := xs.inWant(EncodedLenLen)
	if err != nil {
		return VcNil(), 0, ErrDev
	}
	digitCount, err := DecodeLen(lp)
	if err != nil {
		return VcNil(), 0, ErrParse
	}
	if digitCount <= 0 || digitCount > xs.MaxElementLen {
		return VcNil(), 0, ErrElemLen
	}
	dp, err := xs.inWant(digitCount)
	if err != nil {
		return VcNil(), 0, ErrDev
	}
	flen, err := DecodeLong(dp, digitCount)
	if err != nil {
		return VcNil(), 0, ErrParse
	}
	if flen <= 0 || flen > 100 || flen > int64(xs.MaxElementLen) {
		return VcNil(), 0, ErrElemLen
	}
	// Read the float string
	fp, err := xs.inWant(int(flen))
	if err != nil {
		return VcNil(), 0, ErrDev
	}
	val, err := strconv.ParseFloat(string(fp[:flen]), 64)
	if err != nil {
		return VcNil(), 0, ErrParse
	}
	n := EncodedLenLen + digitCount + int(flen)
	xs.memoryTally += int64(n)
	return VcDouble(val), n, nil
}

func vcStringXferOut(xs *XferStream, s string) (int, error) {
	slen := len(s)
	countField := pltoa(int64(slen), -1)
	nDigits := len(countField)
	prefixLen := EncodedLenLen + nDigits
	total := prefixLen + slen

	buf, err := xs.outWant(total)
	if err != nil {
		return 0, ErrDev
	}
	prefixCount := pltoa(int64(nDigits), 2)
	buf[0] = prefixCount[0]
	buf[1] = prefixCount[1]
	copy(buf[2:], countField)
	copy(buf[prefixLen:], s)
	return total, nil
}

func vcStringXferIn(xs *XferStream) (Vc, int, error) {
	lp, err := xs.inWant(EncodedLenLen)
	if err != nil {
		return VcNil(), 0, ErrDev
	}
	numDigits, err := DecodeLen(lp)
	if err != nil {
		return VcNil(), 0, ErrParse
	}
	if numDigits <= 0 || numDigits > xs.MaxElementLen {
		return VcNil(), 0, ErrElemLen
	}

	dp, err := xs.inWant(numDigits)
	if err != nil {
		return VcNil(), 0, ErrDev
	}
	slen, err := DecodeLong(dp, numDigits)
	if err != nil {
		return VcNil(), 0, ErrParse
	}
	if slen < 0 || int(slen) > xs.MaxElementLen {
		return VcNil(), 0, ErrElemLen
	}

	sp, err := xs.inWant(int(slen))
	if err != nil {
		return VcNil(), 0, ErrDev
	}
	s := string(sp)
	n := EncodedLenLen + numDigits + int(slen)
	xs.memoryTally += int64(slen)
	return VcString(s), n, nil
}

// --- Decomposable helpers ---

func encodeNumElems(xs *XferStream, nelems int) (int, error) {
	idxBytes, idxLen := EncodeLong(int64(nelems))
	buf, err := xs.outWant(idxLen)
	if err != nil {
		return 0, ErrDev
	}
	copy(buf, idxBytes)
	return idxLen, nil
}

func decodeNumElems(xs *XferStream) (int, int, error) {
	lp, err := xs.inWant(EncodedLenLen)
	if err != nil {
		return 0, 0, ErrDev
	}
	digitCount, err := DecodeLen(lp)
	if err != nil {
		return 0, 0, ErrParse
	}
	if digitCount <= 0 || digitCount > 18 {
		return 0, 0, ErrParse
	}
	dp, err := xs.inWant(digitCount)
	if err != nil {
		return 0, 0, ErrDev
	}
	nelems, err := DecodeLong(dp, digitCount)
	if err != nil {
		return 0, 0, ErrParse
	}
	if nelems < 0 || int(nelems) > xs.MaxElements {
		return 0, 0, ErrElems
	}
	total := EncodedLenLen + digitCount
	return int(nelems), total, nil
}

func decomposeXferOut(xs *XferStream, elems []Vc) (int, error) {
	total, err := encodeNumElems(xs, len(elems))
	if err != nil {
		return 0, err
	}
	for i := range elems {
		n, err := elems[i].XferOut(xs)
		if err != nil {
			return 0, err
		}
		total += n
	}
	return total, nil
}

func decomposeXferIn(xs *XferStream) ([]Vc, int, error) {
	nelems, total, err := decodeNumElems(xs)
	if err != nil {
		return nil, total, err
	}
	elems := make([]Vc, nelems)
	for i := 0; i < nelems; i++ {
		v, n, err := xs.XferIn()
		if err != nil {
			return nil, total, err
		}
		elems[i] = v
		total += n
	}
	return elems, total, nil
}

func mapXferOut(xs *XferStream, kvs []kvPair) (int, error) {
	total, err := encodeNumElems(xs, len(kvs))
	if err != nil {
		return 0, err
	}
	for i := range kvs {
		n, err := kvs[i].key.XferOut(xs)
		if err != nil {
			return 0, err
		}
		total += n
		n, err = kvs[i].value.XferOut(xs)
		if err != nil {
			return 0, err
		}
		total += n
	}
	return total, nil
}

func mapXferIn(xs *XferStream) ([]kvPair, int, error) {
	nelems, total, err := decodeNumElems(xs)
	if err != nil {
		return nil, total, err
	}
	kvs := make([]kvPair, nelems)
	for i := 0; i < nelems; i++ {
		k, n, err := xs.XferIn()
		if err != nil {
			return nil, total, err
		}
		total += n
		v, n, err := xs.XferIn()
		if err != nil {
			return nil, total, err
		}
		total += n
		kvs[i] = kvPair{key: k, value: v}
	}
	return kvs, total, nil
}

// --- Convenience API ---

// Encode serializes a Vc to bytes.
func (v *Vc) Encode() ([]byte, error) {
	xs := NewXferStreamForWrite()
	n, err := v.XferOut(xs)
	if err != nil {
		return nil, err
	}
	out := make([]byte, n)
	copy(out, xs.wbuf[:n])
	return out, nil
}

// Decode deserializes a Vc from bytes.
func Decode(data []byte) (Vc, error) {
	xs := NewXferStreamForRead(data)
	v, _, err := xs.XferIn()
	return v, err
}

// DecodeWithConstraints deserializes with explicit security limits.
func DecodeWithConstraints(data []byte, maxMem, maxDepth, maxElems, maxElemLen int64) (Vc, error) {
	xs := NewXferStreamForRead(data)
	xs.SetConstraints(maxMem, maxDepth, maxElems, maxElemLen)
	v, _, err := xs.XferIn()
	return v, err
}
