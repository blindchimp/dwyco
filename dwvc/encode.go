package dwvc

// encode.go — wire format encoding/decoding matching C++ vcenco.cpp exactly.
//
// The wire format uses ASCII-encoded numbers for text-safe transport:
//   - Type tags: 2 ASCII digits (01–37)
//   - Lengths: 2-digit count of following digits, then the digits themselves
//   - Signed ints: 2-digit count of following chars (including '-'), then the chars

import (
	"errors"
	"math"
	"strconv"
)

const (
	EncodedTypeLen = 2
	EncodedLenLen  = 2
)

var (
	ErrParse   = errors.New("vc: parse error")
	ErrDev     = errors.New("vc: device error")
	ErrOOM     = errors.New("vc: memory limit exceeded")
	ErrDepth   = errors.New("vc: depth limit exceeded")
	ErrElems   = errors.New("vc: element count limit exceeded")
	ErrElemLen = errors.New("vc: element length limit exceeded")
)

// pltoa converts a non-negative long to ASCII with a minimum field width.
// If fieldWidth < 0, no padding. Returns the number of ASCII digits produced.
// Matches C++ pltoa(..., fieldWidth).
func pltoa(value int64, fieldWidth int) string {
	digits := strconv.FormatInt(value, 10)
	n := len(digits)
	if fieldWidth > n {
		pad := make([]byte, fieldWidth-n)
		for i := range pad {
			pad[i] = '0'
		}
		return string(pad) + digits
	}
	return digits
}

// EncodeLong writes a non-negative integer as [2-digit-count][ASCII-digits].
// Returns the encoded bytes and length written. Matches C++ encode_long.
func EncodeLong(l int64) ([]byte, int) {
	if l < 0 {
		panic("dwvc: EncodeLong negative value")
	}
	digits := pltoa(l, -1)
	n := len(digits)
	countField := pltoa(int64(n), 2) // always 2 digits
	out := countField + digits
	return []byte(out), len(out)
}

// encodeLongBuf encodes l into buf starting at offset. Returns bytes written.
func encodeLongBuf(buf []byte, offset int, l int64) int {
	if l < 0 {
		panic("dwvc: encodeLongBuf negative value")
	}
	digits := pltoa(l, -1)
	n := len(digits)
	countField := pltoa(int64(n), 2)
	buf[offset] = countField[0]
	buf[offset+1] = countField[1]
	copy(buf[offset+2:], digits)
	return 2 + n
}

// DecodeLen reads a 2-digit length prefix. Matches C++ decode_len.
func DecodeLen(buf []byte) (int, error) {
	if len(buf) < EncodedLenLen {
		return -1, ErrParse
	}
	c0 := buf[0] - '0'
	c1 := buf[1] - '0'
	if c0 > 9 || c1 > 9 {
		return -1, ErrParse
	}
	return int(c0)*10 + int(c1), nil
}

// DecodeLong reads `length` ASCII digits as a non-negative integer.
// Matches C++ decode_long. Returns -1 on error.
func DecodeLong(buf []byte, length int) (int64, error) {
	if len(buf) < length {
		return -1, ErrParse
	}
	var l int64
	for i := 0; i < length; i++ {
		c := buf[i] - '0'
		if c > 9 {
			return -1, ErrParse
		}
		if l > math.MaxInt64/10 {
			return -1, ErrParse
		}
		l *= 10
		if l > math.MaxInt64-int64(c) {
			return -1, ErrParse
		}
		l += int64(c)
	}
	return l, nil
}

// DecodeLong2 reads a signed ASCII integer with overflow detection.
// Matches C++ decode_long2.
func DecodeLong2(buf []byte, length int) (int64, error) {
	if length <= 0 || len(buf) < length {
		return -1, ErrParse
	}
	start := 0
	neg := false
	if buf[0] == '-' {
		if length == 1 {
			return -1, ErrParse
		}
		neg = true
		start = 1
	}

	const maxUint64 = ^uint64(0)
	const maxInt64 = int64(maxUint64 >> 1)

	var l uint64
	for i := start; i < length; i++ {
		c := buf[i] - '0'
		if c > 9 {
			return -1, ErrParse
		}
		if l > maxUint64/10 {
			return -1, ErrParse
		}
		l *= 10
		d := uint64(c)
		if l > maxUint64-d {
			return -1, ErrParse
		}
		l += d
	}

	if neg {
		if l > uint64(maxInt64) {
			return -1, ErrParse
		}
		return -int64(l), nil
	}
	if l > maxUint64/2 {
		return -1, ErrParse
	}
	return int64(l), nil
}

// EncodeType writes a 2-digit type tag. Matches C++ encode_type.
func EncodeType(buf []byte, t VcType) int {
	v := int(t)
	if v < 0 || v > 99 {
		panic("dwvc: EncodeType bad value")
	}
	buf[0] = byte('0' + v/10)
	buf[1] = byte('0' + v%10)
	return EncodedTypeLen
}

// DecodeType reads a 2-digit type tag. Matches C++ decode_type.
func DecodeType(buf []byte) (VcType, error) {
	if len(buf) < EncodedTypeLen {
		return 0, ErrParse
	}
	c0 := buf[0] - '0'
	c1 := buf[1] - '0'
	if c0 > 9 || c1 > 9 {
		return 0, ErrParse
	}
	return VcType(int(c0)*10 + int(c1)), nil
}

// CompatLtoa converts a signed long to ASCII (no field padding).
// Matches C++ compat_ltoa.
func CompatLtoa(l int64) string {
	return strconv.FormatInt(l, 10)
}

// EncodeSignedInt encodes a signed integer for xfer_out.
// Wire format: [2-digit count of chars][chars from compat_ltoa].
// Returns encoded bytes and length.
func EncodeSignedInt(v int64) ([]byte, int) {
	s := CompatLtoa(v)
	n := len(s)
	countField := pltoa(int64(n), 2)
	out := countField + s
	return []byte(out), len(out)
}
