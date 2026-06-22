* Use CMake files to understand the project. Ignore the qmake files.
* do not run or test code. i will do that manually.
* ignore anything in the "build" directory
* edit only files in toxd, phoo, and cdc32.
* assume DWYCO_VC_CONV is defined
* GRTLOG macro arguments are "printf-format" then 2 data arguments, for example GRTLOG("hello %s %d", "world", 1)
* GRTLOGVC takes and prints one "class vc" argument
* focus is on toxbridge and toxd
* for objects of type "vc", when reading the type of a string, it is always "VC_STRING". VC_BSTRING is only used during contruction to indicate it is a binary string, and to ignore 0 characters and use just the length during the creation.
