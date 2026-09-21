package dwvc

// vc.go — the universal value type and its concrete representations.
//
// Vc is a dynamically-typed container analogous to the C++ vc class.
// For the xfer subsystem, only the serializable subset of types is needed.

import (
	"bytes"
	"fmt"
	"io"
	"math"
)

// VcType matches the C++ vc_type enum values that are serialized.
type VcType int16

const (
	VCInt     VcType = 1
	VCString  VcType = 2
	VCDouble  VcType = 3
	VCNil     VcType = 4
	VCSet     VcType = 7
	VCMap     VcType = 8
	VCVector  VcType = 9
	VCList    VcType = 10
	VCBag     VcType = 11
	VCRegex   VcType = 12
	VCCHit    VcType = 17
	VCTree    VcType = 31
)

// vcRep is the internal interface for type-specific serialization.
// Analog of vc_default's virtual xfer_out/xfer_in methods.
type vcRep interface {
	vcType() VcType
	isAtomic() bool
	xferOut(xs *XferStream) (int, error)
	xferIn(xs *XferStream) (int, error)
}

// Vc is the universal value type.
type Vc struct {
	rep vcRep
}

// IsNil returns true if this Vc holds no value.
func (v Vc) IsNil() bool {
	return v.rep == nil
}

// Type returns the wire type of the value.
func (v Vc) Type() VcType {
	if v.rep == nil {
		return VCNil
	}
	return v.rep.vcType()
}

// --- Constructors ---

func VcNil() Vc {
	return Vc{rep: nil}
}

func VcInt(i int64) Vc {
	return Vc{rep: &vcInt{i: i}}
}

func VcDouble(d float64) Vc {
	return Vc{rep: &vcDouble{d: d}}
}

func VcString(s string) Vc {
	return Vc{rep: &vcString{s: s}}
}

func VcVector(elems ...Vc) Vc {
	e := make([]Vc, len(elems))
	copy(e, elems)
	return Vc{rep: &vcVector{elems: e}}
}

func VcMap(pairs ...Vc) Vc {
	// pairs must be key, value, key, value...
	if len(pairs)%2 != 0 {
		panic("dwvc: VcMap requires even number of arguments")
	}
	kvs := make([]kvPair, 0, len(pairs)/2)
	for i := 0; i < len(pairs); i += 2 {
		kvs = append(kvs, kvPair{key: pairs[i], value: pairs[i+1]})
	}
	return Vc{rep: &vcMap{kvs: kvs}}
}

func VcSet(elems ...Vc) Vc {
	e := make([]Vc, len(elems))
	copy(e, elems)
	return Vc{rep: &vcSet{elems: e}}
}

func VcBag(elems ...Vc) Vc {
	e := make([]Vc, len(elems))
	copy(e, elems)
	return Vc{rep: &vcBag{elems: e}}
}

func VcList(elems ...Vc) Vc {
	e := make([]Vc, len(elems))
	copy(e, elems)
	return Vc{rep: &vcList{elems: e}}
}

func VcTree(pairs ...Vc) Vc {
	if len(pairs)%2 != 0 {
		panic("dwvc: VcTree requires even number of arguments")
	}
	kvs := make([]kvPair, 0, len(pairs)/2)
	for i := 0; i < len(pairs); i += 2 {
		kvs = append(kvs, kvPair{key: pairs[i], value: pairs[i+1]})
	}
	return Vc{rep: &vcTree{kvs: kvs}}
}

// --- Accessors ---

func (v Vc) Int() (int64, bool) {
	if iv, ok := v.rep.(*vcInt); ok {
		return iv.i, true
	}
	return 0, false
}

func (v Vc) Double() (float64, bool) {
	if dv, ok := v.rep.(*vcDouble); ok {
		return dv.d, true
	}
	return 0, false
}

// AsString returns the string value and true if this is a VCString.
func (v Vc) AsString() (string, bool) {
	if sv, ok := v.rep.(*vcString); ok {
		return sv.s, true
	}
	return "", false
}

func (v Vc) Len() int {
	if v.rep == nil {
		return 0
	}
	switch r := v.rep.(type) {
	case *vcVector:
		return len(r.elems)
	case *vcMap:
		return len(r.kvs)
	case *vcSet:
		return len(r.elems)
	case *vcBag:
		return len(r.elems)
	case *vcList:
		return len(r.elems)
	case *vcTree:
		return len(r.kvs)
	case *vcString:
		return len(r.s)
	}
	return 0
}

func (v Vc) Elems() []Vc {
	if v.rep == nil {
		return nil
	}
	switch r := v.rep.(type) {
	case *vcVector:
		return r.elems
	case *vcSet:
		return r.elems
	case *vcBag:
		return r.elems
	case *vcList:
		return r.elems
	}
	return nil
}

func (v Vc) KVPairs() []kvPair {
	if v.rep == nil {
		return nil
	}
	switch r := v.rep.(type) {
	case *vcMap:
		return r.kvs
	case *vcTree:
		return r.kvs
	}
	return nil
}

// kvPair is a key-value pair for maps and trees.
type kvPair struct {
	key   Vc
	value Vc
}

// --- Concrete representations ---

type vcInt struct {
	i int64
}

func (v *vcInt) vcType() VcType  { return VCInt }
func (v *vcInt) isAtomic() bool  { return true }
func (v *vcInt) xferOut(xs *XferStream) (int, error) { return vcIntXferOut(xs, v.i) }
func (v *vcInt) xferIn(xs *XferStream) (int, error) {
	vcVal, n, err := vcIntXferIn(xs)
	if err != nil {
		return n, err
	}
	if iv, ok := vcVal.Int(); ok {
		v.i = iv
	}
	return n, nil
}

type vcDouble struct {
	d float64
}

func (v *vcDouble) vcType() VcType  { return VCDouble }
func (v *vcDouble) isAtomic() bool  { return true }
func (v *vcDouble) xferOut(xs *XferStream) (int, error) { return vcDoubleXferOut(xs, v.d) }
func (v *vcDouble) xferIn(xs *XferStream) (int, error) {
	vcVal, n, err := vcDoubleXferIn(xs)
	if err != nil {
		return n, err
	}
	if dv, ok := vcVal.Double(); ok {
		v.d = dv
	}
	return n, nil
}

type vcString struct {
	s string
}

func (v *vcString) vcType() VcType  { return VCString }
func (v *vcString) isAtomic() bool  { return true }
func (v *vcString) xferOut(xs *XferStream) (int, error) { return vcStringXferOut(xs, v.s) }
func (v *vcString) xferIn(xs *XferStream) (int, error) {
	vcVal, n, err := vcStringXferIn(xs)
	if err != nil {
		return n, err
	}
	if sv, ok := vcVal.AsString(); ok {
		v.s = sv
	}
	return n, nil
}

type vcVector struct {
	elems []Vc
}

func (v *vcVector) vcType() VcType  { return VCVector }
func (v *vcVector) isAtomic() bool  { return false }
func (v *vcVector) xferOut(xs *XferStream) (int, error) { return decomposeXferOut(xs, v.elems) }
func (v *vcVector) xferIn(xs *XferStream) (int, error) {
	elems, n, err := decomposeXferIn(xs)
	if err != nil {
		return n, err
	}
	v.elems = elems
	return n, nil
}

type vcMap struct {
	kvs []kvPair
}

func (v *vcMap) vcType() VcType  { return VCMap }
func (v *vcMap) isAtomic() bool  { return false }
func (v *vcMap) xferOut(xs *XferStream) (int, error) { return mapXferOut(xs, v.kvs) }
func (v *vcMap) xferIn(xs *XferStream) (int, error) {
	kvs, n, err := mapXferIn(xs)
	if err != nil {
		return n, err
	}
	v.kvs = kvs
	return n, nil
}

type vcSet struct {
	elems []Vc
}

func (v *vcSet) vcType() VcType  { return VCSet }
func (v *vcSet) isAtomic() bool  { return false }
func (v *vcSet) xferOut(xs *XferStream) (int, error) { return decomposeXferOut(xs, v.elems) }
func (v *vcSet) xferIn(xs *XferStream) (int, error) {
	elems, n, err := decomposeXferIn(xs)
	if err != nil {
		return n, err
	}
	v.elems = elems
	return n, nil
}

type vcBag struct {
	elems []Vc
}

func (v *vcBag) vcType() VcType  { return VCBag }
func (v *vcBag) isAtomic() bool  { return false }
func (v *vcBag) xferOut(xs *XferStream) (int, error) { return decomposeXferOut(xs, v.elems) }
func (v *vcBag) xferIn(xs *XferStream) (int, error) {
	elems, n, err := decomposeXferIn(xs)
	if err != nil {
		return n, err
	}
	v.elems = elems
	return n, nil
}

type vcList struct {
	elems []Vc
}

func (v *vcList) vcType() VcType  { return VCList }
func (v *vcList) isAtomic() bool  { return false }
func (v *vcList) xferOut(xs *XferStream) (int, error) { return decomposeXferOut(xs, v.elems) }
func (v *vcList) xferIn(xs *XferStream) (int, error) {
	elems, n, err := decomposeXferIn(xs)
	if err != nil {
		return n, err
	}
	v.elems = elems
	return n, nil
}

type vcTree struct {
	kvs []kvPair
}

func (v *vcTree) vcType() VcType  { return VCTree }
func (v *vcTree) isAtomic() bool  { return false }
func (v *vcTree) xferOut(xs *XferStream) (int, error) { return mapXferOut(xs, v.kvs) }
func (v *vcTree) xferIn(xs *XferStream) (int, error) {
	kvs, n, err := mapXferIn(xs)
	if err != nil {
		return n, err
	}
	v.kvs = kvs
	return n, nil
}

// --- String representation for debugging ---

func (v Vc) writeTo(w io.Writer, visited map[uintptr]int) {
	if v.rep == nil {
		fmt.Fprint(w, "nil")
		return
	}
	if v.rep.isAtomic() {
		switch r := v.rep.(type) {
		case *vcInt:
			fmt.Fprintf(w, "%d", r.i)
		case *vcDouble:
			fmt.Fprintf(w, "%.*g", 17, r.d)
		case *vcString:
			fmt.Fprintf(w, "%q", r.s)
		}
		return
	}
	id := repID(v.rep)
	if idx, ok := visited[id]; ok {
		fmt.Fprintf(w, "#%d", idx)
		return
	}
	visited[id] = len(visited)
	switch r := v.rep.(type) {
	case *vcVector:
		fmt.Fprint(w, "vector(")
		for i, e := range r.elems {
			if i > 0 {
				fmt.Fprint(w, " ")
			}
			e.writeTo(w, visited)
		}
		fmt.Fprint(w, ")")
	case *vcSet:
		fmt.Fprint(w, "set(")
		for i, e := range r.elems {
			if i > 0 {
				fmt.Fprint(w, " ")
			}
			e.writeTo(w, visited)
		}
		fmt.Fprint(w, ")")
	case *vcBag:
		fmt.Fprint(w, "bag(")
		for i, e := range r.elems {
			if i > 0 {
				fmt.Fprint(w, " ")
			}
			e.writeTo(w, visited)
		}
		fmt.Fprint(w, ")")
	case *vcList:
		fmt.Fprint(w, "list(")
		for i, e := range r.elems {
			if i > 0 {
				fmt.Fprint(w, " ")
			}
			e.writeTo(w, visited)
		}
		fmt.Fprint(w, ")")
	case *vcMap:
		fmt.Fprint(w, "map(")
		for i, kv := range r.kvs {
			if i > 0 {
				fmt.Fprint(w, " ")
			}
			fmt.Fprint(w, "vector(")
			kv.key.writeTo(w, visited)
			fmt.Fprint(w, " ")
			kv.value.writeTo(w, visited)
			fmt.Fprint(w, ")")
		}
		fmt.Fprint(w, ")")
	case *vcTree:
		fmt.Fprint(w, "tree(")
		for i, kv := range r.kvs {
			if i > 0 {
				fmt.Fprint(w, " ")
			}
			fmt.Fprint(w, "vector(")
			kv.key.writeTo(w, visited)
			fmt.Fprint(w, " ")
			kv.value.writeTo(w, visited)
			fmt.Fprint(w, ")")
		}
		fmt.Fprint(w, ")")
	default:
		fmt.Fprint(w, "?")
	}
}

func (v Vc) String() string {
	var buf bytes.Buffer
	v.writeTo(&buf, make(map[uintptr]int))
	return buf.String()
}

// EncodeDouble produces the %g representation matching C++ vc_double::xfer_out.
// Uses precision 1024 to match C++ snprintf(fbuf, 2048, "%.*g", 1024, d).
func EncodeDouble(d float64) string {
	if math.IsInf(d, 1) {
		return "inf"
	}
	if math.IsInf(d, -1) {
		return "-inf"
	}
	if math.IsNaN(d) {
		return "nan"
	}
	return fmt.Sprintf("%.*g", 1024, d)
}
