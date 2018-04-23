ns="namespace jhead {"
ens="}"

for i in exif.c gpsinfo.c iptc.c jhead.c jpgfile.c jpgqguess.c makernote.c myglob.c paths.c
do
cpp=${i}pp
echo $ns > $cpp
echo "#include \"$i\"" >>$cpp
echo $ens >>$cpp
done
