This is a collection of components that
together can build helper apps that run on the client side with CDC-X.

Also, some simple bots that can be run just about anywhere, but usually
end up on the same servers as the backend servers.

bots:
* randobot - the bot that is used to redistribute random content using the "rando" app. NOTE NOTE: this must be compiled with the Qt5 that is installed on whatever Ubuntu distro you are running it with, current 16.04. The way this is done is this:

cd git/dwyco
export QT_SELECT=5
sh helperbld/buildbots.sh

The other bots can be build with qt4 (actually easier since it is easier to deploy qt4.)


The helpers are:
* ftpreview - A server-side video previewer, simply snags first frame of video into an image file. Used to produce static web pages from video profiles.

* dwycobg - A program that runs to upload messages after main UI has exited. The client can run without this, so you can leave it out of a deployment if it causes too many problems (some virus checkers think this behavior is suspicious and they block running it.)

* xferdl - Downloads an update in the background. Used for the gated-updater, but gated updater has a variety of problems so it isn't used much.

* qdwyrun - Qt4 based launcher, whose main purpose is to check for an update and apply it before starting the main program. Used on Windows and Linux.
