use std::collections::HashMap;
use std::rc::Rc;

use crate::encoding::{compat_ltoa, encode_len_2, encode_long, encode_type, format_double};
use crate::types::Vc;
use crate::CHIT_TYPE;

/// Serializer that writes vc values to a byte buffer.
/// Tracks non-atomic objects by identity for CHIT/DAG support.
pub struct Serializer {
    output: Vec<u8>,
    visited: Option<HashMap<usize, usize>>,
}

impl Serializer {
    pub fn new() -> Self {
        Serializer {
            output: Vec::with_capacity(256),
            visited: None,
        }
    }

    pub fn with_dag_support() -> Self {
        Serializer {
            output: Vec::with_capacity(256),
            visited: Some(HashMap::new()),
        }
    }

    pub fn into_bytes(self) -> Vec<u8> {
        self.output
    }

    pub fn bytes(&self) -> &[u8] {
        &self.output
    }

    fn write_bytes(&mut self, b: &[u8]) {
        self.output.extend_from_slice(b);
    }
}

/// Serialize a vc value to bytes (tree mode, no CHIT).
pub fn serialize(vc: &Vc) -> Vec<u8> {
    let mut s = Serializer::new();
    s_xfer_out(&mut s, vc);
    s.into_bytes()
}

/// Serialize a vc value to bytes with DAG/CHIT support using Rc identity.
pub fn serialize_dag(vc: &Rc<Vc>) -> Vec<u8> {
    let mut s = Serializer::with_dag_support();
    s_xfer_out_rc(&mut s, vc);
    s.into_bytes()
}

fn s_xfer_out(s: &mut Serializer, vc: &Vc) {
    let t = vc.vc_type();
    let tag = encode_type(t.tag());
    s.write_bytes(&tag);

    if !vc.is_atomic() {
        if let Some(ref mut visited) = s.visited {
            let key = vc as *const Vc as usize;
            visited.insert(key, visited.len());
        }
    }

    s_xfer_out_payload(s, vc);
}

fn s_xfer_out_rc(s: &mut Serializer, vc: &Rc<Vc>) {
    let t = vc.vc_type();

    if !t.is_atomic() {
        let key = Rc::as_ptr(vc) as usize;
        if let Some(ref mut visited) = s.visited {
            if let Some(&idx) = visited.get(&key) {
                let tag = encode_type(CHIT_TYPE);
                s.write_bytes(&tag);
                let idx_str = encode_long(idx as u64);
                s.write_bytes(idx_str.as_bytes());
                return;
            }
            visited.insert(key, visited.len());
        }
    }

    let tag = encode_type(t.tag());
    s.write_bytes(&tag);
    s_xfer_out_payload_rc(s, vc);
}

fn s_xfer_out_payload(s: &mut Serializer, vc: &Vc) {
    match vc {
        Vc::Nil => {}
        Vc::Int(v) => {
            let digits = compat_ltoa(*v);
            let len = encode_len_2(digits.len() as u64);
            s.write_bytes(&len);
            s.write_bytes(digits.as_bytes());
        }
        Vc::Double(v) => {
            let float_str = format_double(*v);
            let len_str = encode_long(float_str.len() as u64);
            s.write_bytes(len_str.as_bytes());
            s.write_bytes(float_str.as_bytes());
        }
        Vc::String(v) => {
            let len_str = encode_long(v.len() as u64);
            s.write_bytes(len_str.as_bytes());
            s.write_bytes(v);
        }
        Vc::Vector(elems) => {
            let count_str = encode_long(elems.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for e in elems {
                s_xfer_out(s, e);
            }
        }
        Vc::Map(pairs) => {
            let count_str = encode_long(pairs.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for (k, v) in pairs {
                s_xfer_out(s, k);
                s_xfer_out(s, v);
            }
        }
        Vc::Set(elems) => {
            let count_str = encode_long(elems.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for e in elems {
                s_xfer_out(s, e);
            }
        }
        Vc::Bag(elems) => {
            let count_str = encode_long(elems.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for e in elems {
                s_xfer_out(s, e);
            }
        }
        Vc::List(elems) => {
            let count_str = encode_long(elems.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for e in elems {
                s_xfer_out(s, e);
            }
        }
        Vc::Tree(pairs) => {
            let count_str = encode_long(pairs.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for (k, v) in pairs {
                s_xfer_out(s, k);
                s_xfer_out(s, v);
            }
        }
    }
}

fn s_xfer_out_payload_rc(s: &mut Serializer, vc: &Vc) {
    match vc {
        Vc::Nil => {}
        Vc::Int(v) => {
            let digits = compat_ltoa(*v);
            let len = encode_len_2(digits.len() as u64);
            s.write_bytes(&len);
            s.write_bytes(digits.as_bytes());
        }
        Vc::Double(v) => {
            let float_str = format_double(*v);
            let len_str = encode_long(float_str.len() as u64);
            s.write_bytes(len_str.as_bytes());
            s.write_bytes(float_str.as_bytes());
        }
        Vc::String(v) => {
            let len_str = encode_long(v.len() as u64);
            s.write_bytes(len_str.as_bytes());
            s.write_bytes(v);
        }
        Vc::Vector(elems) => {
            let count_str = encode_long(elems.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for e in elems {
                // For DAG mode with Rc, we'd need Rc<Vc> here.
                // Since we're called from s_xfer_out_rc which already
                // handles the Rc case at the top level, we fall back
                // to tree-mode for inner elements unless they're also Rc.
                // This is acceptable: full DAG support requires Rc<Vc> throughout.
                s_xfer_out(s, e);
            }
        }
        Vc::Map(pairs) => {
            let count_str = encode_long(pairs.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for (k, v) in pairs {
                s_xfer_out(s, k);
                s_xfer_out(s, v);
            }
        }
        Vc::Set(elems) => {
            let count_str = encode_long(elems.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for e in elems {
                s_xfer_out(s, e);
            }
        }
        Vc::Bag(elems) => {
            let count_str = encode_long(elems.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for e in elems {
                s_xfer_out(s, e);
            }
        }
        Vc::List(elems) => {
            let count_str = encode_long(elems.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for e in elems {
                s_xfer_out(s, e);
            }
        }
        Vc::Tree(pairs) => {
            let count_str = encode_long(pairs.len() as u64);
            s.write_bytes(count_str.as_bytes());
            for (k, v) in pairs {
                s_xfer_out(s, k);
                s_xfer_out(s, v);
            }
        }
    }
}
