#include <string.h>
#include <iostreams.h>
#include <fstream.h>
#include "vclex.h"
//static char Rcsid[] = "$Header: g:/dwight/repo/vc/rcs/tlex.cpp 1.45 1996/11/17 05:58:00 dwight Stable $";

main(int, char**)
{
	//VcLexerString lexer("123.456e+3 .2 0.2 .2E3 .9E-2 2e3 e3 0e0 0e 2 +2 -3 -.3");
	//VcLexerString lexer("0e 2 +2 -3 -.3");
	//VcLexerString lexer("0b0 0t0 1t0 0x0 1x0 0x11 0X0 0o0 b0 b1");
	//VcLexerString lexer("|abc0\\nfoofoo| \\a \\2 2\\22");
	//VcLexerString lexer("< >{}() &* ^2^3^ `abcd'");
	//VcLexerString lexer("0b1111.0e3");
	//VcLexerString lexer("\\ \\|oink\\|oink|");
	//VcLexerString lexer("\\( bar bar \\}");
#if 0
	VcLexerString lexer("|xfer rep: \
representation that is suitable for xfer between hosts on \
a network. may need an internal byte stream type to make \
this play properly. \
\
writing xfer rep:  \
					   \
check to see if the item being written has already been \
written out. if so, there is a cycle, and we must    \
write a reference to the item into the stream instead. \
								 \
register and install a new item in the decode table. if there   \
is a cycle in the structure, this allows later references to  \
simply be sent as a single chit.|");
#endif
	//VcLexerStream lexer(cin, 1);
	ifstream tst("test.lex");
	ofstream to("test.out");
	VcLexerStream lexer(tst, cerr);

	const char *buf;
	long len;
	VcLexer::Atom type;
	VcLexer::Token tok;


	while((tok = lexer.next_token(buf, len, type)) != VcLexer::EOS)
	{
		char tmp[1024];
		strncpy(tmp, buf, len);
		tmp[len] = '\0';
		to << "tokval = \"" << tmp << "\" len = " << len << "\n";
		to << "tok = " << tok << " type = " << type << "\n";
	}

}

void
oopanic(const char *s)
{
	cout << s << "\n";
	exit(1);
}
