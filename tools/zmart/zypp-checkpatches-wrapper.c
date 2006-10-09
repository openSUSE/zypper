/* A setuid-root wrapper for zypp-checkpatches */

/* clearenv */
#include <stdlib.h>
/* chdir, execl */
#include <unistd.h>
/* perror */
#include <stdio.h>

#define WRAPPER_ERROR 101

const char *app = "/usr/sbin/zypp-checkpatches";

int main (void) {
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
    /* execute the real application */
    execl (app, app, (char *) NULL);

    /* if we are still here, it has failed */
    perror ("exec");
    return WRAPPER_ERROR;
}
