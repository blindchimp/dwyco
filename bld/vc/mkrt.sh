#!/bin/sh

dbg/vc -t 1 -l loadq.lh $1  >lhcompiled.cpp
astyle lhcompiled.cpp

gcc -DINLINE_SOME -O3 -o run -fpermissive -g -fno-operator-names  -DNO_IOSTREAMS -DLINUX -DEXC_EXPLICIT -DNO_IOSTREAMS 	-DPERFHACKS -DLHOBJ -DFUNCACHE -DVCXFER -DLONGNAMES -DUNIX -DLINUX -DUSE_BERKSOCK -DVCXFER -DVCSOCK -DLHSOCK -DVCCRYPTO -DVCREGEX -DBUFFER_SOCKETS -DVC_COMPRESS -DUSE_ZLIB -DOLD_EXCHANDLE 	 -DDBG -DVCLH -DCACHE_LOOKUPS 	 -I.. -I. -I/home/dwight/depot/lh2/ins/dbg/inc vctrun.cpp cunit.cpp hacked_sqlite3.cpp hacked_spread.xml.cpp dbg/vc.a /home/dwight/depot/lh2/ins/dbg/lib/gnustr.a /home/dwight/depot/lh2/ins/dbg/lib/crypto5.a /home/dwight/depot/lh2/ins/dbg/lib/zlib.a /home/dwight/depot/lh2/ins/dbg/lib/kazlib.a /home/dwight/depot/lh2/ins/dbg/lib/jenkins.a /home/dwight/depot/lh2/ins/dbg/lib/dwcls.a 	 /home/dwight/depot/lh2/ins/dbg/lib/libspread.a -lsqlite3 -lstdc++ -lm -pthread -lrt
