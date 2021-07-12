while true
do
./nobot "$1" "$2"
if [ -f exit ]
then
	exit 0
fi
sleep 60
done
