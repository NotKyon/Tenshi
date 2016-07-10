#include "AppPath.hpp"

#include <errno.h>

// Mac OS X
#if defined(__APPLE__)
# include <mach-o/dyld.h>
char *Ax::System::GetAppPath(char *buff, size_t n) {
	uint32_t size;

	size = n;

	if (_NSGetExecutablePath(buff, &size)==0) {
		errno = ERANGE;
		return (char *)0;
	}

	return buff;
}
// Linux
#elif defined(linux)||defined(__linux)
# include <sys/types.h>
# include <unistd.h>
char *Ax::System::GetAppPath(char *buff, size_t n) {
	pid_t pid;
	char name[256];
	int r;

	pid = getpid();

	snprintf(name, sizeof(name), "/proc/%i/exe", (int)pid);

	if ((r = readlink(name, buff, n))==-1)
		return (char *)0;
	else if(r >= n) {
		errno = ERANGE;
		return (char *)0;
	}

	buff[r] = 0;
	return buff;
}
// Solaris
#elif (defined(sun)||defined(__sun))&&(defined(__SVR4)||defined(__svr4__))
# include <stdlib.h>
char *Ax::System::GetAppPath(char *buff, size_t n) {
	strncpy(buff, getexecname(), n-1);
	buff[n-1] = 0;

	return buff;
}
// FreeBSD
#elif defined(__FreeBSD__)
# include <sys/sysctl.h>
char *Ax::System::GetAppPath(char *buff, size_t n) {
	size_t size;
	int name[4];

	name[0] = CTL_KERN;
	name[1] = KERN_PROC;
	name[2] = KERN_PROC_PATHNAME;
	name[3] = -1;

	if (sysctl(name, sizeof(name)/sizeof(name[0]), buff, &size, 0, 0)==-1)
		return (char *)0;

	return buff;
}
// UNIX (general)
#elif defined(unix)||defined(__unix__)
# include <stdio.h>
# include <stdlib.h>
char *Ax::System::GetAppPath(char *buff, size_t n) {
	char *s;

	if ((s=getenv("_"))!=(char *)0) {
		snprintf(buff, s, n-1);
		buff[n-1] = 0;

		return buff;
	}

	// TODO: Write a port for this UNIX
	fprintf(stderr, "Ax::System::GetAppPath: your unix distribution isn't supported.\n");
	fflush(stderr);

	return (char *)0;
}
// Windows
#elif defined(_WIN32)
# undef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN 1
# include <Windows.h>
char *Ax::System::GetAppPath(char *buff, size_t n) {
# ifdef _WIN64
	if (!GetModuleFileNameA(NULL, buff, n > 0xFFFFFFFF ? 0xFFFFFFFF : (DWORD)n))
		return (char *)0;
# else
	if (!GetModuleFileNameA(NULL, buff, (DWORD)n))
		return (char *)0;
#endif

	return buff;
}
// Unknown
#else
# error Ax: GetAppPath() not on this platform
#endif

char *Ax::System::GetAppDir( char *buff, size_t n )
{
	if( !GetAppPath( buff, n ) )
	{
		return nullptr;
	}

#if defined( _WIN32 )
	char *const p = strrchr( buff, '\\' );
	if( p != nullptr )
	{
		*p = '\0';
	}
#else
	char *const p = strrchr( buff, '/' );
	if( p != nullptr && p != buff ) //file could be at root on janky OS
	{
		*p = '\0';
	}
#endif

	return buff;
}
