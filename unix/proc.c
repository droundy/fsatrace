#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined __linux__
#include <libproc.h>
#endif
#include "../proc.h"

void
procPath(char *fullpath)
{
#if defined __linux__
	char		exepath   [64];
	snprintf(exepath, sizeof(exepath), "/proc/%d/exe", getpid());
	readlink(exepath, fullpath, PATH_MAX);
#else
			proc_pidpath  (getpid(), fullpath, PATH_MAX);
#endif
}

enum procerr
procRun(const char *cmd, char **args)
{
	int		ret;
	int		r;
	int		child;
	char		so        [PATH_MAX];
	char		fullpath  [PATH_MAX];
	procPath(fullpath);
	{
#if defined __linux__
		char		exepath   [64];
		snprintf(so, sizeof(so), "%s.so", fullpath);
		setenv("LD_PRELOAD", so, 1);
#else
		snprintf(so, sizeof(so), "%s.so", fullpath);
		setenv("DYLD_INSERT_LIBRARIES", so, 1);
		setenv("DYLD_FORCE_FLAT_NAMESPACE", "1", 1);
#endif
	}
	child = fork();
	switch (child) {
	case -1:
		ret = ERR_PROC_FORK;
		break;
	case 0:
		execvp(cmd, args);
		ret = ERR_PROC_EXEC;
		break;
	default:
		r = wait(&ret);
		ret = r == -1 ? ERR_PROC_WAIT : WEXITSTATUS(ret);
	}
	return ret;
}