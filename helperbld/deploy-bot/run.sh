while true
do
./dwycobot "$1" "$2"
if [ -f exit ]
then
	exit 0
fi
sleep 60
done
