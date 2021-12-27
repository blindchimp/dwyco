
/* ===
; Copyright (c) 1995-present, Dwyco, Inc.
; 
; This Source Code Form is subject to the terms of the Mozilla Public
; License, v. 2.0. If a copy of the MPL was not distributed with this file,
; You can obtain one at https://mozilla.org/MPL/2.0/.
*/
#ifndef VCLEX_H
#define VCLEX_H
// $Header: g:/dwight/repo/vc/rcs/vclex.h 1.47 1998/08/06 04:46:08 dwight Exp $

#include "dwgrows.h"
#include "enc.h"
#include "vc.h"
#include "dwstr.h"

class VcLexer
{
public:
	VcLexer(VcIO o);
	virtual ~VcLexer() {}
	enum Token {BOGUS, ATOM, LBRACKET, RBRACKET,
		LBRACE, RBRACE, RPAREN, LPAREN, LTICK, RTICK, UPARROW,
		LSBRACK, RSBRACK, EOS};
	enum Atom {BOGUS_ATOM, STRING, INTEGER, FLOAT};

        int emit_lexical_warnings;

        //virtual Token next_token();
        // WARNING: the returned char pointers are reset
        // on a call to next_token, if you need to keep the
        // string, you must copy it out right after the call to
        // next_token.
	virtual Token next_token(const char*&, long& len, Atom& );
	virtual long token_linenum();
	virtual long token_linenum_start_scan();
        virtual DwString input_description();
	virtual void set_input_description(const char *); 

	virtual VcIO get_err_strm() { return err_strm;}

	int lexical_error;	// 1 if input is unlexable
        long char_start_token;  // absolute char index of start of token
        long char_end_token;    // absolute chat index of end of token

private:

	DwGrowingString tokval;	// token is accumulated here
	int base;               // base if integer recognized
	int curbase;
	Token token;            // set to token class
	Atom type;              // if token == ATOM, gives type
	VcIO err_strm;      // errors are logged to this stream
	// default source tracking, line numbers only
	long lines_read;        // number of newline chars read
	long start_scan;        // line number when "next_token" entered
	long start_token;       // line number of 1st char of token
	long end_token;         // line number of last char of token

	int is_special(char c);
	int is_terminator(char);
	int is_digit(char, int);
	void append_tok(const char *, int);
	void mark_token();
	void putback_to_mark();
	void commit_to_mark();

	int get_literal();
	int get_sinteger();
	int get_uinteger();
	int get_symatom();
	int get_safe_char(char &);
	int peek_char(char &);
	int get_toplev();
	int get_special(char);
	int term_state(int, char, Token, Atom);
	void eat_comment();

	void lex_error(const char *);
	void lex_warning(const char *);

    virtual void set_beginning_of_token();
	virtual void set_end_of_token();
	virtual void set_beginning_of_scan();
	
protected:
        DwString inp_desc;
        long cur_char_index;
	void forward_track_source(const char *, long);
	void backward_track_source(const char *, long);

	virtual long get_chars(char *& buf_out, long len) = 0;
	virtual void put_back(char *buf, long cnt) = 0;
	virtual int no_more_available() = 0;
	virtual long token_len();

};

class VcLexerString : public VcLexer
{

public:
	VcLexerString(char *, VcIO);
	VcLexerString(char *, long, VcIO);

protected:
        char * const str;
	long len;
        char *cur;
	long len_left;

	long get_chars(char *&, long);
	void put_back(char *, long);
	int no_more_available();
	virtual void encrypt(char *buf, int len) {}
	virtual void decrypt(char *buf, int len) {}

};
#define VCLEX_READAHEAD 128

// WARNING: this class mauls your string in place
class VcLexerStringEncrypted : public VcLexerString
{
public:
	VcLexerStringEncrypted(char *s, long len, VcIO o) :
		VcLexerString(s, len, o) {}

private:
	ns_vc::Enc enc;
	virtual void encrypt(char *buf, int len);
	virtual void decrypt(char *buf, int len);
};


#include <stdio.h>
class VcLexerStdio : public VcLexer
{
public:
	VcLexerStdio(FILE *in, VcIO err_out, int = VCLEX_READAHEAD);

private:
	FILE *ifile;
	char ourbuf[VCLEX_READAHEAD];
	int len_left;
	char *cur;
	int readahead;

	long get_chars(char *&, long);
	void put_back(char *, long);
	int no_more_available();
	virtual void encrypt(char *buf, int len){}
	virtual void decrypt(char *buf, int len){}

};

class VcLexerStdioEncrypted : public VcLexerStdio
{
public:
	VcLexerStdioEncrypted(FILE *in, VcIO err_out, int r = VCLEX_READAHEAD) :
		VcLexerStdio(in, err_out, r) {}

private:
	ns_vc::Enc enc;
	virtual void encrypt(char *buf, int len);
	virtual void decrypt(char *buf, int len);
};

#endif
