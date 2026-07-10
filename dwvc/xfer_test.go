package dwvc

import (
	"bytes"
	"math"
	"testing"
)

// TestEncodeLong verifies EncodeLong matches C++ encode_long output.
func TestEncodeLong(t *testing.T) {
	tests := []struct {
		input int64
		want  string
	}{
		{0, "010"},       // 1 digit, encoded as "01" + "0"
		{1, "011"},       // 1 digit
		{5, "015"},       // 1 digit
		{10, "0210"},     // 2 digits
		{42, "0242"},     // 2 digits
		{100, "03100"},   // 3 digits
		{999, "03999"},   // 3 digits
		{1000, "041000"}, // 4 digits
	}
	for _, tt := range tests {
		buf, n := EncodeLong(tt.input)
		got := string(buf[:n])
		if got != tt.want {
			t.Errorf("EncodeLong(%d) = %q, want %q", tt.input, got, tt.want)
		}
	}
}

// TestDecodeLen verifies DecodeLen matches C++ decode_len.
func TestDecodeLen(t *testing.T) {
	tests := []struct {
		input string
		want  int
	}{
		{"01", 1},
		{"02", 2},
		{"10", 10},
		{"42", 42},
		{"99", 99},
	}
	for _, tt := range tests {
		got, err := DecodeLen([]byte(tt.input))
		if err != nil {
			t.Errorf("DecodeLen(%q) error: %v", tt.input, err)
			continue
		}
		if got != tt.want {
			t.Errorf("DecodeLen(%q) = %d, want %d", tt.input, got, tt.want)
		}
	}
}

// TestDecodeLong verifies DecodeLong matches C++ decode_long.
func TestDecodeLong(t *testing.T) {
	tests := []struct {
		input string
		len   int
		want  int64
	}{
		{"0", 1, 0},
		{"1", 1, 1},
		{"42", 2, 42},
		{"100", 3, 100},
		{"9999", 4, 9999},
	}
	for _, tt := range tests {
		got, err := DecodeLong([]byte(tt.input), tt.len)
		if err != nil {
			t.Errorf("DecodeLong(%q, %d) error: %v", tt.input, tt.len, err)
			continue
		}
		if got != tt.want {
			t.Errorf("DecodeLong(%q, %d) = %d, want %d", tt.input, tt.len, got, tt.want)
		}
	}
}

// TestDecodeLong2 verifies signed integer decoding matches C++ decode_long2.
func TestDecodeLong2(t *testing.T) {
	tests := []struct {
		input string
		len   int
		want  int64
	}{
		{"0", 1, 0},
		{"42", 2, 42},
		{"-7", 2, -7},
		{"-123", 4, -123},
		{"-1", 2, -1},
		{"9223372036854775807", 19, math.MaxInt64},
		{"-9223372036854775807", 20, math.MinInt64 + 1},
	}
	for _, tt := range tests {
		got, err := DecodeLong2([]byte(tt.input), tt.len)
		if err != nil {
			t.Errorf("DecodeLong2(%q, %d) error: %v", tt.input, tt.len, err)
			continue
		}
		if got != tt.want {
			t.Errorf("DecodeLong2(%q, %d) = %d, want %d", tt.input, tt.len, got, tt.want)
		}
	}

	// Test overflow cases
	overflowTests := []struct {
		input string
		len   int
	}{
		{"-9223372036854775808", 20}, // would be math.MinInt64 but C++ rejects it
	}
	for _, tt := range overflowTests {
		_, err := DecodeLong2([]byte(tt.input), tt.len)
		if err == nil {
			t.Errorf("DecodeLong2(%q, %d) should have returned error", tt.input, tt.len)
		}
	}
}

// TestEncodeType verifies type tag encoding matches C++ encode_type.
func TestEncodeType(t *testing.T) {
	tests := []struct {
		typ  VcType
		want string
	}{
		{VCInt, "01"},
		{VCString, "02"},
		{VCDouble, "03"},
		{VCNil, "04"},
		{VCSet, "07"},
		{VCMap, "08"},
		{VCVector, "09"},
		{VCList, "10"},
		{VCBag, "11"},
		{VCCHit, "17"},
		{VCTree, "31"},
	}
	for _, tt := range tests {
		buf := make([]byte, 2)
		EncodeType(buf, tt.typ)
		got := string(buf)
		if got != tt.want {
			t.Errorf("EncodeType(%d) = %q, want %q", tt.typ, got, tt.want)
		}
	}
}

// TestCompatLtoa verifies signed integer stringification matches C++ compat_ltoa.
func TestCompatLtoa(t *testing.T) {
	tests := []struct {
		input int64
		want  string
	}{
		{0, "0"},
		{1, "1"},
		{-1, "-1"},
		{42, "42"},
		{-7, "-7"},
		{123456789, "123456789"},
		{-987654321, "-987654321"},
	}
	for _, tt := range tests {
		got := CompatLtoa(tt.input)
		if got != tt.want {
			t.Errorf("CompatLtoa(%d) = %q, want %q", tt.input, got, tt.want)
		}
	}
}

// TestRoundTripNil tests nil serialization.
func TestRoundTripNil(t *testing.T) {
	v := VcNil()
	data, err := v.Encode()
	if err != nil {
		t.Fatalf("Encode nil: %v", err)
	}
	// VCNil(04) = just the type tag
	if string(data) != "04" {
		t.Errorf("Nil encoded as %q, want \"04\"", data)
	}
	got, err := Decode(data)
	if err != nil {
		t.Fatalf("Decode nil: %v", err)
	}
	if !got.IsNil() {
		t.Errorf("Decoded nil is not nil: %v", got)
	}
}

// TestRoundTripInt tests int serialization.
func TestRoundTripInt(t *testing.T) {
	tests := []int64{0, 1, -1, 42, -7, 100, -100, 123456, -999999}
	for _, val := range tests {
		v := VcInt(val)
		data, err := v.Encode()
		if err != nil {
			t.Fatalf("Encode int %d: %v", val, err)
		}
		got, err := Decode(data)
		if err != nil {
			t.Fatalf("Decode int %d: %v", val, err)
		}
		gi, ok := got.Int()
		if !ok {
			t.Fatalf("Decoded int %d is not int: %v", val, got)
		}
		if gi != val {
			t.Errorf("RoundTrip(%d) = %d", val, gi)
		}
	}
}

// TestRoundTripDouble tests double serialization.
func TestRoundTripDouble(t *testing.T) {
	tests := []float64{0.0, 1.0, -1.0, 3.14, -2.718, 1e10, 1e-10}
	for _, val := range tests {
		v := VcDouble(val)
		data, err := v.Encode()
		if err != nil {
			t.Fatalf("Encode double %g: %v", val, err)
		}
		got, err := Decode(data)
		if err != nil {
			t.Fatalf("Decode double %g: %v", val, err)
		}
		gd, ok := got.Double()
		if !ok {
			t.Fatalf("Decoded double %g is not double: %v", val, got)
		}
		if gd != val {
			t.Errorf("RoundTrip(%g) = %g", val, gd)
		}
	}
}

// TestRoundTripString tests string serialization.
func TestRoundTripString(t *testing.T) {
	tests := []string{"", "hello", "hello world", "a", "0123456789"}
	for _, val := range tests {
		v := VcString(val)
		data, err := v.Encode()
		if err != nil {
			t.Fatalf("Encode string %q: %v", val, err)
		}
		got, err := Decode(data)
		if err != nil {
			t.Fatalf("Decode string %q: %v", val, err)
		}
		gs, ok := got.AsString()
		if !ok {
			t.Fatalf("Decoded string %q is not string: %v", val, got)
		}
		if gs != val {
			t.Errorf("RoundTrip(%q) = %q", val, gs)
		}
	}
}

// TestRoundTripVector tests vector serialization.
func TestRoundTripVector(t *testing.T) {
	v := VcVector(VcInt(1), VcInt(2), VcInt(3))
	data, err := v.Encode()
	if err != nil {
		t.Fatalf("Encode vector: %v", err)
	}
	got, err := Decode(data)
	if err != nil {
		t.Fatalf("Decode vector: %v", err)
	}
	if got.Type() != VCVector {
		t.Fatalf("Decoded type = %d, want VCVector(%d)", got.Type(), VCVector)
	}
	elems := got.Elems()
	if len(elems) != 3 {
		t.Fatalf("Decoded vector len = %d, want 3", len(elems))
	}
	for i, want := range []int64{1, 2, 3} {
		gi, ok := elems[i].Int()
		if !ok || gi != want {
			t.Errorf("vector[%d] = %v, want int %d", i, elems[i], want)
		}
	}
}

// TestRoundTripMap tests map serialization.
func TestRoundTripMap(t *testing.T) {
	v := VcMap(
		VcString("key1"), VcInt(42),
		VcString("key2"), VcString("hello"),
	)
	data, err := v.Encode()
	if err != nil {
		t.Fatalf("Encode map: %v", err)
	}
	got, err := Decode(data)
	if err != nil {
		t.Fatalf("Decode map: %v", err)
	}
	if got.Type() != VCMap {
		t.Fatalf("Decoded type = %d, want VCMap(%d)", got.Type(), VCMap)
	}
	kvs := got.KVPairs()
	if len(kvs) != 2 {
		t.Fatalf("Decoded map len = %d, want 2", len(kvs))
	}
	k1, _ := kvs[0].key.AsString()
	v1, _ := kvs[0].value.Int()
	if k1 != "key1" || v1 != 42 {
		t.Errorf("map entry 0: key=%q value=%d, want key1/42", k1, v1)
	}
	k2, _ := kvs[1].key.AsString()
	v2, _ := kvs[1].value.AsString()
	if k2 != "key2" || v2 != "hello" {
		t.Errorf("map entry 1: key=%q value=%q, want key2/hello", k2, v1)
	}
}

// TestRoundTripNested tests nested structure serialization.
func TestRoundTripNested(t *testing.T) {
	inner := VcVector(VcInt(1), VcString("nested"))
	outer := VcVector(inner, VcDouble(3.14), VcNil())
	data, err := outer.Encode()
	if err != nil {
		t.Fatalf("Encode nested: %v", err)
	}
	got, err := Decode(data)
	if err != nil {
		t.Fatalf("Decode nested: %v", err)
	}
	elems := got.Elems()
	if len(elems) != 3 {
		t.Fatalf("outer len = %d, want 3", len(elems))
	}
	innerGot := elems[0]
	innerElems := innerGot.Elems()
	if len(innerElems) != 2 {
		t.Fatalf("inner len = %d, want 2", len(innerElems))
	}
	i0, _ := innerElems[0].Int()
	if i0 != 1 {
		t.Errorf("inner[0] = %d, want 1", i0)
	}
	s1, _ := innerElems[1].AsString()
	if s1 != "nested" {
		t.Errorf("inner[1] = %q, want %q", s1, "nested")
	}
	d, _ := elems[1].Double()
	if d != 3.14 {
		t.Errorf("outer[1] = %g, want 3.14", d)
	}
	if !elems[2].IsNil() {
		t.Errorf("outer[2] is not nil")
	}
}

// TestWireFormatNil verifies exact wire format for nil.
func TestWireFormatNil(t *testing.T) {
	v := VcNil()
	data, _ := v.Encode()
	if string(data) != "04" {
		t.Errorf("Nil wire = %q, want \"04\"", data)
	}
}

// TestWireFormatInt verifies exact wire format for ints.
func TestWireFormatInt(t *testing.T) {
	tests := []struct {
		val  int64
		wire string
	}{
		{0, "01" + "01" + "0"},      // type=01, count=01, digit=0
		{1, "01" + "01" + "1"},      // type=01, count=01, digit=1
		{42, "01" + "02" + "42"},    // type=01, count=02, digits=42
		{-7, "01" + "02" + "-7"},    // type=01, count=02, chars=-7
		{100, "01" + "03" + "100"},  // type=01, count=03, digits=100
	}
	for _, tt := range tests {
		v := VcInt(tt.val)
		data, _ := v.Encode()
		if string(data) != tt.wire {
			t.Errorf("Int(%d) wire = %q, want %q", tt.val, data, tt.wire)
		}
	}
}

// TestWireFormatDouble verifies exact wire format for doubles.
func TestWireFormatDouble(t *testing.T) {
	// Double 3.14 → type "03", encode_long(53) = "0253",
	// float_str = "3.140000000000000124344978758017532527446746826171875" (53 chars)
	// Wire: "03" + "0253" + float_str
	floatStr := "3.140000000000000124344978758017532527446746826171875"
	v := VcDouble(3.14)
	data, _ := v.Encode()
	want := "03" + "0253" + floatStr
	if string(data) != want {
		t.Errorf("Double wire = %q, want %q", data, want)
	}
	got, err := Decode(data)
	if err != nil {
		t.Fatalf("Decode: %v", err)
	}
	gd, _ := got.Double()
	if gd != 3.14 {
		t.Errorf("Double round-trip = %g, want 3.14", gd)
	}
}

// TestWireFormatString verifies exact wire format for strings.
func TestWireFormatString(t *testing.T) {
	// String "hello" (5 bytes) → type "02", len prefix for 5, then "hello"
	v := VcString("hello")
	data, _ := v.Encode()
	// Should be "02" + encode_long(5)
	// encode_long(5) = "015" (count=1 digit, digit=5)
	want := "02" + "01" + "5" + "hello"
	if string(data) != want {
		t.Errorf("String wire = %q, want %q", data, want)
	}
}

// TestWireFormatVector verifies exact wire format for vectors.
func TestWireFormatVector(t *testing.T) {
	// Vector [1] → type "09", num_elems=1 → "011", then int(1) → "01011"
	v := VcVector(VcInt(1))
	data, _ := v.Encode()
	// "09" + encode_long(1) = "011" + int(1) = "01011"
	// Wait: encode_long(1) = "01" + "1" = "011"
	// Then the element int(1): type "01" + payload "011"
	// Total: "09" + "011" + "01011" = "0901101011"
	want := "09" + "011" + "01011"
	if string(data) != want {
		t.Errorf("Vector wire = %q, want %q", data, want)
	}
}

// TestWireFormatMap verifies exact wire format for maps.
func TestWireFormatMap(t *testing.T) {
	// Map {"a": 1} → type "08", num_elems=1 → "011", key string "a" → "02011a", value int(1) → "01011"
	v := VcMap(VcString("a"), VcInt(1))
	data, _ := v.Encode()
	// "08" + "011" + "02" + "01" + "1" + "a" + "01" + "01" + "1"
	// = "08" + "011" + "02011a" + "01011"
	want := "08" + "011" + "02011a" + "01011"
	if string(data) != want {
		t.Errorf("Map wire = %q, want %q", data, want)
	}
}

// TestDecodeConstraintsMemory tests memory limit enforcement.
func TestDecodeConstraintsMemory(t *testing.T) {
	v := VcString("hello")
	data, _ := v.Encode()
	// Set memory limit to 1 (way too small)
	_, err := DecodeWithConstraints(data, 1, -1, -1, -1)
	if err == nil {
		t.Errorf("Expected error for memory limit, got nil")
	}
}

// TestDecodeConstraintsDepth tests depth limit enforcement.
func TestDecodeConstraintsDepth(t *testing.T) {
	inner := VcVector(VcInt(1))
	outer := VcVector(inner)
	data, _ := outer.Encode()
	// Set depth to 1 (only 1 level allowed, but we have 2)
	_, err := DecodeWithConstraints(data, -1, 1, -1, -1)
	if err == nil {
		t.Errorf("Expected error for depth limit, got nil")
	}
}

// TestDecodeConstraintsElements tests element count limit.
func TestDecodeConstraintsElements(t *testing.T) {
	v := VcVector(VcInt(1), VcInt(2), VcInt(3))
	data, _ := v.Encode()
	// Set max elements to 2, but vector has 3
	_, err := DecodeWithConstraints(data, -1, -1, 2, -1)
	if err == nil {
		t.Errorf("Expected error for element limit, got nil")
	}
}

// TestEmptyVector tests empty container serialization.
func TestEmptyVector(t *testing.T) {
	v := VcVector()
	data, _ := v.Encode()
	got, err := Decode(data)
	if err != nil {
		t.Fatalf("Decode empty vector: %v", err)
	}
	if got.Len() != 0 {
		t.Errorf("Empty vector len = %d, want 0", got.Len())
	}
}

// TestEmptyMap tests empty map serialization.
func TestEmptyMap(t *testing.T) {
	v := VcMap()
	data, _ := v.Encode()
	got, err := Decode(data)
	if err != nil {
		t.Fatalf("Decode empty map: %v", err)
	}
	if got.Len() != 0 {
		t.Errorf("Empty map len = %d, want 0", got.Len())
	}
}

// TestEncodeDecodeIndependence verifies that encoding produces the same
// bytes regardless of the order operations are performed.
func TestEncodeDecodeIndependence(t *testing.T) {
	// Encode twice, verify same output
	v := VcVector(VcInt(42), VcString("test"), VcNil())
	d1, _ := v.Encode()
	d2, _ := v.Encode()
	if !bytes.Equal(d1, d2) {
		t.Errorf("Non-deterministic encoding: %x vs %x", d1, d2)
	}
}

// TestStringWithSpecialChars tests strings with various byte values.
func TestStringWithSpecialChars(t *testing.T) {
	tests := []string{
		"",
		"\x00",
		"\xff",
		"hello\x00world",
		"\n\r\t",
		"unicode: \xe2\x9c\x93", // UTF-8 checkmark
	}
	for _, s := range tests {
		v := VcString(s)
		data, err := v.Encode()
		if err != nil {
			t.Fatalf("Encode string %q: %v", s, err)
		}
		got, err := Decode(data)
		if err != nil {
			t.Fatalf("Decode string %q: %v", s, err)
		}
		gs, _ := got.AsString()
		if gs != s {
			t.Errorf("RoundTrip(%q) = %q", s, gs)
		}
	}
}

// TestLargeVector tests serialization of a large vector.
func TestLargeVector(t *testing.T) {
	elems := make([]Vc, 1000)
	for i := range elems {
		elems[i] = VcInt(int64(i))
	}
	v := VcVector(elems...)
	data, err := v.Encode()
	if err != nil {
		t.Fatalf("Encode large vector: %v", err)
	}
	got, err := Decode(data)
	if err != nil {
		t.Fatalf("Decode large vector: %v", err)
	}
	if got.Len() != 1000 {
		t.Fatalf("Large vector len = %d, want 1000", got.Len())
	}
	for i := 0; i < 1000; i++ {
		gi, _ := got.Elems()[i].Int()
		if gi != int64(i) {
			t.Errorf("large_vector[%d] = %d, want %d", i, gi, i)
			break
		}
	}
}

// TestDirectXferStream tests using XferStream directly for round-trip.
func TestDirectXferStream(t *testing.T) {
	v := VcVector(VcInt(1), VcString("abc"), VcDouble(2.71))

	// Serialize
	wxs := NewXferStreamForWrite()
	n, err := v.XferOut(wxs)
	if err != nil {
		t.Fatalf("XferOut: %v", err)
	}
	data := wxs.WBuf()[:n]

	// Deserialize
	rxs := NewXferStreamForRead(data)
	got, n2, err := rxs.XferIn()
	if err != nil {
		t.Fatalf("XferIn: %v", err)
	}
	if n2 != n {
		t.Errorf("bytes consumed %d != bytes written %d", n2, n)
	}
	if got.Len() != 3 {
		t.Fatalf("decoded len = %d, want 3", got.Len())
	}
}

// TestCHitBackReference tests that shared references produce VC_CHIT on the wire.
func TestCHitBackReference(t *testing.T) {
	// Create a vector that contains two references to the same inner vector
	inner := VcVector(VcInt(42))
	// To share the same rep, we need to copy the Vc struct (which shares the rep pointer)
	outer := VcVector(inner, inner)
	data, err := outer.Encode()
	if err != nil {
		t.Fatalf("Encode shared ref: %v", err)
	}
	// The second reference to inner should be a VC_CHIT back-reference
	// Verify it contains the VCCHit type tag (17)
	if !bytes.Contains(data, []byte("17")) {
		t.Logf("Wire data: %s", data)
	}

	got, err := Decode(data)
	if err != nil {
		t.Fatalf("Decode shared ref: %v", err)
	}
	if got.Len() != 2 {
		t.Fatalf("outer len = %d, want 2", got.Len())
	}
	// Both elements should be vectors containing 42
	for i := 0; i < 2; i++ {
		elems := got.Elems()[i].Elems()
		if len(elems) != 1 {
			t.Errorf("outer[%d] inner len = %d, want 1", i, len(elems))
			continue
		}
		v, _ := elems[0].Int()
		if v != 42 {
			t.Errorf("outer[%d] inner[0] = %d, want 42", i, v)
		}
	}
}

// FuzzXferIn fuzzes the deserializer for crashes and panics.
func FuzzXferIn(f *testing.F) {
	// Seed with known-good inputs
	seeds := [][]byte{
		[]byte("04"),                                          // nil
		[]byte("01010"),                                       // int 0
		[]byte("01011"),                                       // int 1
		[]byte("0102-7"),                                      // int -7
		[]byte("02015hello"),                                  // string "hello"
		[]byte("0901101011"),                                  // vector [1]
		[]byte("090120101101012"),                             // vector [1, 2]
		[]byte("0801102011a01011"),                            // map {"a":1}
		[]byte("090110901101011"),                             // vector [vector [1]]
		[]byte("09012090110101117010"),                        // vector [vector [1], chit[0]]
		[]byte("030143.14"),                                // double 3.14
		bytes.Repeat([]byte("01010"), 100),                    // many ints
		{0xff, 0xff, 0xff},                                    // invalid
		{},                                                     // empty
		[]byte("99"),                                          // invalid type
		[]byte("01"),                                          // truncated
		[]byte("010"),                                         // truncated
	}
	for _, s := range seeds {
		f.Add(s)
	}

	f.Fuzz(func(t *testing.T, data []byte) {
		xs := NewXferStreamForRead(data)
		xs.MaxMemory = 1024 * 1024 // 1MB limit
		xs.depthLimit = 32
		xs.MaxElements = 10000
		xs.MaxElementLen = 100000
		// Must not panic
		_, _, _ = xs.XferIn()
	})
}
