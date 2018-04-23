#include "dwdate.h"
#include <stdio.h>
void
oopanic(char *)
{
}

main(int, char **)
{
	printf("%s", DwTime().AsString().c_str());
}
