#include <scarab_os.h>
#include <errno.h>
#include <string.h>
#include <windows.h>

void
scarab_os_init()
{
}

const char*
scarab_os_strerror(int err)
{
	return strerror(err);
}

int
scarab_os_geterr()
{
	return WSAGetLastError();
}
