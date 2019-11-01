while true
do
LD_LIBRARY_PATH=. ./clbot /home/dwight/local/db/profiles.sqlite3 /home/dwight/local/db/iy.sqlite3
if [ -f exit ]
then
	exit 0
fi
sleep 60
done
