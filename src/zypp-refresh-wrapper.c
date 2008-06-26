/* A setuid-root wrapper for zypp-refresh utility */

/* setgid, umask and open */
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

const char *app = "/usr/sbin/zypp-refresh";
/* const char *arg1 = ""; */

char *lang = NULL;

int main (void)
{
  /* see http://rechner.lst.de/~okir/blackhats/node41.html */
  while (1)
  {
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
  while (--fd > 2)
      close(fd);

  /* cd / to avoid NFS problems */
  if (chdir ("/"))
  {
    perror ("chdir");
    return WRAPPER_ERROR;
  }

  /* save language */
  lang = getenv("LANG");

  /* do not look at argv... done */
  /* clear environment */
  if (clearenv ())
  {
    fprintf (stderr, "clearenv failed\n");
    return WRAPPER_ERROR;
  }


  /* set minimal environment... done */
  /* prevent the user from sending signals */

  if (initgroups("root", 0) != 0 || setgid (0) != 0)
  {
    fprintf (stdout,
        "Unable to refresh repositories because /usr/sbin/zypp-refresh-wrapper"
        " helper programm is not set SUID root.\n"
        "This problem might be solved by setting 'File Permissons' in YaST"
        " 'Local Security' tab to 'easy' or by modifying"
        " /etc/permissions.local\n");
    return WRAPPER_ERROR;
  }

  if (setuid (0) != 0)
  {
    // perror ("setuid");
    // Forgot to chmod this program?
    fprintf (stdout,
        "Unable to refresh repositories because /usr/sbin/zypp-refresh-wrapper"
        " helper programm is not set SUID root.\n"
        "This problem might be solved by setting 'File Permissons' in YaST"
        " 'Local Security' tab to 'easy' or by modifying"
        " /etc/permissions.local\n");
    return WRAPPER_ERROR;
  }

  umask(0022);

  /* set language */
  if (lang != NULL)
    setenv("LANG", lang, 1);

  /* execute the real application */
  execl (app, app, (char *) NULL);

  /* if we are still here, it has failed */
  perror ("execl");
  return WRAPPER_ERROR;
}
