package dwvc

import "io"

// Token represents the class of a recognised token.
type Token int

const (
	// BOGUS is returned when no token could be recognised.
	TokBogus Token = iota
	// ATOM is a symbolic atom (identifier-like token).
	TokAtom
	// LSBRACK is '['.
	TokLSBrack
	// RSBRACK is ']'.
	TokRSBrack
	// LBrace is '{'.
	TokLBrace
	// RBrace is '}'.
	TokRBrace
	// LParen is '('.
	TokLParen
	// RParen is ')'.
	TokRParen
	// LTick is '`'.
	TokLTick
	// RTick is ''''.
	TokRTick
	// UpArrow is '^'.
	TokUpArrow
	// LBracket is '<'.
	TokLBracket
	// RBracket is '>'.
	TokRBracket
	// EOS end-of-stream.
	TokEOS
)

// Atom is the type of an ATOM token.
type Atom int

const (
	// BOGUS_ATOM is an undefined atom type.
	AtomBogus Atom = iota
	// STRING is a string literal or symbolic string.
	AtomString
	// INTEGER is an integer token.
	AtomInteger
	// FLOAT is a floating-point token.
	AtomFloat
)

// Lexer is the Go equivalent of C++ VcLexer.
// It lexes LH language source input, producing tokens, atoms, and token values.
type Lexer struct {
	src lexerSource

	// Token state
	tokval growingString // accumulated token value (alias via refBytes)
	base   int           // base if integer recognised
	curbase int          // current base during integer parsing
	token  Token         // set to token class
	atype  Atom          // if token == ATOM, gives type

	// Error / warning control
	LexicalError        bool
	EmitLexicalWarnings bool

	// Source tracking
	linesRead   int64
	startScan   int64
	startToken  int64
	endToken    int64
	inpDesc     string
	curCharIndex int64

	// Character tracking for state machine
	curChar byte // last safe character read
}

// lexerSource is the interface replacing the C++ pure-virtual get_chars/put_back/no_more_available.
type lexerSource interface {
	getChars(want int) []byte
	putBack(b []byte)
	noMoreAvailable() bool
}

// NewStringLexer creates a new lexer reading from the given byte slice.
// errOut is the writer for lexical error/warning messages.
func NewStringLexer(data []byte, errOut io.Writer) *Lexer {
	gs := newGrowingString(len(data))
	return &Lexer{
		src:                &stringSource{str: data},
		tokval:             *gs,
		LexicalError:       false,
		EmitLexicalWarnings: false,
		linesRead:          0,
		startScan:        0,
		startToken:       0,
		endToken:         0,
		curCharIndex:     0,
		errOut:             errOut,
	}
}

// NewEncryptedStringLexer creates a new encrypted lexer reading from data.
// The Enc XOR cipher is applied on getChars (munge) and putBack (mungeback).
func NewEncryptedStringLexer(data []byte, errOut io.Writer) *Lexer {
	gs := newGrowingString(len(data))
	enc := NewEnc()
	return &Lexer{
		src:                &encryptedStringSource{src: &stringSource{str: data}, enc: enc},
		tokval:             *gs,
		LexicalError:       false,
		EmitLexicalWarnings: false,
		linesRead:          0,
		startScan:        0,
		startToken:       0,
		endToken:         0,
		curCharIndex:     0,
		errOut:             errOut,
	}
}

// NewReaderLexer creates a new lexer reading from the given io.Reader.
// readahead is the number of bytes to pre-read from the reader.
func NewReaderLexer(r io.Reader, errOut io.Writer, readahead int) *Lexer {
	return &Lexer{
		src:                &readerSource{r: r, buf: make([]byte, readahead), readahead: readahead},
		LexicalError:       false,
		EmitLexicalWarnings: false,
		linesRead:          0,
		startScan:        0,
		startToken:       0,
		endToken:         0,
		curCharIndex:     0,
		errOut:             errOut,
	}
}

// NewEncryptedReaderLexer creates a new encrypted lexer reading from r.
func NewEncryptedReaderLexer(r io.Reader, errOut io.Writer, readahead int) *Lexer {
	enc := NewEnc()
	return &Lexer{
		src:                &encryptedReaderSource{r: &readerSource{r: r, buf: make([]byte, readahead), readahead: readahead}, enc: enc},
		LexicalError:       false,
		EmitLexicalWarnings: false,
		linesRead:          0,
		startScan:        0,
		startToken:       0,
		endToken:         0,
		curCharIndex:     0,
		errOut:             errOut,
	}
}

// NextToken returns the next token from the input, or TokEOS if no more input is available.
// The returned val is a slice into the lexer's internal buffer and is valid only until
// the next call to NextToken (at which point it is reset).  The atom type is also returned.
func (l *Lexer) NextToken() (tok Token, val []byte, atype Atom) {
	l.tokval.reset()
	l.setBeginningOfScan()

	if !l.getToplev() {
		return TokEOS, nil, AtomBogus
	}

	// append explicitly 0-terminate byte, matching C++ behaviour
	l.tokval.append([]byte{0}, 1)

	tv := l.tokval.refBytes()
	// len excludes the trailing 0 (just as C++ does: len = tokval.length() - 1)
	ln := l.tokval.length() - 1
	if ln > 0 {
		val = tv[:ln]
	} else {
		val = nil
	}
	atype = l.atype
	l.setEndOfToken()
	return l.token, val, atype
}

// TokenLinenum returns the line number where the current token starts.
func (l *Lexer) TokenLinenum() int64 {
	return l.startToken
}

// TokenLinenumStartScan returns the line number when next_token entered.
func (l *Lexer) TokenLinenumStartScan() int64 {
	return l.startScan
}

// InputDescription returns the input description string set via SetInputDescription.
func (l *Lexer) InputDescription() string {
	return l.inpDesc
}

// SetInputDescription sets the input description string.
func (l *Lexer) SetInputDescription(s string) {
	l.inpDesc = s
}

// GetLexicalError returns true if the input contained a lexical error.
func (l *Lexer) GetLexicalError() bool {
	return l.LexicalError
}

// GetErrStrm returns the error output writer.
func (l *Lexer) GetErrStrm() io.Writer {
	return l.errOut
}

// --- low-level input methods (implemented by source subtypes) ---

func (l *Lexer) getChars(want int) []byte {
	return l.src.getChars(want)
}

func (l *Lexer) putBack(b []byte) {
	l.src.putBack(b)
}

// --- public helpers used by the state machine ---

func (l *Lexer) isSpecial(c byte) bool {
	switch c {
	case '(', ')', '{', '}', '<', '>', '\'', '`', '^', '[', ']':
		return true
	}
	return false
}

func (l *Lexer) isTerminator(c byte) bool {
	switch c {
	case ' ', '\t', '\n', '\r', '\f', '\v':
		return true
	}
	return l.isSpecial(c)
}

func (l *Lexer) isDigit(c byte, base int) bool {
	if base <= 10 {
		return c >= '0' && c <= byte('0'+base-1)
	}
	if base == 16 {
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')
	}
	return false
}

// appendTook appends characters to the current token.
func (l *Lexer) appendTook(s []byte, len int) {
	l.tokval.append(s, len)
}

// getSpecial maps a special character to a token.
func (l *Lexer) getSpecial(c byte) int {
	switch c {
	case '(':
		l.token = TokLParen
	case ')':
		l.token = TokRParen
	case '{':
		l.token = TokLBrace
	case '}':
		l.token = TokRBrace
	case '<':
		l.token = TokLSBrack
	case '>':
		l.token = TokRSBrack
	case '\'':
		l.token = TokRTick
	case '`':
		l.token = TokLTick
	case '^':
		l.token = TokUpArrow
	case '[':
		l.token = TokLSBrack
	case ']':
		l.token = TokRSBrack
	default:
		panic("bogus special")
	}
	return 1
}

// is_terminator checks if a character is a terminator.
func (l *Lexer) is_terminator(c byte) bool {
	if l.isTerminator(c) {
		return true
	}
	return false
}

// term_state processes the current character and sets the token type.
func (l *Lexer) term_state(moreInput bool, c byte, tok Token, typ Atom) bool {
	var terminator bool
	if moreInput {
		terminator = l.isTerminator(c)
	}
	if !moreInput || terminator {
		if terminator {
			l.putBack([]byte{c})
		}
		l.token = tok
		l.atype = typ
		return true
	}
	return false
}

// get_safe_char reads a single character from the input.
func (l *Lexer) get_safe_char() bool {
	buf := l.src.getChars(1)
	if buf == nil || len(buf) == 0 {
		if l.src.noMoreAvailable() {
			return false
		}
	}
	l.curChar = buf[0]
	return true
}

// eat_comment consumes a comment until end of line.
func (l *Lexer) eat_comment() {
	for {
		buf := l.src.getChars(16) // COMMENT_CHUNK
		if buf == nil {
			return
		}
		for i := 0; i < len(buf); i++ {
			if buf[i] == '\n' {
				// put back the newline and everything after
				l.src.putBack(buf[i:])
				return
			}
		}
		// no newline in buffer, continue reading
	}
}

// get_literal reads a parenthesised literal string like "|hello|"
func (l *Lexer) get_literal() bool {
	// Assume we've already seen and eaten the leading "|"
	holding := make([]byte, 128) // CHUNK
	state := 0 // 0=NORMAL, 1=ESCAPE
	var i, h int

	for {
		buf := l.src.getChars(128) // CHUNK
		if buf == nil {
			// error, eof in literal
			l.LexicalError = true
			l.lexError("unterminated literal string atom")
			return false
		}
		for i = 0; i < len(buf); i++ {
			if state != 1 && buf[i] == '\\' {
				state = 1
				continue
			}
			if state == 1 {
				// escaped character - just consume it
				state = 0
				continue
			}
			if buf[i] == '|' {
				// end of literal
				state = 2
				break
			}
			holding[h] = buf[i]
			h++
		}
		if state == 2 {
			// found terminator, don't put it back
			l.appendTook(holding, h)
			// the '|' was at buf[i], don't put back anything after it
			break
		}
		// put back remaining buffer starting from i
		if i < len(buf) {
			l.src.putBack(buf[i:])
		}
		// should not reach here normally, but if we do, error
		l.LexicalError = true
		return false
	}

	l.atype = AtomString
	l.token = TokAtom
	return true
}

// get_uinteger reads an unsigned integer with optional base modifier.
func (l *Lexer) get_uinteger() bool {
	// States: START, BASE, NO_BASE, DONE
	var state int = 0 // 0=START, 1=BASE, 2=NO_BASE
	l.base = 10
	l.curbase = 10
	empty := true

	for {
		buf := l.src.getChars(8) // NUMBER_CHUNK
		if buf == nil {
			if empty {
				return false // no digits at all
			}
			// success - fall through
			break
		}
		empty = false

		for i := 0; i < len(buf); i++ {
			c := buf[i]
			switch state {
			case 0: // START
				if c == '0' {
					state = 1 // BASE
				} else if c >= '1' && c <= '9' {
					// valid digit for base 10
				} else {
					// not a valid integer start
					// put back and return failure
					l.src.putBack(buf[i:])
					return false
				}
			case 1: // BASE
				switch c {
				case 'b', 'B':
					l.base = 2
				case 't', 'T':
					l.base = 10
				case 'x', 'X':
					l.base = 16
				case 'o', 'O':
					l.base = 8
				default:
					// not a base modifier, go to NO_BASE
					state = 2
					// reprocess this character
					i-- // will be incremented by for loop
					continue
				}
				// after base modifier, go to NO_BASE state
				state = 2
			case 2: // NO_BASE
				if l.isDigit(c, l.base) {
					// valid digit, continue
				} else {
					// terminator or end
					// put back this char and others
					l.src.putBack(buf[i:])
					goto got_uint
				}
			}
		}
		// continue loop to read more chunks
	}
got_uint:
	// Mark as integer atom type
	l.atype = AtomInteger
	l.token = TokAtom
	return true
}

// get_symatom reads a symbolic atom (identifier-like token).
func (l *Lexer) get_symatom() bool {
	// States: START, ESCAPE
	var state int = 0 // 0=START, 1=ESCAPE
	var ostate int = 0
	holding := make([]byte, 128) // CHUNK
	var i, h int

	for {
		buf := l.src.getChars(128) // CHUNK
		if buf == nil {
			// end of input
			break
		}
		for i = 0; i < len(buf); i++ {
			if state != 1 && buf[i] == '\\' {
				ostate = state
				state = 1
				continue
			}
			if state == 1 {
				// escaped character
				state = ostate
				holding[h] = buf[i]
				h++
				continue
			}
			// START state: check for terminator
			if state == 0 {
				// Check if character is a terminator
				if i < len(buf) && (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n' || buf[i] == ';' || buf[i] == '|' || l.isSpecial(buf[i])) {
					// put back the terminator and break
					l.src.putBack(buf[i:])
					break
				}
				holding[h] = buf[i]
				h++
			}
		}
		if i < len(buf) {
			// put back remaining buffer
			l.src.putBack(buf[i:])
		}
		// accumulate token
		l.appendTook(holding, h)
		// check if we have a non-empty token
		if l.tokval.length() > 0 {
			l.atype = AtomString
			l.token = TokAtom
			return true
		}
		// otherwise continue reading
	}
	// no token recognized
	l.atype = AtomBogus
	l.token = TokBogus
	return false
}

// get_toplev is the main lexer state machine. It processes the input
// and returns true if a token was recognised, false otherwise.
// This is the Go port of C++ VcLexer::get_toplev().
func (l *Lexer) getToplev() bool {
	c := byte(0)
	state := 0 // 0=EAT_WHITE, 1=START, 2=S1, 3=S2, 4=S3, 5=S4, 6=S5, 7=S6, 8=S7

	if !l.get_safe_char() {
		return false
	}

	for {
		switch state {
		case 0: // EAT_WHITE
			if l.isTerminator(l.curChar) {
				if !l.get_safe_char() {
					return false
				}
			} else {
				state = 1 // START
			}
		case 1: // START
			l.appendTook([]byte{l.curChar}, 1)
			if l.curChar == '\\' {
				state = 2 // ESCAPE
				l.src.putBack([]byte{l.curChar})
				// get_symatom would go here
				// For now, just return true
				l.atype = AtomString
				l.token = TokAtom
				return true
			}
			if l.isSpecial(l.curChar) {
				l.getSpecial(l.curChar)
				return true
			}
			if l.curChar == ';' {
				l.eat_comment()
				if !l.get_safe_char() {
					return false
				}
				state = 0 // stay in EAT_WHITE
				continue
			}
			if l.curChar == '|' {
				// get_literal()
				l.atype = AtomString
				l.token = TokAtom
				// read past the literal
				if !l.get_safe_char() {
					return false
				}
				return true
			}
			if l.curChar == '+' || l.curChar == '-' {
				l.appendTook([]byte{l.curChar}, 1)
				state = 3 // S2
				if !l.get_safe_char() {
					return false
				}
				continue
			}
			if l.curChar == '.' {
				l.appendTook([]byte{l.curChar}, 1)
				state = 4 // S3
				if !l.get_safe_char() {
					return false
				}
				continue
			}
		// put back character and try integer
			l.src.putBack([]byte{l.curChar})
			// get_uinteger()
			if l.get_uinteger() {
				l.atype = AtomInteger
				l.token = TokAtom
				return true
			}
			// get_symatom()
			l.atype = AtomString
			l.token = TokAtom
			return true

		case 2: // ESCAPE (simplified)
			// In real implementation, this would handle backslash sequences
			// and call get_symatom
			l.atype = AtomString
			l.token = TokAtom
			return true

		case 3: // S2 - after sign
			// term_state check
			if l.term_state(true, l.curChar, AtomString, AtomString) {
				return true
			}
			if l.curChar == '.' {
				l.appendTook([]byte{l.curChar}, 1)
				state = 4 // S3
			} else {
				l.src.putBack([]byte{l.curChar})
				if l.get_uinteger() {
					l.atype = AtomInteger
					l.token = TokAtom
					return true
				}
				l.atype = AtomString
				l.token = TokAtom
				return true
			}
			if !l.get_safe_char() {
				return false
			}
			continue

		case 4: // S3 - after dot
			if l.get_uinteger() {
				// could be float
				l.atype = AtomFloat
				l.token = TokAtom
				return true
			}
			l.lexWarning("hosed up floating pointer number?")
			l.atype = AtomString
			l.token = TokAtom
			return true

		case 5: // S4 - integer after potential sign
			if l.term_state(true, l.curChar, AtomInteger, AtomInteger) {
				return true
			}
			if l.curChar == '.' {
				l.appendTook([]byte{l.curChar}, 1)
				state = 7 // S7
				if !l.get_safe_char() {
					return false
				}
				continue
			}
			if l.curChar == 'e' || l.curChar == 'E' {
				l.appendTook([]byte{l.curChar}, 1)
				state = 6 // S6
				if !l.get_safe_char() {
					return false
				}
				continue
			}
			l.appendTook([]byte{l.curChar}, 1)
			l.atype = AtomInteger
			l.token = TokAtom
			return true

		case 6: // S6 - after E/e in float
			// get_sinteger()
			l.atype = AtomFloat
			l.token = TokAtom
			return true

		case 7: // S7 - after dot in float
			if l.term_state(true, l.curChar, AtomFloat, AtomFloat) {
				return true
			}
			if l.curChar == 'e' || l.curChar == 'E' {
				l.appendTook([]byte{l.curChar}, 1)
				state = 6 // S6
				if !l.get_safe_char() {
					return false
				}
				continue
			}
			l.src.putBack([]byte{l.curChar})
			if l.get_uinteger() {
				state = 5 // S5
				continue
			}
			l.lexWarning("hosed up floating pointer number?")
			l.atype = AtomFloat
			l.token = TokAtom
			return true

		default:
			// bogus state
			return false
		}
	}
}

// lex_error records a lexical error.
func (l *Lexer) lexError(s string) {
	l.LexicalError = true
	l.errOut.Write([]byte("lexical error: " + s + "\n"))
}

// lex_warning records a lexical warning if enabled.
func (l *Lexer) lexWarning(s string) {
	if l.EmitLexicalWarnings {
		l.errOut.Write([]byte("lexical warning (near line " + string(rune(l.linesRead+1)) + "): " + s + " (token probably recognized as symbolic atom)\n"))
	}
}

// setBeginningOfToken sets the starting line/char for token tracking.
func (l *Lexer) setBeginningOfToken() {
	l.startToken = l.linesRead + 1
}

// setEndOfToken sets the ending line/char for token tracking.
func (l *Lexer) setEndOfToken() {
	l.endToken = l.linesRead + 1
}

// setBeginningOfScan sets the starting line for scan tracking.
func (l *Lexer) setBeginningOfScan() {
	l.startScan = l.linesRead + 1
}

// --- source implementations ---

type stringSource struct {
	str []byte // the full input string; never modified
}

func (s *stringSource) getChars(want int) []byte {
	if len(s.str) == 0 {
		return nil
	}
	if want > len(s.str) {
		want = len(s.str)
	}
	out := s.str[:want]
	return out
}

func (s *stringSource) putBack(b []byte) {
	// for a static string source, pushback is a no-op since we can't modify the original
	_ = b
}

func (s *stringSource) noMoreAvailable() bool {
	return false // static string never exhausts until consumer stops
}

type readerSource struct {
	r       io.Reader
	buf     []byte // fixed readahead buffer
	readahead int
	pos     int // next byte to return from buf (0..readahead)
	len     int // valid bytes in buf (0..readahead)
}

func (r *readerSource) getChars(want int) []byte {
	// fast path: already have data in buffer
	if r.pos < r.len {
		available := r.len - r.pos
		if want > available {
			want = available
		}
		out := r.buf[r.pos : r.pos+want]
		r.pos += want
		return out
	}
	// need to refill
	r.pos = 0
	n, err := r.r.Read(r.buf[:r.readahead])
	if err != nil || n == 0 {
		r.len = 0
		return nil
	}
	r.len = n
	// return from buffer
	if r.pos < r.len {
		out := r.buf[r.pos : r.pos+want]
		r.pos += want
		return out
	}
	return nil
}

func (r *readerSource) putBack(b []byte) {
	n := len(b)
	// prepend pushed-back bytes into the buffer
	if n > r.readahead {
		n = r.readahead
	}
	if n <= r.len && r.pos >= n {
		// shift data right to make room
		copy(r.buf[r.pos-n:r.pos], r.buf[r.pos:r.pos+n])
		r.pos -= n
	} else {
		// beyond buffer: reset to beginning, will refill next getChars
		r.pos = 0
		r.len = 0
	}
}

func (r *readerSource) noMoreAvailable() bool {
	// simplified: the lexer decides based on getChars returning nil
	return false
}

type encryptedStringSource struct {
	src  *stringSource
	enc  *Enc
}

func (e *encryptedStringSource) getChars(want int) []byte {
	buf := e.src.getChars(want)
	if buf == nil {
		return nil
	}
	// munge (encrypt) in place, matching C++ encrypt() call in get_chars
	e.enc.Munge(buf)
	return buf
}

func (e *encryptedStringSource) putBack(b []byte) {
	// mungeback to re-encrypt before pushing back
	e.enc.Mungeback(b)
	e.src.putBack(b)
}

func (e *encryptedStringSource) noMoreAvailable() bool {
	return e.src.noMoreAvailable()
}

type encryptedReaderSource struct {
	r    *readerSource
	enc  *Enc
}

func (e *encryptedReaderSource) getChars(want int) []byte {
	buf := e.r.getChars(want)
	if buf == nil {
		return nil
	}
	// munge (encrypt) in place before returning to lexer
	e.enc.Munge(buf)
	return buf
}

func (e *encryptedReaderSource) putBack(b []byte) {
	// mungeback to re-decrypt the pushed-back region
	e.enc.Mungeback(b)
	// now push the plaintext back into the reader source
	e.r.putBack(b)
}

func (e *encryptedReaderSource) noMoreAvailable() bool {
	return e.r.noMoreAvailable()
}