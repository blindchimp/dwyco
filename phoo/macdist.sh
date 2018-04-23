export PATH=~/Qt/5.6/clang_64/bin:$PATH:/usr/local/bin
rm -rf phoo.app
rm -f phoo.dmg
make clean
if make -j 4 
then
 id="Developer ID Application: Dwight Melcher"
 strip phoo.app/Contents/MacOS/phoo
 macdeployqt phoo.app -verbose=2
 codesign -s "$id" --file-list - -f -v --deep phoo.app
 hdiutil create -fs HFS+ -volname "Dwyco Phoo" -srcfolder phoo.app phoo.dmg
 cp phoo.dmg macdist
 tar czf macdist/phoo.app.tar.gz phoo.app
else
 echo FAIL FAIL FAIL
 exit 1
fi
