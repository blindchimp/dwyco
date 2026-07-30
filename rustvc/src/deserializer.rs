use crate::encoding::{
    decode_len, decode_long, decode_long2, decode_type, DecodeError, ENCODED_TYPE_LEN,
};
use crate::types::{Vc, VcType};

/// Constraints for deserialization, mirroring C++ vcxstream limits.
#[derive(Debug, Clone)]
pub struct Constraints {
    pub max_depth: Option<usize>,
    pub max_elements: Option<usize>,
    pub max_element_len: Option<usize>,
    pub max_memory: Option<usize>,
}

impl Default for Constraints {
    fn default() -> Self {
        Constraints {
            max_depth: None,
            max_elements: None,
            max_element_len: None,
            max_memory: None,
        }
    }
}

#[derive(Debug)]
pub enum VcError {
    DeviceError,
    ParseError,
    ConstraintViolation(&'static str),
}

impl std::fmt::Display for VcError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            VcError::DeviceError => write!(f, "device error: not enough data in stream"),
            VcError::ParseError => write!(f, "parse error"),
            VcError::ConstraintViolation(msg) => write!(f, "constraint violation: {}", msg),
        }
    }
}

impl std::error::Error for VcError {}

impl From<DecodeError> for VcError {
    fn from(e: DecodeError) -> Self {
        match e {
            DecodeError::NotEnoughData => VcError::DeviceError,
            _ => VcError::ParseError,
        }
    }
}

struct Deserializer<'a> {
    data: &'a [u8],
    pos: usize,
    chit_table: Vec<Vc>,
    depth: usize,
    memory_tally: usize,
    constraints: Constraints,
}

impl<'a> Deserializer<'a> {
    fn new(data: &'a [u8], constraints: Constraints) -> Self {
        Deserializer {
            data,
            pos: 0,
            chit_table: Vec::new(),
            depth: 0,
            memory_tally: 0,
            constraints,
        }
    }

    fn peek(&self, n: usize) -> Result<&'a [u8], VcError> {
        if self.pos + n > self.data.len() {
            return Err(VcError::DeviceError);
        }
        Ok(&self.data[self.pos..self.pos + n])
    }

    fn advance(&mut self, n: usize) -> Result<(), VcError> {
        if self.pos + n > self.data.len() {
            return Err(VcError::DeviceError);
        }
        self.pos += n;
        Ok(())
    }

    fn check_depth(&mut self) -> Result<(), VcError> {
        if let Some(max) = self.constraints.max_depth {
            if self.depth >= max {
                return Err(VcError::ConstraintViolation("max_depth"));
            }
        }
        Ok(())
    }

    fn add_memory(&mut self, n: usize) -> Result<(), VcError> {
        self.memory_tally += n;
        if let Some(max) = self.constraints.max_memory {
            if self.memory_tally >= max {
                return Err(VcError::ConstraintViolation("max_memory"));
            }
        }
        Ok(())
    }

    /// Read the 2-byte type tag without consuming it (peek).
    fn peek_type_tag(&mut self) -> Result<VcType, VcError> {
        let buf = self.peek(ENCODED_TYPE_LEN)?;
        let tag = decode_type(buf)?;
        VcType::from_u8(tag).ok_or(VcError::ParseError)
    }

    fn consume_type_tag(&mut self) -> Result<VcType, VcError> {
        let t = self.peek_type_tag()?;
        self.advance(ENCODED_TYPE_LEN)?;
        Ok(t)
    }

    /// Read an encode_long value (2-digit length prefix + decimal digits).
    fn read_encode_long_value(&mut self) -> Result<u64, VcError> {
        let len_buf = self.peek(2)?;
        let len = decode_len(len_buf)?;
        self.advance(2)?;
        if len == 0 {
            return Err(VcError::ParseError);
        }
        let val_buf = self.peek(len)?;
        let val = decode_long(val_buf, len)?;
        self.advance(len)?;
        Ok(val)
    }

    fn read_encode_long_value_usize(&mut self) -> Result<usize, VcError> {
        self.read_encode_long_value().map(|v| v as usize)
    }

    /// Top-level deserialization with depth tracking.
    /// Mirrors C++ vc::xfer_in which wraps real_xfer_in with depth/memory checking.
    fn xfer_in(&mut self) -> Result<Vc, VcError> {
        self.check_depth()?;
        self.depth += 1;
        let result = self.real_xfer_in();
        self.depth -= 1;
        result
    }

    /// Core deserialization. Mirrors C++ vc::real_xfer_in.
    fn real_xfer_in(&mut self) -> Result<Vc, VcError> {
        let t = self.consume_type_tag()?;

        match t {
            VcType::Nil => {
                self.add_memory(std::mem::size_of::<Vc>())?;
                Ok(Vc::Nil)
            }

            VcType::Int => {
                let len_buf = self.peek(2)?;
                let len = decode_len(len_buf)?;
                self.advance(2)?;
                if len == 0 {
                    return Err(VcError::ParseError);
                }
                if let Some(max) = self.constraints.max_element_len {
                    if len > max {
                        return Err(VcError::ConstraintViolation("max_element_len (int)"));
                    }
                }
                let val_buf = self.peek(len)?;
                let val = decode_long2(val_buf, len)?;
                self.advance(len)?;
                self.add_memory(std::mem::size_of::<Vc>())?;
                Ok(Vc::Int(val))
            }

            VcType::Double => {
                let flen = self.read_encode_long_value_usize()?;
                if flen == 0 {
                    return Err(VcError::ParseError);
                }
                if let Some(max) = self.constraints.max_element_len {
                    if flen > max {
                        return Err(VcError::ConstraintViolation("max_element_len (double)"));
                    }
                }
                let float_buf = self.peek(flen)?;
                let float_str =
                    std::str::from_utf8(float_buf).map_err(|_| VcError::ParseError)?;
                let val: f64 = float_str.parse().map_err(|_| VcError::ParseError)?;
                self.advance(flen)?;
                self.add_memory(std::mem::size_of::<Vc>())?;
                Ok(Vc::Double(val))
            }

            VcType::String => {
                let slen = self.read_encode_long_value_usize()?;
                if let Some(max) = self.constraints.max_element_len {
                    if slen > max {
                        return Err(VcError::ConstraintViolation("max_element_len (string)"));
                    }
                }
                let str_buf = self.peek(slen)?;
                let s = str_buf.to_vec();
                self.advance(slen)?;
                self.add_memory(std::mem::size_of::<Vc>() + slen)?;
                Ok(Vc::String(s))
            }

            VcType::Chit => {
                let idx = self.read_encode_long_value()? as usize;
                if idx >= self.chit_table.len() {
                    return Err(VcError::ParseError);
                }
                Ok(self.chit_table[idx].clone())
            }

            VcType::Vector | VcType::Set | VcType::Bag | VcType::List => {
                self.read_sequence(t)
            }

            VcType::Map | VcType::Tree => self.read_map(t),
        }
    }

    /// Read a sequence type (Vector, Set, Bag, List).
    /// Mirrors C++ vc_decomposable::xfer_in → decode_numelems + decode_elems.
    fn read_sequence(&mut self, vctype: VcType) -> Result<Vc, VcError> {
        let nelems = self.read_encode_long_value_usize()?;
        if let Some(max) = self.constraints.max_elements {
            if nelems > max {
                return Err(VcError::ConstraintViolation("max_elements"));
            }
        }
        self.add_memory(std::mem::size_of::<Vc>())?;
        // Add placeholder to chit table before reading contents,
        // matching C++ vc::real_xfer_in which calls chit_append
        // before rep->xfer_in. This allows CHIT refs to the container.
        let placeholder = Vc::Vector(Vec::new());
        let chit_idx = self.chit_table.len();
        self.chit_table.push(placeholder);
        let mut elems = Vec::with_capacity(nelems);
        for _ in 0..nelems {
            let e = self.xfer_in()?;
            elems.push(e);
        }
        let result = match vctype {
            VcType::Vector => Vc::Vector(elems),
            VcType::Set => Vc::Set(elems),
            VcType::Bag => Vc::Bag(elems),
            VcType::List => Vc::List(elems),
            _ => unreachable!(),
        };
        self.chit_table[chit_idx] = result.clone();
        Ok(result)
    }

    /// Read a map type (Map, Tree).
    /// Mirrors C++ vc_map::decode_elems / vc_tree::decode_elems.
    fn read_map(&mut self, vctype: VcType) -> Result<Vc, VcError> {
        let nelems = self.read_encode_long_value_usize()?;
        if let Some(max) = self.constraints.max_elements {
            if nelems > max {
                return Err(VcError::ConstraintViolation("max_elements"));
            }
        }
        self.add_memory(std::mem::size_of::<Vc>())?;
        let placeholder = Vc::Map(Vec::new());
        let chit_idx = self.chit_table.len();
        self.chit_table.push(placeholder);
        let mut pairs = Vec::with_capacity(nelems);
        for _ in 0..nelems {
            let k = self.xfer_in()?;
            let v = self.xfer_in()?;
            pairs.push((k, v));
        }
        let result = match vctype {
            VcType::Map => Vc::Map(pairs),
            VcType::Tree => Vc::Tree(pairs),
            _ => unreachable!(),
        };
        self.chit_table[chit_idx] = result.clone();
        Ok(result)
    }
}

/// Deserialize a single vc value from bytes.
pub fn deserialize(data: &[u8]) -> Result<Vc, VcError> {
    deserialize_with_constraints(data, &Constraints::default())
}

/// Deserialize a single vc value from bytes with constraints.
pub fn deserialize_with_constraints(
    data: &[u8],
    constraints: &Constraints,
) -> Result<Vc, VcError> {
    let mut d = Deserializer::new(data, constraints.clone());
    let vc = d.xfer_in()?;
    if d.pos != d.data.len() {
        return Err(VcError::ParseError);
    }
    Ok(vc)
}

/// Deserialize multiple vc values from a byte buffer.
pub fn deserialize_many(
    data: &[u8],
    constraints: &Constraints,
) -> Result<(Vec<Vc>, usize), VcError> {
    let mut d = Deserializer::new(data, constraints.clone());
    let mut results = Vec::new();
    while d.pos < d.data.len() {
        let vc = d.xfer_in()?;
        results.push(vc);
    }
    Ok((results, d.pos))
}
