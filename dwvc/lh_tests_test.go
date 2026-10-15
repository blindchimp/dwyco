package dwvc

import (
	"bufio"
	"bytes"
	"errors"
	"io"
	"math/rand"
	"net"
	"os"
	"path/filepath"
	"regexp"
	"strings"
	"testing"
)

// lhSubst is the direct translation of subst in bld/vc/test/subst.lh.
func lhSubst(s, x, y string) string {
	if x == "" {
		return s
	}
	sx := len(x)
	i := strings.Index(s, x)
	var res strings.Builder
	for i >= 0 {
		f := s[:i]
		res.WriteString(f)
		res.WriteString(y)
		s = s[i+sx:]
		i = strings.Index(s, x)
	}
	if res.Len() == 0 {
		return s
	}
	res.WriteString(s)
	return res.String()
}

// TestLHSubst translates bld/vc/test/subst.lh.
func TestLHSubst(t *testing.T) {
	tests := []struct {
		s, x, y string
		want    string
	}{
		{"foo", "foo", "foo", "foo"},
		{"foo", "foo", "barbar", "barbar"},
		{"foofoo", "foo", "barbar", "barbarbarbar"},
		{"creep", "foo", "barbar", "creep"},
		{"rrfoofooxfoow", "foo", "f", "rrffxfw"},
		{"unsigned int", " ", "_", "unsigned_int"},
	}

	for _, tt := range tests {
		got := lhSubst(tt.s, tt.x, tt.y)
		if got != tt.want {
			t.Errorf("lhSubst(%q, %q, %q) = %q, want %q", tt.s, tt.x, tt.y, got, tt.want)
		}
		if stdGot := strings.ReplaceAll(tt.s, tt.x, tt.y); stdGot != tt.want {
			t.Errorf("strings.ReplaceAll(%q, %q, %q) = %q, want %q", tt.s, tt.x, tt.y, stdGot, tt.want)
		}
	}
}

// TestLHTstSer translates bld/vc/test/tstser.lh.
func TestLHTstSer(t *testing.T) {
	tmpFile := filepath.Join(t.TempDir(), "tmp.out")

	// Write phase: openfile(tmp.out wb) + putfile + closefile
	outFile, err := os.Create(tmpFile)
	if err != nil {
		t.Fatalf("create tmp.out: %v", err)
	}

	var a int64 = 0x7fffffffffffffff
	itemsToWrite := []Vc{
		VcInt(a),
		VcInt(-a),
		VcInt(-a - 1), // sub(<a> 1) == math.MinInt64 (-9223372036854775808)
		VcVector(),
		VcList(VcInt(1), VcInt(2), VcInt(3)),
		VcMap(VcInt(1), VcInt(2), VcInt(3), VcInt(4)),
	}

	for idx, item := range itemsToWrite {
		if _, err := item.EncodeTo(outFile); err != nil {
			outFile.Close()
			t.Fatalf("putfile item %d: %v", idx, err)
		}
	}
	if err := outFile.Close(); err != nil {
		t.Fatalf("closefile after write: %v", err)
	}

	// Read phase: openfile(tmp.out rb) + 6x getfile(<out> a)
	inFile, err := os.Open(tmpFile)
	if err != nil {
		t.Fatalf("open tmp.out for read: %v", err)
	}
	defer inFile.Close()

	r := bufio.NewReader(inFile)
	var boundA Vc

	// 1: 0x7fffffffffffffff
	v, n, err := DecodeFrom(r)
	if err != nil {
		t.Fatalf("getfile 1: %v", err)
	}
	boundA = v
	if n != 23 || boundA.String() != "9223372036854775807" {
		t.Errorf("getfile 1: n=%d val=%s, want n=23 val=9223372036854775807", n, boundA)
	}

	// 2: -0x7fffffffffffffff
	v, n, err = DecodeFrom(r)
	if err != nil {
		t.Fatalf("getfile 2: %v", err)
	}
	boundA = v
	if n != 24 || boundA.String() != "-9223372036854775807" {
		t.Errorf("getfile 2: n=%d val=%s, want n=24 val=-9223372036854775807", n, boundA)
	}

	// 3: sub(-0x7fffffffffffffff, 1) == math.MinInt64
	// C++ decode_long2 rejects LLONG_MIN as overflow, so getfile returns nil
	// and leaves 'a' bound to its previous value while consuming the 24 wire bytes.
	_, _, err = DecodeFrom(r)
	if !errors.Is(err, ErrParse) {
		t.Errorf("getfile 3 (MinInt64): got err=%v, want ErrParse", err)
	}
	if boundA.String() != "-9223372036854775807" {
		t.Errorf("getfile 3: boundA=%s, want unchanged -9223372036854775807", boundA)
	}

	// 4: vector()
	v, n, err = DecodeFrom(r)
	if err != nil {
		t.Fatalf("getfile 4: %v", err)
	}
	boundA = v
	if n != 5 || boundA.String() != "vector()" {
		t.Errorf("getfile 4: n=%d val=%s, want n=5 val=vector()", n, boundA)
	}

	// 5: list(1 2 3)
	v, n, err = DecodeFrom(r)
	if err != nil {
		t.Fatalf("getfile 5: %v", err)
	}
	boundA = v
	if n != 20 || boundA.String() != "list(1 2 3)" {
		t.Errorf("getfile 5: n=%d val=%s, want n=20 val=list(1 2 3)", n, boundA)
	}

	// 6: map(vector(1 2) vector(3 4))
	v, n, err = DecodeFrom(r)
	if err != nil {
		t.Fatalf("getfile 6: %v", err)
	}
	boundA = v
	if n != 25 || boundA.String() != "map(vector(1 2) vector(3 4))" {
		t.Errorf("getfile 6: n=%d val=%s, want n=25 val=map(vector(1 2) vector(3 4))", n, boundA)
	}
}

// lhRegexMatch emulates LH's match(re, str): matches anchored at the beginning
// of str and returns the matched substring and true, or ("", false) for nil.
func lhRegexMatch(re *regexp.Regexp, s string) (string, bool) {
	loc := re.FindStringIndex(s)
	if loc == nil || loc[0] != 0 {
		return "", false
	}
	return s[loc[0]:loc[1]], true
}

// lhRegexSplit emulates LH's split(re, str, subs...): matches anchored at the
// beginning of str and returns the full match, capture group substrings, and ok.
func lhRegexSplit(re *regexp.Regexp, s string) (string, []string, bool) {
	idxs := re.FindStringSubmatchIndex(s)
	if idxs == nil || idxs[0] != 0 {
		return "", nil, false
	}
	full := s[idxs[0]:idxs[1]]
	var subs []string
	for i := 2; i < len(idxs); i += 2 {
		if idxs[i] >= 0 && idxs[i+1] >= 0 {
			subs = append(subs, s[idxs[i]:idxs[i+1]])
		} else {
			subs = append(subs, "")
		}
	}
	return full, subs, true
}

// TestLHTstRe translates bld/vc/test/tstre.lh.
func TestLHTstRe(t *testing.T) {
	// LH regex: |foo.*\\([0-9][0-9]*\\)baz| (POSIX BRE capture group \(...\))
	re := regexp.MustCompile(`^foo.*([0-9][0-9]*)baz`)

	matchCases := []struct {
		input   string
		wantStr string
		wantOK  bool
	}{
		{"mumble", "", false},
		{"foo and baz", "", false},
		{"foo 9 baz", "", false},
		{"foo 9baz", "foo 9baz", true},
	}

	for _, tc := range matchCases {
		got, ok := lhRegexMatch(re, tc.input)
		if ok != tc.wantOK || got != tc.wantStr {
			t.Errorf("match(%q) = (%q, %v), want (%q, %v)", tc.input, got, ok, tc.wantStr, tc.wantOK)
		}
	}

	// printl(split(<a> |foo9baz| subs))
	// printl(<subs>)
	full, subs, ok := lhRegexSplit(re, "foo9baz")
	if !ok {
		t.Fatalf("split(\"foo9baz\") returned no match, want match")
	}
	if full != "foo9baz" {
		t.Errorf("split full match = %q, want %q", full, "foo9baz")
	}
	if len(subs) != 1 || subs[0] != "9" {
		t.Errorf("split subs = %v, want [\"9\"]", subs)
	}
}

// TestLHNonBlockingSocketXfer translates bld/vc/test/testnb.lh and testnbr.lh.
// In Go, net.Conn uses the runtime's non-blocking network poller under the hood,
// replacing LH's manual E:*would* exchandle retry loop.
func TestLHNonBlockingSocketXfer(t *testing.T) {
	const (
		vectorSize = 1000
		iterations = 50
	)

	ln, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatalf("net.Listen: %v", err)
	}
	defer ln.Close()

	// Build the test vector as in testnb.lh:
	// lbind(a vector2(vectorSize))
	// srand(1)
	// for(i 0 vectorSize-1 putidx(<a> <i> rand()))
	rng := rand.New(rand.NewSource(1))
	elems := make([]Vc, vectorSize)
	for i := 0; i < vectorSize; i++ {
		elems[i] = VcInt(rng.Int63())
	}
	sendVec := VcVector(elems...)
	wantStr := sendVec.String()

	errCh := make(chan error, 1)

	// Sender goroutine (testnb.lh)
	go func() {
		conn, err := net.Dial("tcp", ln.Addr().String())
		if err != nil {
			errCh <- err
			return
		}
		defer conn.Close()

		bw := bufio.NewWriter(conn)
		for n := 0; n < iterations; n++ {
			if _, err := sendVec.EncodeTo(bw); err != nil {
				errCh <- err
				return
			}
		}
		errCh <- bw.Flush()
	}()

	// Receiver (testnbr.lh)
	srvConn, err := ln.Accept()
	if err != nil {
		t.Fatalf("ln.Accept: %v", err)
	}
	defer srvConn.Close()

	br := bufio.NewReader(srvConn)
	var outBuf bytes.Buffer

	for n := 0; n < iterations; n++ {
		recvd, _, err := DecodeFrom(br)
		if err != nil {
			t.Fatalf("socket-recv iteration %d: %v", n, err)
		}
		if recvd.Type() != VCVector || recvd.Len() != vectorSize {
			t.Fatalf("iteration %d: got type=%d len=%d, want VCVector len=%d", n, recvd.Type(), recvd.Len(), vectorSize)
		}
		// fprint(<foo> <a>) in testnbr.lh; verify first and last iterations match wantStr
		if n == 0 || n == iterations-1 {
			outBuf.Reset()
			if _, err := io.WriteString(&outBuf, recvd.String()); err != nil {
				t.Fatalf("fprint iteration %d: %v", n, err)
			}
			if outBuf.String() != wantStr {
				t.Fatalf("iteration %d: received vector mismatch", n)
			}
		}
	}

	if err := <-errCh; err != nil {
		t.Fatalf("sender error: %v", err)
	}
}
