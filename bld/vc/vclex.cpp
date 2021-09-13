
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#include <string.h>
#include <ctype.h>
#include "vclex.h"
#include "vcio.h"
#ifndef NO_VCEVAL

//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/vclex.cpp 1.49 1998/08/06 04:45:11 dwight Exp $";

// ad hoc lexer for LH

#define CHUNK 128
#define NUMBER_CHUNK 8
#define COMMENT_CHUNK 16

VcLexer::VcLexer(VcIO o)
	: err_strm(o)
{
	lines_read = 0;
	start_scan = 0;
	start_token = 0;
	end_token = 0;
	lexical_error = 0;
    emit_lexical_warnings = 0;
    cur_char_index = 0;
}

//VcLexer::Token
//VcLexer::next_token()
//{
//	tokval.reset();
//	set_beginning_of_scan();
//	if(get_toplev())
//	{
//		append_tok("\0", 1); // explicitly 0 terminate
//		set_end_of_token();
//		return token;
//	}
//	return EOS;
//}

VcLexer::Token
VcLexer::next_token(const char *& tv, long& len, Atom& tp_out)
{
	tokval.reset();
	set_beginning_of_scan();
	if(get_toplev())
	{
		append_tok("\0", 1); // explicitly 0 terminate, just in case
		tv = tokval.ref_str();
		len = tokval.length() - 1; // don't include 0, in case it was binary
		tp_out = type;
		set_end_of_token();
		return token;
	}
	return EOS;
}

long
VcLexer::token_len()
{
	return tokval.length();
}


void
VcLexer::lex_error(const char *s)
{
	lexical_error = 1;
	err_strm << "lexical error: " << s << "\n";
}

void
VcLexer::lex_warning(const char *s)
{
    if(emit_lexical_warnings)
	{
		err_strm << "lexical warning (near line " << lines_read + 1 << "): " << s;
		err_strm << " (token probably recognized as symbolic atom)\n";
	}
}

// default source tracking stuff is based
// on line numbers only. 

void
VcLexer::set_beginning_of_token()
{
	start_token = lines_read + 1;
    char_start_token = cur_char_index;
}

void
VcLexer::set_end_of_token()
{
	end_token = lines_read + 1;
    char_end_token = cur_char_index;
}

void
VcLexer::set_beginning_of_scan()
{
	start_scan = lines_read + 1;
}

long
VcLexer::token_linenum()
{
	return start_token;
}
long
VcLexer::token_linenum_start_scan()
{
	return start_scan;
}

DwString
VcLexer::input_description()
{
	return inp_desc;
}

void
VcLexer::set_input_description(const char *s)
{
    inp_desc = s;
}



// This is here because these functions implement the
// input scanning needed by the lexer, which must
// be implemented explicitly in each derived class.
// These are here simply to provide a hanging place
// for the common "source coordinate" tracking
// that most derived classes share. the derived classes
// must call this function explicitly to enable
// the proper tracking of line/char num.
//
void
VcLexer::forward_track_source(const char *buf_out, long len)
{
	for(long i = 0; i < len; ++i)
	{
		if(buf_out[i] == '\n')
			++lines_read;
	}
}

void
VcLexer::backward_track_source(const char *buf, long len)
{
	for(long i = len - 1; i >= 0; --i)
	{
		if(buf[i] == '\n')
			--lines_read;
	}
}


int
VcLexer::is_special(char c)
{
	switch(c)
	{
	case '(':
	case ')':
	case '{':
	case '}':
	case '<':
	case '>':
	case '\'':
	case '`':
	case '^':
	case '[':
	case ']':
		return 1;
	}
	return 0;
}

int
VcLexer::get_special(char c)
{
	switch(c)
	{
	case '(':
		token = LPAREN;
		break;
	case ')':
		token = RPAREN;
		break;
	case '{':
		token = LBRACE;
		break;
	case '}':
		token = RBRACE;
		break;
	case '<':
		token = LBRACKET;
		break;
	case '>':
		token = RBRACKET;
		break;
	case '\'':
		token = RTICK;
		break;
	case '`':
		token = LTICK;
		break;
	case '^':
		token = UPARROW;
		break;
	case '[':
		token = LSBRACK;
		break;
	case ']':
		token = RSBRACK;
		break;
	default:
		oopanic("bogus special");
		/*NOTREACHED*/
	}
	return 1;
}

int
VcLexer::is_terminator(char c)
{
	if(is_special(c) || isspace(c)
		 || c == '|' || c == ';')
		return 1;
	return 0;
}

int
VcLexer::is_digit(char c, int base)
{
	if(base <= 10)
		return c >= '0' && c <= ('0' + base - 1);
	if(base == 16)
		return isxdigit(c);
	oopanic("bogus base");
	/*NOTREACHED*/
	return 0;
}

void
VcLexer::append_tok(const char *s, int l)
{
	tokval.append(s, l);
}

void
VcLexer::mark_token()
{
	tokval.mark();
}

void
VcLexer::putback_to_mark()
{
	char *b;
	long l;
	tokval.pop_to_mark(b, l);
	put_back(b, l);
}

void
VcLexer::commit_to_mark()
{
	tokval.toss_mark();
}


int
VcLexer::get_literal()
{
	// assume we've already seen and eaten
	// the leading "|"

	char holding_buf[CHUNK];
	char *buf;

	int h;
	int i;
	enum {DONE, NORMAL, ESCAPE} state;
	state = NORMAL;
	while(state != DONE)
	{
		long got = get_chars(buf, CHUNK);
		if(got == 0 && no_more_available())
		{
			// error, eof in literal
			lex_error("unterminated literal string atom");
			goto got_literal;
		}

		for(i = 0, h = 0; i < got; ++i)
		{
			if(state != ESCAPE && buf[i] == '\\')
			{
				state = ESCAPE;
				continue;
			}
			if(state == ESCAPE)
			{
				char c;
				switch(buf[i])
				{
				case 'n':
					c = '\n';
					break;
				case 'r':
					c = '\r';
					break;
				case 'f':
					c = '\f';
					break;
				case 't':
					c = '\t';
					break;
				default:
					c = buf[i];
				}
				holding_buf[h++] = c;
				state = NORMAL;
				continue;
			}
			if(buf[i] == '|')
			{
				state = DONE;
				break;
			}
			holding_buf[h++] = buf[i];
		}
		append_tok(holding_buf, h);
		if(i < got)
		{
			if(buf[i] == '|')
				++i; // don't put back terminator
			put_back(&buf[i], got - i);
		}
	}
got_literal:
	type = STRING;
	token = ATOM;
	return 1;
}



// non-backtracking version
// this function appends chars for an unsigned int
// into the token. it returns 1 if there is a valid
// integer recognized, and 0 if some char foiled
// the recognition. integers cannot be 0 length.
// the number is assumed to be base 10 unless
// one of the base-modifiers is present, in
// which case this function sets the "base"
// member to the base indicated by the
// base modifier.
//
int
VcLexer::get_uinteger()
{
	enum {DONE, START, BASE, NO_BASE, ESCAPE} state, ostate;
	int i;
	char *buf;
	long got = 0;
	int empty = 1;

	state = ostate = START;
	curbase = 10;
	base = 10;
	while(state != DONE)
	{
		got = get_chars(buf, NUMBER_CHUNK);
		if(got == 0)
		{
			if(no_more_available())
			{
				if(empty)
					return 0;
				goto got_uint;
			}
		}
		else
			empty = 0;
		for(i = 0; i < got; ++i)
		{
			switch(state)
			{
			case START:
				if(buf[i] == '0')
				{
					state = BASE;
					break;
				}
				else if(is_digit(buf[i], base))
				{
					state = NO_BASE;
					break;
				}
				else
				{
					goto not_uint;
				}
				break;

			case NO_BASE:
			no_base:
				if(is_digit(buf[i], base))
				{
					break;
				}
				else if(is_terminator(buf[i]) || buf[i] == '.' ||
					buf[i] == 'e' || buf[i] == 'E')
				{
					state = DONE;
					goto eoloop;
				}
				else
				{
					lex_warning("digit is not legal for integer in given base");
					 // warn: digit/ char foiled int recognition
					goto not_uint;
				}


			case BASE:
				switch(buf[i])
				{
				case 'b':
				case 'B':
					base = 2;
					break;
				case 't':
				case 'T':
					base = 10;
					break;
				case 'x':
				case 'X':
					base = 16;
					break;
				case 'o':
				case 'O':
					base = 8;
					break;
				default:
					if(isalpha(buf[i]))
						lex_warning("unknown base after leading 0 (should be: [btxo])");
					// goto no base without consuming char
					state = NO_BASE;
					goto no_base;
				}
                state = NO_BASE;
				break;

			case ESCAPE:
				state = ostate; // effectively skip over escaped char
				break;
			default:
				oopanic("bogus state (uint)");
				/*NOTREACHED*/
			}
		} // for
	eoloop:
		append_tok(buf, i);
		if(i < got)
			put_back(&buf[i], got - i);
	}// while
got_uint:
	return 1;
not_uint:
	append_tok(buf, i);
	if(i < got)
		put_back(&buf[i], got - i);
	return 0;
}


int
VcLexer::get_sinteger()
{
	char c;

	if(!get_safe_char(c))
		return 0;

	if(c == '+' || c == '-')
		append_tok(&c, 1);
	else
		put_back(&c, 1);
	if(!get_uinteger())
		return 0;
	return 1;
}

//
// this function appends chars until a
// terminator onto the current token.
// this function returns 1
// except if there are no chars in the
// resulting token.
//
int
VcLexer::get_symatom()
{
	enum {DONE, ESCAPE, START} state, ostate;
	char *buf;
	char holding_buf[CHUNK];
	int i;
	int h;

	state = START;
	ostate = START;
	while(state != DONE)
	{
		long got = get_chars(buf, CHUNK);
		if(got == 0 && no_more_available())
			goto gotsymatom;
		for(i = 0, h = 0; i < got; ++i)
		{
			if(state != ESCAPE && buf[i] == '\\')
			{
				ostate = state;
				state = ESCAPE;
				continue;
			}
			switch(state)
			{
			case START:
				if(is_terminator(buf[i]))
				{
					state = DONE;
					goto eoloop;
				}
				holding_buf[h++] = buf[i];
				break;
			case ESCAPE:
				holding_buf[h++] = buf[i];
				state = ostate; // effectively skip over escaped char
				break;
			default:
				oopanic("bogus state (symatom)");
				/*NOTREACHED*/
			}
		} // for
	eoloop:
		append_tok(holding_buf, h);
		if(i < got)
			put_back(&buf[i], got - i);
	}// while
gotsymatom:
	if(token_len() == 0)
		return 0;
	token = ATOM;
	type = STRING;
	return 1;
}

int
VcLexer::get_safe_char(char &c)
{
	char *cp;
	long got = get_chars(cp, 1);
	if(got == 0 && no_more_available())
		return 0;
	c = *cp;
	return 1;
}

int
VcLexer::peek_char(char &c)
{
	if(!get_safe_char(c))
		return 0;
	put_back(&c, 1);
	return 1;
}

void
VcLexer::eat_comment()
{
	char *buf;
	int i;
	long got;

	while(1)
	{
		got = get_chars(buf, COMMENT_CHUNK);
		if(got == 0 && no_more_available())
			return;
		for(i = 0; i < got; ++i)
		{
			if(buf[i] == '\n')
				goto eo_comment;
		}
	}
eo_comment:
	// note: comment looks like
	// one whitespace char, so
	// this leaves the trailing
	// newline so then next reader
	// gets some whitespace.
	if(i < got)
		put_back(&buf[i], got - i);
}



int
VcLexer::get_toplev()
{
	char c;
	enum {DONE, START, EAT_WHITE, ESCAPE,
		S1, S2, S3, S4, S5, S6, S7,
		GOT_FLOAT, GOT_SYMATOM} state;
	int t;

	state = EAT_WHITE;
	if(!get_safe_char(c))
			return 0;
	while(state != DONE)
	{
		switch(state)
		{
		case ESCAPE:
			return get_symatom();

		case EAT_WHITE:
			if(isspace(c))
			{
				if(!get_safe_char(c))
					return 0;
			}
			else
			{
				state = S1;
			}
			break;

		case S1:
			set_beginning_of_token();
			if(c == '\\' && state != ESCAPE)
			{
				// note: backslash handled in get_symatom
				state = ESCAPE;
				put_back(&c, 1);
				return get_symatom();
			}
			if(is_special(c))
				return get_special(c);
			if(c == ';') // comment
			{
				eat_comment();
				state = EAT_WHITE;
				if(!get_safe_char(c))
					return 0;
				continue;
			}
			if(c == '|')
			{
				return get_literal();
			}

			if(c == '+' || c == '-')
			{
				append_tok(&c, 1);
				state = S2;
				continue;
			}
			if(c == '.') // potential start of float
			{
				append_tok(&c, 1);
				state = S3;
				continue;
			}
            put_back(&c, 1);
			if(get_uinteger())
			{
				state = S4;
				continue;
			}
			return get_symatom();

		case S2:
			t = get_safe_char(c);
			if(term_state(t, c, ATOM, STRING))
				return 1;
			if(c == '.')
			{
				append_tok(&c, 1);
				state = S3;
				continue;
			}
			put_back(&c, 1);
			if(get_uinteger())
			{
				state = S4;
				continue;
			}
			return get_symatom();

		case S3:
			if(get_uinteger())
			{
				state = S5;
				continue;
			}
			lex_warning("hosed up floating pointer number?");
			return get_symatom();

		case S4:
			t = get_safe_char(c);
			if(term_state(t, c, ATOM, INTEGER))
				return 1;
			if(c == '.')
			{
				append_tok(&c, 1);
				state = S7;
				continue;
			}
			if(c == 'e' || c == 'E')
			{
				append_tok(&c, 1);
				state = S6;
				continue;
			}
			append_tok(&c, 1);
			return get_symatom();

		case S5:
			t = get_safe_char(c);
			if(term_state(t, c, ATOM, FLOAT))
				return 1;
			if(c == 'e' || c == 'E')
			{
				append_tok(&c, 1);
				state = S6;
				continue;
			}
			append_tok(&c, 1);
			return get_symatom();

		case S6:
			if(get_sinteger())
			{
				token = ATOM;
				type = FLOAT;
				return 1;
			}
			lex_warning("hosed up floating point number?");
			return get_symatom();

		case S7:
			t = get_safe_char(c);
			if(term_state(t, c, ATOM, FLOAT))
				return 1;
			if(c == 'e' || c == 'E')
			{
				append_tok(&c, 1);
				state = S6;
				continue;
			}
			put_back(&c, 1);
			if(get_uinteger())
			{
				state = S5;
				continue;
			}
			lex_warning("hosed up floating pointer number?");
			return get_symatom();

		default:
			oopanic("bogus state in toplev");
			/*NOTREACHED*/
		} // switch
	} // while
	oopanic("bad toplev");
	return 0;
}

int
VcLexer::term_state(int more_input, char c, Token tok, Atom typ)
{
	int terminator = 0;
	if(more_input)
		terminator = is_terminator(c);
	if(!more_input || terminator)
	{
		if(terminator)
			put_back(&c, 1);
		token = tok;
		type = typ;
		return 1;
	}
	return 0;
}




//
// lex from a string
//
VcLexerString::VcLexerString(char *s, VcIO o)
    : VcLexer(o), str(s)
{
	len = strlen(s);
	cur = str;
	len_left = len;
}

VcLexerString::VcLexerString(char *s, long l, VcIO o)
    : VcLexer(o), str(s)
{
	len = l;
	len_left = l;
	cur = str;
}

long
VcLexerString::get_chars(char *& out_buf, long size)
{
	if(size > len_left)
		size = len_left;
	out_buf = cur;
	len_left -= size;
	cur += size;
	decrypt(out_buf, size);
	forward_track_source(out_buf, size);
    cur_char_index = cur - str;
	return size;
}

void
VcLexerString::put_back(char *buf, long cnt)
{
	if(cur - cnt < str)
		oopanic("pushing back too much in string");
	cur -= cnt;
	encrypt(cur, cnt);
	len_left += cnt;
	backward_track_source(buf, cnt);
    cur_char_index -= cnt;
}

int
VcLexerString::no_more_available()
{
	if(len_left == 0)
		return 1;
	return 0;
}


void
VcLexerStringEncrypted::encrypt(char *buf, int buf_len)
{
    enc.mungeback(buf, buf_len);
}
void
VcLexerStringEncrypted::decrypt(char *buf, int buf_len)
{
    enc.munge(buf, buf_len);
}


VcLexerStdio::VcLexerStdio(FILE *i, VcIO o, int ra)
	: VcLexer(o), ifile(i)
{
	cur = ourbuf;
	len_left = 0;
	readahead = ra;
}

long
VcLexerStdio::get_chars(char *& buf_out, long want)
{
	if(len_left == 0)
	{
		cur = ourbuf;
		len_left = fread(ourbuf, 1, readahead, ifile);
		decrypt(ourbuf, len_left);
	}
	if(want > len_left)
		want = len_left;
	buf_out = cur;
	len_left -= want;
	cur += want;
	forward_track_source(buf_out, want);
    cur_char_index += want;
	return want;
}

void
VcLexerStdio::put_back(char *buf, long cnt)
{
	if(cnt <= cur - ourbuf) // all chars in buf
	{
		cur -= cnt;
		len_left += cnt;
	}
	else
	{
		for(long i = cnt - 1; i >= 0; --i)
		{
			encrypt(&buf[i], 1);
			if(ungetc(buf[i], ifile) == EOF)
				oopanic("failed putback");
		}
		len_left = 0; // forced read on next get_char
		cur = ourbuf;
	}
	backward_track_source(buf, cnt);
    cur_char_index -= cnt;
}

int
VcLexerStdio::no_more_available()
{
	if(ferror(ifile) || feof(ifile))
		return 1;
	return 0;
}

void
VcLexerStdioEncrypted::encrypt(char *buf, int len)
{
	enc.mungeback(buf, len);
}
void
VcLexerStdioEncrypted::decrypt(char *buf, int len)
{
	enc.munge(buf, len);
}

#endif
