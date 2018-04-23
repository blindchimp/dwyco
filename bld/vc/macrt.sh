dbg/vc -t 1 -l loadq.lh dbg/t.lh  >lhcompiled.cpp
astyle lhcompiled.cpp

gcc -o run -fpermissive -g -fno-operator-names  -DNO_IOSTREAMS -DLINUX -DEXC_EXPLICIT -DNO_IOSTREAMS 	-DPERFHACKS -DLHOBJ -DFUNCACHE -DVCXFER -DLONGNAMES -DUNIX -DLINUX -DUSE_BERKSOCK -DVCXFER -DVCSOCK -DLHSOCK -DVCCRYPTO -DVCREGEX -DBUFFER_SOCKETS -DVC_COMPRESS -DUSE_ZLIB -DOLD_EXCHANDLE 	 -DDBG -DVCLH -DCACHE_LOOKUPS 	 -I.. -I. -I/Users/dwight/depot/lh2/ins/dbg/inc vctrun.cpp cunit.cpp dbg/vc.a  /Users/dwight/depot/lh2/ins/dbg/lib/crypto5.a /Users/dwight/depot/lh2/ins/dbg/lib/zlib.a /Users/dwight/depot/lh2/ins/dbg/lib/kazlib.a /Users/dwight/depot/lh2/ins/dbg/lib/jenkins.a /Users/dwight/depot/lh2/ins/dbg/lib/dwcls.a 	 -lstdc++ -lm 
