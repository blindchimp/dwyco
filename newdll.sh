
# bld/cdc32 is the main api header
find . -name dlli.h >/tmp/k
md5sum `cat /tmp/k`
find . -name dlli.h | grep -v cdc32 >/tmp/k
for i in `cat /tmp/k`
do
	cp -v bld/cdc32/dlli.h $i
done
