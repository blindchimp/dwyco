use std::fmt;

/// Type tags matching the C++ vc_type enum from vc.h.
/// DO NOT REORDER OR CHANGE THESE NUMBERS — they are encoded in streams.
#[repr(u8)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub enum VcType {
    Int = 1,
    String = 2,
    Double = 3,
    Nil = 4,
    Vector = 9,
    Map = 8,
    Set = 7,
    Bag = 11,
    List = 10,
    Tree = 31,
    Chit = 17,
}

impl VcType {
    pub fn from_u8(v: u8) -> Option<VcType> {
        match v {
            1 => Some(VcType::Int),
            2 => Some(VcType::String),
            3 => Some(VcType::Double),
            4 => Some(VcType::Nil),
            7 => Some(VcType::Set),
            8 => Some(VcType::Map),
            9 => Some(VcType::Vector),
            10 => Some(VcType::List),
            11 => Some(VcType::Bag),
            17 => Some(VcType::Chit),
            31 => Some(VcType::Tree),
            _ => None,
        }
    }

    pub fn tag(self) -> u8 {
        self as u8
    }

    pub fn is_atomic(self) -> bool {
        matches!(self, VcType::Int | VcType::Double | VcType::String | VcType::Nil)
    }
}

/// A vc value. Corresponds to the C++ vc class for serialization purposes.
#[derive(Debug, Clone, PartialEq)]
pub enum Vc {
    Nil,
    Int(i64),
    Double(f64),
    String(Vec<u8>),
    Vector(Vec<Vc>),
    Map(Vec<(Vc, Vc)>),
    Set(Vec<Vc>),
    Bag(Vec<Vc>),
    List(Vec<Vc>),
    Tree(Vec<(Vc, Vc)>),
}

impl Vc {
    pub fn nil() -> Self {
        Vc::Nil
    }

    pub fn int(v: i64) -> Self {
        Vc::Int(v)
    }

    pub fn double(v: f64) -> Self {
        Vc::Double(v)
    }

    pub fn string(v: impl Into<Vec<u8>>) -> Self {
        Vc::String(v.into())
    }

    pub fn vector(v: Vec<Vc>) -> Self {
        Vc::Vector(v)
    }

    pub fn map(v: Vec<(Vc, Vc)>) -> Self {
        Vc::Map(v)
    }

    pub fn set(v: Vec<Vc>) -> Self {
        Vc::Set(v)
    }

    pub fn bag(v: Vec<Vc>) -> Self {
        Vc::Bag(v)
    }

    pub fn list(v: Vec<Vc>) -> Self {
        Vc::List(v)
    }

    pub fn tree(v: Vec<(Vc, Vc)>) -> Self {
        Vc::Tree(v)
    }

    pub fn vc_type(&self) -> VcType {
        match self {
            Vc::Nil => VcType::Nil,
            Vc::Int(_) => VcType::Int,
            Vc::Double(_) => VcType::Double,
            Vc::String(_) => VcType::String,
            Vc::Vector(_) => VcType::Vector,
            Vc::Map(_) => VcType::Map,
            Vc::Set(_) => VcType::Set,
            Vc::Bag(_) => VcType::Bag,
            Vc::List(_) => VcType::List,
            Vc::Tree(_) => VcType::Tree,
        }
    }

    pub fn is_atomic(&self) -> bool {
        self.vc_type().is_atomic()
    }

    pub fn as_int(&self) -> Option<i64> {
        match self {
            Vc::Int(v) => Some(*v),
            _ => None,
        }
    }

    pub fn as_double(&self) -> Option<f64> {
        match self {
            Vc::Double(v) => Some(*v),
            _ => None,
        }
    }

    pub fn as_str(&self) -> Option<&[u8]> {
        match self {
            Vc::String(v) => Some(v),
            _ => None,
        }
    }

    pub fn num_elems(&self) -> usize {
        match self {
            Vc::Nil => 0,
            Vc::Vector(v) => v.len(),
            Vc::Map(v) => v.len(),
            Vc::Set(v) => v.len(),
            Vc::Bag(v) => v.len(),
            Vc::List(v) => v.len(),
            Vc::Tree(v) => v.len(),
            _ => 0,
        }
    }
}

impl fmt::Display for Vc {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Vc::Nil => write!(f, "nil"),
            Vc::Int(v) => write!(f, "{}", v),
            Vc::Double(v) => write!(f, "{}", v),
            Vc::String(v) => {
                write!(f, "\"")?;
                for &b in v {
                    if b.is_ascii_graphic() || b == b' ' {
                        write!(f, "{}", b as char)?;
                    } else {
                        write!(f, "\\x{:02x}", b)?;
                    }
                }
                write!(f, "\"")
            }
            Vc::Vector(v) => {
                write!(f, "vector(")?;
                for (i, e) in v.iter().enumerate() {
                    if i > 0 {
                        write!(f, " ")?;
                    }
                    write!(f, "{}", e)?;
                }
                write!(f, ")")
            }
            Vc::Map(v) => {
                write!(f, "map(")?;
                for (i, (k, val)) in v.iter().enumerate() {
                    if i > 0 {
                        write!(f, " ")?;
                    }
                    write!(f, "vector({} {})", k, val)?;
                }
                write!(f, ")")
            }
            Vc::Set(v) => {
                write!(f, "set(")?;
                for (i, e) in v.iter().enumerate() {
                    if i > 0 {
                        write!(f, " ")?;
                    }
                    write!(f, "{}", e)?;
                }
                write!(f, ")")
            }
            Vc::Bag(v) => {
                write!(f, "bag(")?;
                for (i, e) in v.iter().enumerate() {
                    if i > 0 {
                        write!(f, " ")?;
                    }
                    write!(f, "{}", e)?;
                }
                write!(f, ")")
            }
            Vc::List(v) => {
                write!(f, "list(")?;
                for (i, e) in v.iter().enumerate() {
                    if i > 0 {
                        write!(f, " ")?;
                    }
                    write!(f, "{}", e)?;
                }
                write!(f, ")")
            }
            Vc::Tree(v) => {
                write!(f, "tree(")?;
                for (i, (k, val)) in v.iter().enumerate() {
                    if i > 0 {
                        write!(f, " ")?;
                    }
                    write!(f, "vector({} {})", k, val)?;
                }
                write!(f, ")")
            }
        }
    }
}
