pub(crate) const ENCODED_TYPE_LEN: usize = 2;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DecodeError {
    NotEnoughData,
    InvalidDigit,
    Overflow,
}

/// Positive long to ASCII with minimum field width (zero-padded).
/// Mirrors C `pltoa` from vcenco.cpp.
pub(crate) fn pltoa(mut value: u64, field_width: i32) -> String {
    let mut digits = Vec::new();
    let mut remaining_width = field_width;
    loop {
        digits.push((b'0' + (value % 10) as u8) as char);
        value /= 10;
        remaining_width -= 1;
        if value == 0 && remaining_width <= 0 {
            break;
        }
    }
    digits.reverse();
    digits.into_iter().collect()
}

/// Signed long to ASCII, matching C `compat_ltoa` from vcenco.cpp.
/// Produces output compatible with C's sprintf in the C locale.
pub(crate) fn compat_ltoa(value: i64) -> String {
    let neg = value < 0;
    let mut digits = Vec::new();
    let mut v = value;
    loop {
        let rem = v % 10;
        digits.push((b'0' + (if neg { -rem } else { rem }) as u8) as char);
        v /= 10;
        if v == 0 {
            break;
        }
    }
    if neg {
        digits.push('-');
    }
    digits.reverse();
    digits.into_iter().collect()
}

/// Encode a non-negative long as: 2-digit decimal digit-count + decimal digits.
/// Mirrors C `encode_long` from vcenco.cpp.
/// Used for strings, doubles, and decomposable element counts.
pub(crate) fn encode_long(value: u64) -> String {
    let num_str = pltoa(value, -1);
    let len_str = pltoa(num_str.len() as u64, 2);
    format!("{}{}", len_str, num_str)
}

/// Encode a value as exactly 2 ASCII decimal digits (zero-padded).
/// Used for int digit count in vc_int::xfer_out, which uses
/// `pltoa(bytes, buf2, sizeof(buf2), 2)` directly, NOT encode_long.
pub(crate) fn encode_len_2(value: u64) -> [u8; 2] {
    assert!(value <= 99);
    let s = pltoa(value, 2);
    [s.as_bytes()[0], s.as_bytes()[1]]
}

/// Read a 2-digit decimal length prefix. Mirrors C `decode_len`.
pub(crate) fn decode_len(buf: &[u8]) -> Result<usize, DecodeError> {
    if buf.len() < 2 {
        return Err(DecodeError::NotEnoughData);
    }
    if !buf[0].is_ascii_digit() || !buf[1].is_ascii_digit() {
        return Err(DecodeError::InvalidDigit);
    }
    Ok(((buf[0] - b'0') as usize) * 10 + ((buf[1] - b'0') as usize))
}

/// Decode a positive integer from decimal digits with overflow detection.
/// Mirrors C `decode_long` from vcenco.cpp.
pub(crate) fn decode_long(buf: &[u8], len: usize) -> Result<u64, DecodeError> {
    if buf.len() < len {
        return Err(DecodeError::NotEnoughData);
    }
    let mut l: u64 = 0;
    for i in 0..len {
        if !buf[i].is_ascii_digit() {
            return Err(DecodeError::InvalidDigit);
        }
        if l > u64::MAX / 10 {
            return Err(DecodeError::Overflow);
        }
        l *= 10;
        let ndig = (buf[i] - b'0') as u64;
        if l > u64::MAX - ndig {
            return Err(DecodeError::Overflow);
        }
        l += ndig;
    }
    Ok(l)
}

/// Decode a signed decimal integer with overflow detection.
/// Mirrors C `decode_long2` from vcenco.cpp.
pub(crate) fn decode_long2(buf: &[u8], len: usize) -> Result<i64, DecodeError> {
    if buf.len() < len {
        return Err(DecodeError::NotEnoughData);
    }
    if len == 0 {
        return Err(DecodeError::NotEnoughData);
    }
    let neg = buf[0] == b'-';
    let start = if neg { 1 } else { 0 };
    if neg && len == 1 {
        return Err(DecodeError::InvalidDigit);
    }
    let mut l: u64 = 0;
    for i in start..len {
        if !buf[i].is_ascii_digit() {
            return Err(DecodeError::InvalidDigit);
        }
        if l > u64::MAX / 10 {
            return Err(DecodeError::Overflow);
        }
        l *= 10;
        let ndig = (buf[i] - b'0') as u64;
        if l > u64::MAX - ndig {
            return Err(DecodeError::Overflow);
        }
        l += ndig;
    }
    if neg {
        if l > i64::MAX as u64 + 1 {
            return Err(DecodeError::Overflow);
        }
        // Special-case i64::MIN: l == i64::MAX as u64 + 1
        // Can't negate through i64, so return directly.
        if l == i64::MAX as u64 + 1 {
            return Ok(i64::MIN);
        }
        Ok(-(l as i64))
    } else {
        if l > i64::MAX as u64 {
            return Err(DecodeError::Overflow);
        }
        Ok(l as i64)
    }
}

/// Encode a type tag as 2 ASCII decimal digits.
/// Mirrors C `encode_type` from vcenco.cpp.
pub(crate) fn encode_type(t: u8) -> [u8; 2] {
    [b'0' + t / 10, b'0' + t % 10]
}

/// Decode a 2-digit type tag. Mirrors C `decode_type`.
pub(crate) fn decode_type(buf: &[u8]) -> Result<u8, DecodeError> {
    if buf.len() < 2 {
        return Err(DecodeError::NotEnoughData);
    }
    if !buf[0].is_ascii_digit() || !buf[1].is_ascii_digit() {
        return Err(DecodeError::InvalidDigit);
    }
    Ok((buf[0] - b'0') * 10 + (buf[1] - b'0'))
}

/// Format a float to match C's `snprintf(buf, bufsz, "%.*g", 1024, d)`.
/// Uses dtoa for the core conversion, then post-processes to match C %g rules.
pub(crate) fn format_double(value: f64) -> String {
    let mut buf = dtoa::Buffer::new();
    let s = buf.format(value).to_string();
    post_process_g_fmt(&s)
}

/// Post-process a float string to match C %g behavior:
/// strip trailing zeros after decimal point, strip decimal point if nothing follows.
fn post_process_g_fmt(s: &str) -> String {
    if let Some(dot_pos) = s.find('.') {
        // Has a decimal point — strip trailing zeros and the point itself if empty fractional part
        let after_dot = &s[dot_pos + 1..];
        // Check if it's an exponent format like "1.23e+10"
        let frac_end = if let Some(e_pos) = after_dot.find(&['e', 'E'][..]) {
            e_pos
        } else {
            after_dot.len()
        };
        let frac_part = &after_dot[..frac_end];
        let num_trailing_zeros = frac_part.chars().rev().take_while(|&c| c == '0').count();
        if num_trailing_zeros == frac_part.len() {
            // All fractional digits are zeros — strip decimal point and everything after
            let int_part = &s[..dot_pos];
            let suffix = &after_dot[frac_end..];
            format!("{}{}", int_part, suffix)
        } else if num_trailing_zeros > 0 {
            // Strip only the trailing zeros
            let keep = s.len() - num_trailing_zeros;
            s[..keep].to_string()
        } else {
            s.to_string()
        }
    } else {
        s.to_string()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_pltoa() {
        assert_eq!(pltoa(0, -1), "0");
        assert_eq!(pltoa(123, -1), "123");
        assert_eq!(pltoa(1, 2), "01");
        assert_eq!(pltoa(3, 2), "03");
        assert_eq!(pltoa(100, -1), "100");
    }

    #[test]
    fn test_compat_ltoa() {
        assert_eq!(compat_ltoa(0), "0");
        assert_eq!(compat_ltoa(123), "123");
        assert_eq!(compat_ltoa(-123), "-123");
        assert_eq!(compat_ltoa(1), "1");
        assert_eq!(compat_ltoa(-1), "-1");
    }

    #[test]
    fn test_encode_long() {
        assert_eq!(encode_long(0), "010");
        assert_eq!(encode_long(5), "015");
        assert_eq!(encode_long(42), "0242");
        assert_eq!(encode_long(123), "03123");
        assert_eq!(encode_long(100), "03100");
    }

    #[test]
    fn test_decode_len() {
        assert_eq!(decode_len(b"01").unwrap(), 1);
        assert_eq!(decode_len(b"03").unwrap(), 3);
        assert_eq!(decode_len(b"00").unwrap(), 0);
        assert!(decode_len(b"a1").is_err());
    }

    #[test]
    fn test_decode_long() {
        assert_eq!(decode_long(b"0", 1).unwrap(), 0);
        assert_eq!(decode_long(b"123", 3).unwrap(), 123);
        assert_eq!(decode_long(b"42", 2).unwrap(), 42);
        assert!(decode_long(b"abc", 3).is_err());
    }

    #[test]
    fn test_decode_long2() {
        assert_eq!(decode_long2(b"0", 1).unwrap(), 0);
        assert_eq!(decode_long2(b"123", 3).unwrap(), 123);
        assert_eq!(decode_long2(b"-123", 4).unwrap(), -123);
        assert_eq!(decode_long2(b"-1", 2).unwrap(), -1);
        assert!(decode_long2(b"-", 1).is_err());
        assert!(decode_long2(b"abc", 3).is_err());
    }

    #[test]
    fn test_encode_type() {
        assert_eq!(encode_type(1), *b"01");
        assert_eq!(encode_type(9), *b"09");
        assert_eq!(encode_type(10), *b"10");
        assert_eq!(encode_type(31), *b"31");
    }

    #[test]
    fn test_decode_type() {
        assert_eq!(decode_type(b"01").unwrap(), 1);
        assert_eq!(decode_type(b"09").unwrap(), 9);
        assert_eq!(decode_type(b"10").unwrap(), 10);
        assert_eq!(decode_type(b"31").unwrap(), 31);
        assert!(decode_type(b"a1").is_err());
    }

    #[test]
    fn test_format_double() {
        assert_eq!(format_double(3.0), "3");
        assert_eq!(format_double(3.14), "3.14");
        assert_eq!(format_double(0.0), "0");
        assert_eq!(format_double(1.5), "1.5");
        // C's %g: exponent 1 < 1024, so %f format wins → "10", not "1e1"
        assert_eq!(format_double(10.0), "10");
        assert_eq!(format_double(-2.5), "-2.5");
        // C's %g: exponent 2 < 1024, so %f format wins → "100"
        assert_eq!(format_double(100.0), "100");
        assert_eq!(format_double(0.1), "0.1");
    }
}
