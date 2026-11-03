package dwvc

/*
 * grows.go — minimal DwGrowingString port used by the lexer.
 *
 * The C++ DwGrowingString uses a DwVec<char> with mark/pop_to_mark/toss_mark
 * functionality.  We replicate only the subset needed by vclex:
 *   append, mark, popToMark, tossMark, length, refBytes, reset.
 *
 * popToMark returns a slice that aliases the internal buffer at the popped
 * position.  The caller must consume it before any operation that could
 * reallocate the backing array (the same constraint that applies to the C++
 * version when marks interact with auto-expansion).
 */

// growingString is the Go equivalent of DwGrowingString.
type growingString struct {
	str     []byte // underlying buffer; capacity may exceed curlen
	curlen  int    // logical length (next write position at curlen)
	nmark   int    // number of marks on the stack
	markers [5]int // mark stack; NMARKS == 5 from the C++ header
}

// newGrowingString allocates an initial buffer of the given size.
func newGrowingString(size int) *growingString {
	return &growingString{str: make([]byte, 0, size)}
}

// append appends s[0:n] to the growing string at position curlen.
func (gs *growingString) append(s []byte, n int) {
	if gs.curlen+n > len(gs.str) {
		newCap := gs.curlen + n
		if newCap > cap(gs.str) {
			newData := make([]byte, newCap)
			copy(newData, gs.str)
			gs.str = newData
		} else {
			gs.str = gs.str[:newCap]
		}
	}
	copy(gs.str[gs.curlen:], s[:n])
	gs.curlen += n
}

// reset reinitialises the growing string to the empty state.
func (gs *growingString) reset() {
	gs.curlen = 0
	gs.nmark = 0
}

// length returns the current logical length.
func (gs *growingString) length() int {
	return gs.curlen
}

// refBytes returns a slice aliasing the internal buffer [0:curlen].
func (gs *growingString) refBytes() []byte {
	return gs.str[:gs.curlen]
}

// mark pushes the current curlen onto the mark stack.
func (gs *growingString) mark() {
	if gs.nmark >= 5 {
		panic("growingString mark overflow")
	}
	gs.markers[gs.nmark] = gs.curlen
	gs.nmark++
}

// popToMark pops the most recent mark and returns a slice into the
// internal buffer [newCurlen:oldCurlen] plus the new curlen (which
// becomes the popped length).  The caller must use the returned slice
// before any operation that might reallocate gs.str.
func (gs *growingString) popToMark() ([]byte, int) {
	if gs.nmark <= 0 {
		panic("growingString mark underflow")
	}
	gs.nmark--
	newCurlen := gs.markers[gs.nmark]
	oldCurlen := gs.curlen
	buf := gs.str[newCurlen:oldCurlen]
	gs.curlen = newCurlen
	return buf, oldCurlen - newCurlen
}

// tossMark discards the most recent mark without returning the slice.
func (gs *growingString) tossMark() {
	if gs.nmark <= 0 {
		panic("growingString mark underflow")
	}
	gs.nmark--
}