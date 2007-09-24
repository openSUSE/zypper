/* A setuid-root wrapper for zypp-checkpatches */

/* setgid, umask and open */
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <fcntl.h>
/* clearenv */
#include <stdlib.h>
/* chdir, execl, setuid, exit */
#include <unistd.h>
/* perror */
#include <stdio.h>

#define WRAPPER_ERROR 101

const char *app = "/usr/sbin/zypp-checkpatches";

int main (void)
{
    /* see http://rechner.lst.de/~okir/blackhats/node41.html */
    while (1) {
	int fd = open("/dev/null", O_RDWR);
	if (fd < 0)
	    return WRAPPER_ERROR;
	if (fd > 2) {
	    close(fd);
	    break;
	}
    }

    /* see http://rechner.lst.de/~okir/blackhats/node35.html */
    int fd = getdtablesize();
    while (fd-- > 2)
        close(fd);

    /* cd / to avoid NFS problems */
    if (chdir ("/")) {
	perror ("chdir");
	return WRAPPER_ERROR;
    }
    /* do not look at argv... done */
    /* clear environment */
    if (clearenv ()) {
	fprintf (stderr, "clearenv failed\n");
	return WRAPPER_ERROR;
    }
    /* set minimal environment... done */
    /* prevent the user from sending signals */

    if (initgroups("root", 0) != 0 || setgid (0) != 0) {
    	perror ("setuid");
	fprintf (stderr, "Forgot to chmod this program?\n");
	return WRAPPER_ERROR;
    }

    if (setuid (0)) {
	perror ("setuid");
	fprintf (stderr, "Forgot to chmod this program?\n");
	return WRAPPER_ERROR;
    }

    umask(0022);

    /* execute the real application */
    execl (app, app, (char *) NULL);

    /* if we are still here, it has failed */
    perror ("exec");
    return WRAPPER_ERROR;
}
