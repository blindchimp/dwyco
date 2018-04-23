#include "dlli.h"
#include <stdlib.h>

int
main(int argc, char **argv)
{
if(argc != 2) return 0;
int port = atoi(argv[1]);
if(port < 1024 || port > 65535) return 0;
dwyco_background_processing(port, 1, 0, 0, 0);
return 0;
}
