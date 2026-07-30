pub mod encoding;
pub mod types;
pub mod serializer;
pub mod deserializer;

pub use types::{Vc, VcType};
pub use deserializer::{Constraints, VcError};
pub use serializer::{serialize, serialize_dag};
pub use deserializer::{deserialize, deserialize_with_constraints, deserialize_many};

pub(crate) const CHIT_TYPE: u8 = 17;

/// Serialize a vc value to bytes.
pub fn to_bytes(vc: &Vc) -> Vec<u8> {
    serialize(vc)
}

/// Deserialize a vc value from bytes.
pub fn from_bytes(data: &[u8]) -> Result<Vc, VcError> {
    deserialize(data)
}

/// Deserialize a vc value from bytes with constraints.
pub fn from_bytes_with_constraints(
    data: &[u8],
    constraints: &Constraints,
) -> Result<Vc, VcError> {
    deserialize_with_constraints(data, constraints)
}
