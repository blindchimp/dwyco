Wed Jul 31 10:09:36 MDT 2019

Quick overview of how rando works
---------------------------------

Basic idea: you take a pictue, it gets uploaded.
Provided it passes review, you get a random picture in return from
someone else.
You also get a general idea where in the world the picture was taken
(but only based on IP address, so it isn't that intrusive.)

Yes, this is a clone of the old "Rando" app that was decommissioned 
some time ago. But, it works slightly differently for reasons outlined
below.

This is built using the Dwyco messaging system, so there is no "server based" storage of randos for a user. The reviewed content is stored on a server, but the messages sent to a user are not usually  kept.


General flow:
* User takes a picture, and it is sent to the reviewer. 

* The reviewer receives the picture, 
	* if it does not pass review, a message is sent directly to the user indicating it didn't pass review.
	* if it does pass review, the message is forwarded to the "randobot" for processing.

* The randobot receives the message, and looks up the ip address of the last login of the creator of the message. It maps the IP to a location for future use. It also records the rando in a table.

* The randbot is constantly checking for users that have sent in more randos than they have recieved. If it finds a user that has sent in more randos than they have received, it does the following:
	* Picks a rando more or less at random that the user has not received before.
	* Sends that rando directly to the user, the message will contain the location that the rando was generated with.
	* Sends another auxilliary message to the CREATOR of the rando we just sent. The message contains the hash of the picture, and the locatiom of the user we just sent the rando to.


