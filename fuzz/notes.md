using aflplusplus to fuzz the deserializer

using aflplusplus to fuzz the deserializer

i did this because the deserializer is the only "outward facing" part of the servers, and it is easy to crash it. thankfully, noone has bothered to figure it out, so there have been no problems ca. 2024

using aflplusplus from
```
https://github.com/AFLplusplus/AFLplusplus
```

version 4.1 or so on ubuntu 20.04

your best bet is to use the docker container that has all the compilers and stuff built-in

```
docker pull aflplusplus/aflplusplus
docker run -ti -v /location/of/your/target:/src aflplusplus/aflplusplus
```

eg.
```
docker run -ti -v /home/dwight/fuck:/src aflplusplus/aflplusplus
```
this maps /src (inside the container) to ~/fuck.

do
```
cd ~/fuck
mkdir git
cd git
git clone https://github.com/blindchimp/dwyco
```

In the fuzz directory, the "conf.pri" file contains the changes that will make sure when you run qmake inside the container the right thing is built.

inside the container, you will also need to install the qt5 qmake, which is done like this:
```
apt install qt5-qmake
```
Note that you do NOT need to install qt itself, just qmake. you can check that it is installed:
```
QT_SELECT=5 qmake -v
```
to avoid having to do this every time you start the afl container, use whatever docker things are needed to create a snapshot of the container. warning: using docker prune seems to kill those snapshots...

inside the container, use the "fuzz/build.sh" script to build the instrumented LH binary and library
```
cd /src/git/dwyco
sh fuzz/build.sh
```

there are two approaches to doing the fuzzing. the first one i did used the vanilla LH interpreter, with the following script. it simply loads the input file from the fuzzer and tries to deserialize it. this is slower, but it might allow some fuzzing of the interpreter along with the deserializer.
```
getfile(openfile(__argv[0] rb) foo)
```
this is the idea:
```
docker exec -it 2965e17f9217 /bin/bash -c "cd /src/git/build-lh/vccmd;/AFLplusplus/afl-fuzz -M f1 -i ../../../input -o fuck -- ./vccmd f.lh @@"
```

a better way if all you want to do is check the xfer_in part of the system is to use the "fuzz" binary that is generated that only contains a small C++ main stub to call xfer_in. this is much faster and gives you better control over the xfer_in constraints.

the initial input samples are created using this LH script:
```
putfile(openfile(t1 wb) map(vector(1 2) vector( 3 4)))
putfile(openfile(t2 wb) set(vector(1 2) vector( 3 4)))
putfile(openfile(t3 wb) tree(vector(1 2) vector( 3 4)))
putfile(openfile(t4 wb) vector(vector(1 2) vector( 3 4)))
putfile(openfile(t5 wb) map(vector(1 2) vector( 3 4)))
putfile(openfile(t6 wb) map(vector(1. 2) vector( 3 4)))
putfile(openfile(t7 wb) map(vector(1. |2|) vector( 3 4)))
lbind(a vector(a b))
append(<a> <a>)
putfile(openfile(t8 wb) <a>)
lbind(b tree(vector(a b)))
putfile(openfile(t9 wb) set(<b> <b>))
putfile(openfile(t10 wb) div(1. 3.))
putfile(openfile(t11 wb) div(1. 2.))
putfile(openfile(t16 wb) div(1. 0.))
putfile(openfile(t12 wb) ||)
putfile(openfile(t13 wb) |\0\f\t|)
putfile(openfile(t14 wb) string(0 0xff))
putfile(openfile(t15 wb) string(vector2(1024 0)))
```
outside the container, copy the t* files to a dir called ~/fuck/input. these are the seeds the fuzzer uses to construct new cases. i haven't checked the complete coverage caused by this set of seeds, but without the xfer_in_constraints enabled, this will create lots of crashes. of course, with the constraints enabled, there should be no crashes.

to start the main fuzzer, do this (outside the container in a shell window):
```
docker exec -it <<container-id>> /bin/bash -c "cd /src/git/build-afl/fuzz;/AFLplusplus/afl-fuzz -M f1 -i ../../../input -o fuck -- ./fuzz @@"
```

replace the container id with the number you get when you run the original aflplusplus container.

to use more cores, start secondary fuzzers:
```
docker exec -it <<same-container-id>> /bin/bash -c "cd /src/git/build-afl/fuzz;/AFLplusplus/afl-fuzz -S f2 -i ../../../input -o fuck -- ./fuzz @@"
```
note: you'll want to change the "f2" above to something else, like "f3", "f4", etc.

the fuzzing results are written to the "fuck" folder. i ran the fuzzing overnight several times, and new cases were few and far between with no crashes, so i'm confident these changes make it harder to crash the server at a low level anyway. there are still plenty of application level problems that need to be figured out, but that will come later.
