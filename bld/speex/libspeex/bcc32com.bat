rem bcc32 -I"/program files/borland/cbuilder6/include" -I. -I../include -I.. -I../include/speex -Drestrict= *.c >k
bcc32 -I/bc45/include -I. -I../include -I.. -I../include/speex -Drestrict= -WM -y -v -W -5 -O2 -vi- *.c >k
tlib speex.lib @speex.tlb
