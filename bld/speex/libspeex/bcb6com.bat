bcc32 -O2 -5 -ff -w-par -w-inl -Ve -r -a8 -k -y -v -vi- -b- -Vx -tW -tWM -I"/program files/borland/cbuilder6/include" -I. -I../include -I.. -I../include/speex -Drestrict= *.c >k
rem bcc32 -I/bc45/include -I. -I../include -I.. -I../include/speex -Drestrict= -WM -y -v -W -5 -O2 -vi- *.c >k
tlib speex.lib @speex.tlb
