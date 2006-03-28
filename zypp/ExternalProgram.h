/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ExternalProgram.h
 *
 * \todo replace by Blocxx
 *
*/


#ifndef ZYPP_EXTERNALPROGRAM_H
#define ZYPP_EXTERNALPROGRAM_H

#include <map>
#include <string>

#include "zypp/base/ExternalDataSource.h"
#include "zypp/Pathname.h"

namespace zypp {

    /**
     * @short Execute a program and give access to its io
     * An object of this class encapsulates the execution of
     * an external program. It starts the program using fork
     * and some exec.. call, gives you access to the program's
     * stdio and closes the program after use.
     */
    class ExternalProgram : public zypp::externalprogram::ExternalDataSource
    {
    
    public:
      /**
       * Define symbols for different policies on the handling
       * of stderr
       */
      enum Stderr_Disposition {
    	Normal_Stderr,
    	Discard_Stderr,
    	Stderr_To_Stdout,
    	Stderr_To_FileDesc
      };
    
      /**
       * For passing additional environment variables to set
       */
      typedef std::map<std::string,std::string> Environment;
    
      /**
       * Start the external program by using the shell <tt>/bin/sh<tt>
       * with the option <tt>-c</tt>. You can use io direction symbols < and >.
       * @param commandline a shell commandline that is appended to
       * <tt>/bin/sh -c</tt>.
       * @param default_locale whether to set LC_ALL=C before starting
       * @param root directory to chroot into, / or empty to not chroot
       */
      ExternalProgram (std::string commandline,
    		     Stderr_Disposition stderr_disp = Normal_Stderr,
    		     bool use_pty = false, int stderr_fd = -1, bool default_locale = false,
    		     const Pathname& root = "");
    
      /**
       * Start an external program by giving the arguments as an arry of char *pointers.
       * If environment is provided, varaiables will be added to the childs environment,
       * overwriting existing ones.
       */
      
      ExternalProgram();
      
      ExternalProgram (const char *const *argv,
    		     Stderr_Disposition stderr_disp = Normal_Stderr,
    		     bool use_pty = false, int stderr_fd = -1, bool default_locale = false,
    		     const Pathname& root = "");
    
      ExternalProgram (const char *const *argv, const Environment & environment,
    		     Stderr_Disposition stderr_disp = Normal_Stderr,
    		     bool use_pty = false, int stderr_fd = -1, bool default_locale = false,
    		     const Pathname& root = "");
    
      ExternalProgram (const char *binpath, const char *const *argv_1,
    		     bool use_pty = false);
    
    
      ExternalProgram (const char *binpath, const char *const *argv_1, const Environment & environment,
    		     bool use_pty = false);
    
    
      ~ExternalProgram();
    
      int close();
    
      /**
       * Kill the program
       */
      bool kill();
    
      /**
       * Return whether program is running
       */
      bool running();
    
      /**
       * return pid
       * */
      pid_t getpid() { return pid; }
    
      /**
       * origfd will be accessible as newfd and closed (unless they were equal)
       */
      static void renumber_fd (int origfd, int newfd);
    
    protected:
      int checkStatus( int );
    
    private:
    
      /**
       * Set to true, if a pair of ttys is used for communication
       * instead of a pair of pipes.
       */
      bool use_pty;
    
      pid_t pid;
      int _exitStatus;
    
      void start_program (const char *const *argv, const Environment & environment,
    			Stderr_Disposition stderr_disp = Normal_Stderr,
    			int stderr_fd = -1, bool default_locale = false,
    			const char* root = NULL);
    
    };
    
} // namespace zypp

#endif // ZYPP_EXTERNALPROGRAM_H
