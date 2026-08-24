package dwvc

/*
 * enc.go — simple XOR encryption/decryption ported from vc/enc.cpp
 *
 * The encryption is its own inverse: munge(buf, n) followed by munge(buf, n)
 * restores the original text.  The key rolling index is preserved across calls
 * to match the C++ rolling-offset semantics exactly.
 */

// Enc represents the same XOR encryptor as the C++ ns_vc::Enc class.
type Enc struct {
	key    string
	keylen int
	keypos int
}

// NewEnc returns a new Enc instance with the standard "fooBarMumbleGrunch" key.
func NewEnc() *Enc {
	return &Enc{
		key:    "fooBarMumbleGrunch",
		keylen: 18, // len("fooBarMumbleGrunch") - 1 + 1, i.e. sizeof(key)-1 in C is 19-1 = 18
		keypos: 0,
	}
}

// Munge performs in-place XOR encryption on buf[0:n] and advances the key index.
// The ciphertext produced can be recovered by a subsequent Munge call with the
// same Enc instance.  The index rolls over when it reaches keylen.
func (e *Enc) Munge(buf []byte) {
	for i := 0; i < len(buf); i++ {
		e.keypos++
		buf[i] ^= byte(e.key[(e.keypos-1)%e.keylen]) + 'A'
	}
}

// Mungeback performs in-place XOR decryption on buf[0:n] and rewinds the key index.
// It is the inverse of Munge: Munge then Mungeback restores the original bytes.
// The index decrements before use to match the C++ --keychar semantics exactly.
func (e *Enc) Mungeback(buf []byte) {
	for i := 0; i < len(buf); i++ {
		e.keypos--
		k := int(e.keypos) % e.keylen
		if k < 0 {
			k += e.keylen
		}
		buf[len(buf)-1-i] ^= byte(e.key[k]) + 'A'
	}
}

// Reset key position to zero.  Useful for re-keying or resetting between
// independent encrypted streams.
func (e *Enc) Reset() {
	e.keypos = 0
}