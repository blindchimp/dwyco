# this variable is queried in Android.mk in various
# submodules to allow customization based on the app
# the submodule is included within.
# for example, with rando, cdc32 configures itself to
# a stripped down version that doesn't include a lot of
# the built-in multimedia stuff that isn't used.
DWYCO_APP := "rando"
