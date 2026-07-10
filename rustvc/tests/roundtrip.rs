use rustvc::{Constraints, Vc};
use rustvc::types::VcType;

/// Helper: serialize then deserialize, verify round-trip
fn roundtrip(vc: &Vc) -> Vc {
    let bytes = rustvc::to_bytes(vc);
    let result = rustvc::from_bytes(&bytes).expect("deserialize failed");
    result
}

// ============================================================
// Nil
// ============================================================

#[test]
fn test_nil() {
    let vc = Vc::Nil;
    let bytes = rustvc::to_bytes(&vc);
    // VC_NIL = 4, type tag "04", no payload
    assert_eq!(bytes, b"04");
    let rt = roundtrip(&vc);
    assert_eq!(rt, Vc::Nil);
}

// ============================================================
// Int
// ============================================================

#[test]
fn test_int_zero() {
    let vc = Vc::int(0);
    let bytes = rustvc::to_bytes(&vc);
    // VC_INT = 1 → "01"
    // compat_ltoa(0) → "0", len 1 → encode_long(1) → "01"
    // payload: "01" + "0" = "010"
    // full: "01" + "010" = "01010"
    assert_eq!(bytes, b"01010");
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_int(), Some(0));
}

#[test]
fn test_int_positive() {
    let vc = Vc::int(42);
    let bytes = rustvc::to_bytes(&vc);
    // type "01", compat_ltoa(42) → "42", encode_long(2) → "02"
    // payload: "0242"
    // full: "01" + "0242" = "010242"
    assert_eq!(bytes, b"010242");
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_int(), Some(42));
}

#[test]
fn test_int_negative() {
    let vc = Vc::int(-123);
    let bytes = rustvc::to_bytes(&vc);
    // type "01", compat_ltoa(-123) → "-123" (4 bytes), encode_long(4) → "04"
    // full: "01" + "04" + "-123" = "0104-123"
    assert_eq!(bytes, b"0104-123");
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_int(), Some(-123));
}

#[test]
fn test_int_large() {
    let vc = Vc::int(999999999);
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_int(), Some(999999999));
}

#[test]
fn test_int_negative_one() {
    let vc = Vc::int(-1);
    let bytes = rustvc::to_bytes(&vc);
    // compat_ltoa(-1) → "-1" (2 bytes), encode_long(2) → "02"
    // full: "01" + "02" + "-1" = "0102-1"
    assert_eq!(bytes, b"0102-1");
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_int(), Some(-1));
}

// ============================================================
// Double
// ============================================================

#[test]
fn test_double_integer_value() {
    let vc = Vc::double(3.0);
    let bytes = rustvc::to_bytes(&vc);
    // VC_DOUBLE = 3 → "03"
    // format_double(3.0) → "3", encode_long(1) → "011"
    // full: "03" + "011" + "3" = "030113"
    assert_eq!(bytes, b"030113");
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_double(), Some(3.0));
}

#[test]
fn test_double_fractional() {
    let vc = Vc::double(3.14);
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_double(), Some(3.14));
}

#[test]
fn test_double_zero() {
    let vc = Vc::double(0.0);
    let bytes = rustvc::to_bytes(&vc);
    // format_double(0.0) → "0", encode_long(1) → "011"
    assert_eq!(bytes, b"030110");
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_double(), Some(0.0));
}

#[test]
fn test_double_negative() {
    let vc = Vc::double(-2.5);
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_double(), Some(-2.5));
}

// ============================================================
// String
// ============================================================

#[test]
fn test_string_empty() {
    let vc = Vc::string(b"");
    let bytes = rustvc::to_bytes(&vc);
    // VC_STRING = 2 → "02", encode_long(0) → "010"
    // full: "02" + "010" = "02010"
    assert_eq!(bytes, b"02010");
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_str(), Some(b"".as_ref()));
}

#[test]
fn test_string_hello() {
    let vc = Vc::string(b"hello");
    let bytes = rustvc::to_bytes(&vc);
    // type "02", encode_long(5) → "015", then "hello"
    // full: "02" + "015" + "hello" = "02015hello"
    assert_eq!(bytes, b"02015hello");
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_str(), Some(b"hello".as_ref()));
}

#[test]
fn test_string_binary() {
    let data = vec![0u8, 1, 2, 255, 128];
    let vc = Vc::string(data.clone());
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_str(), Some(data.as_ref()));
}

// ============================================================
// Vector
// ============================================================

#[test]
fn test_vector_empty() {
    let vc = Vc::vector(vec![]);
    let bytes = rustvc::to_bytes(&vc);
    // VC_VECTOR = 9 → "09", encode_long(0) → "010"
    assert_eq!(bytes, b"09010");
    let rt = roundtrip(&vc);
    assert_eq!(rt.num_elems(), 0);
}

#[test]
fn test_vector_ints() {
    let vc = Vc::vector(vec![Vc::int(1), Vc::int(2), Vc::int(3)]);
    let bytes = rustvc::to_bytes(&vc);
    // type "09", count "013", then 3 ints
    assert!(bytes.starts_with(b"09013"));
    let rt = roundtrip(&vc);
    assert_eq!(rt.num_elems(), 3);
    assert_eq!(rt.as_int(), None); // vector is not int
}

#[test]
fn test_vector_nested() {
    let inner = Vc::vector(vec![Vc::int(42)]);
    let outer = Vc::vector(vec![inner]);
    let rt = roundtrip(&outer);
    assert_eq!(rt.num_elems(), 1);
}

// ============================================================
// Map
// ============================================================

#[test]
fn test_map_empty() {
    let vc = Vc::map(vec![]);
    let bytes = rustvc::to_bytes(&vc);
    assert_eq!(bytes, b"08010");
}

#[test]
fn test_map_single() {
    let vc = Vc::map(vec![
        (Vc::string(b"key"), Vc::int(42)),
    ]);
    let rt = roundtrip(&vc);
    assert_eq!(rt.num_elems(), 1);
}

// ============================================================
// Set, Bag, List
// ============================================================

#[test]
fn test_set() {
    let vc = Vc::set(vec![Vc::int(1), Vc::int(2)]);
    let rt = roundtrip(&vc);
    assert_eq!(rt.num_elems(), 2);
}

#[test]
fn test_bag() {
    let vc = Vc::bag(vec![Vc::int(1), Vc::int(2), Vc::int(1)]);
    let rt = roundtrip(&vc);
    assert_eq!(rt.num_elems(), 3);
}

#[test]
fn test_list() {
    let vc = Vc::list(vec![Vc::int(1), Vc::string(b"two")]);
    let rt = roundtrip(&vc);
    assert_eq!(rt.num_elems(), 2);
}

// ============================================================
// Tree
// ============================================================

#[test]
fn test_tree() {
    let vc = Vc::tree(vec![
        (Vc::int(1), Vc::string(b"one")),
        (Vc::int(2), Vc::string(b"two")),
    ]);
    let rt = roundtrip(&vc);
    assert_eq!(rt.num_elems(), 2);
}

// ============================================================
// Complex nesting
// ============================================================

#[test]
fn test_complex_structure() {
    let vc = Vc::vector(vec![
        Vc::int(42),
        Vc::string(b"hello"),
        Vc::double(3.14),
        Vc::Nil,
        Vc::vector(vec![Vc::int(1), Vc::int(2)]),
        Vc::map(vec![
            (Vc::string(b"a"), Vc::int(1)),
            (Vc::string(b"b"), Vc::int(2)),
        ]),
    ]);
    let rt = roundtrip(&vc);
    assert_eq!(rt.num_elems(), 6);
}

// ============================================================
// Bit-identical wire format tests
// These verify the serialized bytes match what C++ would produce.
// ============================================================

#[test]
fn test_wire_nil() {
    // vc::xfer_out writes type tag "04", nil::xfer_out writes 0 bytes
    assert_eq!(rustvc::to_bytes(&Vc::Nil), b"04");
}

#[test]
fn test_wire_int_42() {
    // type "01", compat_ltoa(42) → "42" (2 bytes), encode_long(2) → "02"
    assert_eq!(rustvc::to_bytes(&Vc::int(42)), b"010242");
}

#[test]
fn test_wire_int_neg_123() {
    // type "01", compat_ltoa(-123) → "-123" (4 bytes), encode_long(4) → "04"
    assert_eq!(rustvc::to_bytes(&Vc::int(-123)), b"0104-123");
}

#[test]
fn test_wire_string_hello() {
    // type "02", encode_long(5) → "015", "hello"
    assert_eq!(rustvc::to_bytes(&Vc::string(b"hello")), b"02015hello");
}

#[test]
fn test_wire_double_3() {
    // type "03", encode_long(1) → "011", "3"
    assert_eq!(rustvc::to_bytes(&Vc::double(3.0)), b"030113");
}

#[test]
fn test_wire_empty_vector() {
    // type "09", encode_long(0) → "010"
    assert_eq!(rustvc::to_bytes(&Vc::vector(vec![])), b"09010");
}

#[test]
fn test_wire_vector_two_ints() {
    // type "09", encode_long(2) → "012", then int(1) + int(2)
    let vc = Vc::vector(vec![Vc::int(1), Vc::int(2)]);
    let bytes = rustvc::to_bytes(&vc);
    assert_eq!(&bytes[..5], b"09012"); // type + count
    // int(1): "01" + "011" (encode_len_2(1)="01" + "1")
    assert_eq!(&bytes[5..10], b"01011");
    // int(2): "01" + "012" (encode_len_2(1)="01" + "2")
    assert_eq!(&bytes[10..15], b"01012");
}

#[test]
fn test_wire_map_one_entry() {
    // type "08", encode_long(1) → "011", then key + value
    let vc = Vc::map(vec![(Vc::string(b"k"), Vc::int(5))]);
    let bytes = rustvc::to_bytes(&vc);
    assert_eq!(&bytes[..5], b"08011"); // type + count
    // key "k": "02" + "011" + "k"
    assert_eq!(&bytes[5..11], b"02011k");
    // value 5: "01" + "015"
    assert_eq!(&bytes[11..16], b"01015");
}

// ============================================================
// CHIT deserialization tests
// Verify we can deserialize C++-style CHIT streams.
// ============================================================

#[test]
fn test_chit_deserialize() {
    // Construct a stream that encodes:
    //   vector(2) { int(42), chit(0) }
    // where chit(0) references the vector itself (index 0 in chit table)
    //
    // In the C++ wire format:
    //   "09" vector type
    //   "012" count = 2
    //   "010242" int(42)
    //   "17" chit type
    //   "010" chit index 0
    let mut data = Vec::new();
    data.extend_from_slice(b"09");       // vector type
    data.extend_from_slice(b"012");      // count = 2
    data.extend_from_slice(b"010242");   // int(42)
    data.extend_from_slice(b"17");       // chit type
    data.extend_from_slice(b"010");      // chit index 0
    // Note: the deserializer will put the vector in the chit table
    // before reading its body, so index 0 refers to the vector.
    // When it hits chit(0), it returns the same vector.
    // This creates a circular reference, which Vc (being owned) can't represent.
    // The deserializer will clone from the chit table.
    // The result is a vector containing int(42) and itself.
    // Since Vc doesn't do Rc, the clone is a fresh copy.
    // The deserializer will succeed but the result won't be truly circular.
    let result = rustvc::from_bytes(&data);
    // This should succeed — the chit lookup should work
    assert!(result.is_ok());
    let vc = result.unwrap();
    assert_eq!(vc.num_elems(), 2);
}

// ============================================================
// Constraint tests
// ============================================================

#[test]
fn test_constraint_max_depth() {
    // A simple int should work with max_depth=1 (int is depth 0 after type read)
    let vc = Vc::int(42);
    let bytes = rustvc::to_bytes(&vc);
    let c = Constraints {
        max_depth: Some(1),
        ..Default::default()
    };
    let result = rustvc::from_bytes_with_constraints(&bytes, &c);
    assert!(result.is_ok());
}

#[test]
fn test_constraint_max_depth_violation() {
    // A vector with max_depth=0 should fail (depth 0 means no nesting allowed)
    let vc = Vc::vector(vec![Vc::int(1)]);
    let bytes = rustvc::to_bytes(&vc);
    let c = Constraints {
        max_depth: Some(0),
        ..Default::default()
    };
    let result = rustvc::from_bytes_with_constraints(&bytes, &c);
    assert!(result.is_err());
}

#[test]
fn test_constraint_max_elements() {
    let vc = Vc::vector(vec![Vc::int(1), Vc::int(2), Vc::int(3)]);
    let bytes = rustvc::to_bytes(&vc);
    let c = Constraints {
        max_elements: Some(2),
        ..Default::default()
    };
    let result = rustvc::from_bytes_with_constraints(&bytes, &c);
    assert!(result.is_err());
}

#[test]
fn test_constraint_max_element_len_string() {
    let vc = Vc::string(b"hello");
    let bytes = rustvc::to_bytes(&vc);
    let c = Constraints {
        max_element_len: Some(3),
        ..Default::default()
    };
    let result = rustvc::from_bytes_with_constraints(&bytes, &c);
    assert!(result.is_err());
}

// ============================================================
// Edge cases
// ============================================================

#[test]
fn test_int_i64_min() {
    let vc = Vc::int(i64::MIN);
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_int(), Some(i64::MIN));
}

#[test]
fn test_int_i64_max() {
    let vc = Vc::int(i64::MAX);
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_int(), Some(i64::MAX));
}

#[test]
fn test_large_string() {
    let s = vec![b'x'; 10000];
    let vc = Vc::string(s.clone());
    let rt = roundtrip(&vc);
    assert_eq!(rt.as_str(), Some(s.as_ref()));
}

#[test]
fn test_deeply_nested_vector() {
    let mut vc = Vc::int(0);
    for _ in 0..50 {
        vc = Vc::vector(vec![vc]);
    }
    let rt = roundtrip(&vc);
    // Navigate down to verify structure
    let mut current = &rt;
    for _ in 0..50 {
        match current {
            Vc::Vector(v) => current = &v[0],
            _ => panic!("expected vector"),
        }
    }
    assert_eq!(current.as_int(), Some(0));
}

// ============================================================
// Trailing data rejection
// ============================================================

#[test]
fn test_trailing_data_rejected() {
    let mut bytes = rustvc::to_bytes(&Vc::int(42));
    bytes.push(b'x'); // trailing junk
    let result = rustvc::from_bytes(&bytes);
    assert!(result.is_err());
}

// ============================================================
// Empty stream
// ============================================================

#[test]
fn test_empty_stream() {
    let result = rustvc::from_bytes(b"");
    assert!(result.is_err());
}

// ============================================================
// Malformed type tag
// ============================================================

#[test]
fn test_invalid_type_tag() {
    let result = rustvc::from_bytes(b"99");
    assert!(result.is_err());
}

// ============================================================
// Type helpers
// ============================================================

#[test]
fn test_vc_type() {
    assert_eq!(Vc::Nil.vc_type(), VcType::Nil);
    assert_eq!(Vc::int(0).vc_type(), VcType::Int);
    assert_eq!(Vc::double(0.0).vc_type(), VcType::Double);
    assert_eq!(Vc::string(b"").vc_type(), VcType::String);
    assert_eq!(Vc::vector(vec![]).vc_type(), VcType::Vector);
    assert_eq!(Vc::map(vec![]).vc_type(), VcType::Map);
    assert_eq!(Vc::set(vec![]).vc_type(), VcType::Set);
    assert_eq!(Vc::bag(vec![]).vc_type(), VcType::Bag);
    assert_eq!(Vc::list(vec![]).vc_type(), VcType::List);
    assert_eq!(Vc::tree(vec![]).vc_type(), VcType::Tree);
}

#[test]
fn test_is_atomic() {
    assert!(Vc::Nil.is_atomic());
    assert!(Vc::int(0).is_atomic());
    assert!(Vc::double(0.0).is_atomic());
    assert!(Vc::string(b"").is_atomic());
    assert!(!Vc::vector(vec![]).is_atomic());
    assert!(!Vc::map(vec![]).is_atomic());
}

#[test]
fn test_display() {
    assert_eq!(format!("{}", Vc::Nil), "nil");
    assert_eq!(format!("{}", Vc::int(42)), "42");
    assert_eq!(format!("{}", Vc::string(b"hello")), "\"hello\"");
}
