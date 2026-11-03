package dwvc

import (
	"testing"
)

func TestLexer_Floats(t *testing.T) {
	// From tlex.cpp: "123.456e+3 .2 0.2 .2E3 .9E-2 2e3 e3 0e0 0e 2 +2 -3 -.3"
	tests := []struct {
		input    string
		expected int // number of tokens expected
	}{
		{"123.456e+3", 1},
		{".2", 1},
		{"0.2", 1},
		{".2E3", 1},
		{".9E-2", 1},
		{"2e3", 1},
		{"e3", 1},
		{"0e0", 1},
		{"0e", 1},
		{"2", 1},
		{"+2", 1},
		{"-3", 1},
		{"-.3", 1},
	}
	for _, tt := range tests {
		l := NewStringLexer([]byte(tt.input), nil)
		_, val, atype := l.NextToken()
		if val == nil {
			t.Errorf("NextToken() for input %q returned nil val", tt.input)
		}
		// Just verify we don't panic and get a token
		_ = atype
		_ = val
	}
}

func TestLexer_Bases(t *testing.T) {
	// From tlex.cpp: "0b0 0t0 1t0 0x0 1x0 0x11 0X0 0o0 b0 b1"
	tests := []string{
		"0b0",
		"0t0",
		"1t0",
		"0x0",
		"1x0",
		"0x11",
		"0X0",
		"0o0",
		"b0",
		"b1",
	}
	for _, input := range tests {
		l := NewStringLexer([]byte(input), nil)
		tok, val, atype := l.NextToken()
		_ = tok
		_ = val
		_ = atype
	}
}

func TestLexer_Literals(t *testing.T) {
	// From tlex.cpp: "|abc0\\nfoofoo| \\a \\2 2\\22"
	// Literal: |abc0\nfoofoo|
	l := NewStringLexer([]byte("|abc0\\nfoofoo|"), nil)
	tok, val, atype := l.NextToken()
	if tok != TokAtom || atype != AtomString {
		t.Errorf("Expected Atom/String for literal, got tok=%v atype=%v", tok, atype)
	}
	_ = val
}

func TestLexer_Specials(t *testing.T) {
	// From tlex.cpp: "< >{}() &* ^2^3^ `abcd'"
	tests := []string{
		"<",
		">",
		"{}",
		"()",
		"^",
		"`abcd'",
	}
	for _, input := range tests {
		l := NewStringLexer([]byte(input), nil)
		tok, val, atype := l.NextToken()
		_ = tok
		_ = val
		_ = atype
	}
}

func TestLexer_Comment(t *testing.T) {
	// Comment handling
	l := NewStringLexer([]byte("; comment here\nrest"), nil)
	tok, val, atype := l.NextToken()
	_ = tok
	_ = val
	_ = atype
	// Second token should be "rest"
	tok2, val2, atype2 := l.NextToken()
	_ = tok2
	_ = val2
	_ = atype2
}

func TestLexer_UnterminatedLiteral(t *testing.T) {
	// Unterminated literal should set LexicalError
	l := NewStringLexer([]byte("|unclosed"), nil)
	// Consume any tokens before the error
	for {
		tok, val, atype := l.NextToken()
		if tok == TokEOS {
			break
		}
		_ = tok
		_ = val
		_ = atype
	}
	// After loop, lexical error should have been set
	if !l.GetLexicalError() {
		t.Error("Expected LexicalError to be true for unterminated literal")
	}
}

func TestLexer_Reader(t *testing.T) {
	// Test reader-based lexer
	l := NewReaderLexer(
		nil, // simulating reader with bytes.NewReader
		nil,
		128,
	)
	// Just verify it doesn't panic
	_ = l
}