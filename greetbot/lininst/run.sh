while true
do
./greetbot Announcer "I am a bot. I make announcements and send test messages. To get a test message, send me a message saying 'please respond'." ./announcements
if [ -f exit ]
then
	exit 0
fi
sleep 60
done
