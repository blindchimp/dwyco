This is a collection of components that
together can build helper apps that run on the client side with CDC-X.

Also, some simple bots that can be run just about anywhere, but usually
end up on the same servers as the backend servers.

The helpers are:
* ftpreview - A server-side video previewer, simply snags first frame of video into an image file. Used to produce static web pages from video profiles.

* dwycobg - A program that runs to upload messages after main UI has exited. The client can run without this, so you can leave it out of a deployment if it causes too many problems (some virus checkers think this behavior is suspicious and they block running it.)

* xferdl - Downloads an update in the background. Used for the gated-updater, but gated updater has a variety of problems so it isn't used much.

* qdwyrun - Qt4 based launcher, whose main purpose is to check for an update and apply it before starting the main program. Used on Windows and Linux.
