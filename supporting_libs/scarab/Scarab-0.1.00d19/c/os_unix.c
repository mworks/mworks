#include "scarab_os.h"
#include <errno.h>
#include <string.h>

void
scarab_os_init(void)
{
}

const char*
scarab_os_strerror(int err)
{
	return strerror(err);
}

int
scarab_os_geterr(void)
{
	return errno;
}
