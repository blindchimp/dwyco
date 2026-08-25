* Use CMake files to understand the project. Ignore the qmake files.
* do not run or test code. i will do that manually.
* ignore anything in the "build" directory
* ignore trc_wrappers.cpp, it is auto-generated for debugging the api.
* when you make changes, create a temporary directory to check if it compiles.
* edit only files in toxd, phoo, and cdc32.
* assume DWYCO_VC_CONV is defined
* GRTLOG macro arguments are "printf-format" then 2 data arguments, for example GRTLOG("hello %s %d", "world", 1)
* GRTLOGVC takes and prints one "class vc" argument
* focus is on toxbridge and toxd
* for objects of type "vc", when reading the type of a string, it is always "VC_STRING". VC_BSTRING is only used during contruction to indicate it is a binary string, and to ignore 0 characters and use just the length during the creation.
* EXTREMELY CRITICAL: the dwyco api in dlli.h is a vanilla C interface. uids are BINARY strings with associated lengths. message id's (mid's) are ASCII character strings that are 0 terminated. 
* SUPER EXTREMELY CRITICAL: the hand-written database code stores all binary uid's as HEXADECIMAL ASCII strings (uid.toHex()). the reason this is done is to avoid having to use "blobs" everywhere for very commmon uid columns in tables. it is also a lot easier to debug a database with hex strings.
* as a convention, convert uid's to hex only near the site where they are about to be stored in the database. when reading them out of the database, convert them back to binary for consumption by callers.
