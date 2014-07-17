/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/ExternalProgram.h
*/


#ifndef ZYPP_EXTERNALPROGRAM_H
#define ZYPP_EXTERNALPROGRAM_H

#include <unistd.h>

#include <map>
#include <string>
#include <vector>

#include "zypp/base/ExternalDataSource.h"
#include "zypp/Pathname.h"

namespace zypp {

    /**
     * @short Execute a program and give access to its io
     * An object of this class encapsulates the execution of
     * an external program. It starts the program using fork
     * and some exec.. call, gives you access to the program's
     * stdio and closes the program after use.
     *
     * \code
     *
     * const char* argv[] =
     * {
     *     "/usr/bin/foo,
     *     "--option1",
     *     "--option2",
     *     NULL
     * };
     *
     * ExternalProgram prog( argv,
     *                        ExternalProgram::Discard_Stderr,
     *                        false, -1, true);
     * string line;
     * for(line = prog.receiveLine();
     *     ! line.empty();
     *     line = prog.receiveLine() )
     * {
     *     stream << line;
     * }
     * prog.close();
     *
     * \endcode
     */
    class ExternalProgram : public zypp::externalprogram::ExternalDataSource
    {

    public:

      typedef std::vector<std::string> Arguments;

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
       * @param root directory to chroot into; or just 'cd' if '/'l;  nothing if empty
       */
      ExternalProgram (std::string commandline,
    		     Stderr_Disposition stderr_disp = Normal_Stderr,
    		     bool use_pty = false, int stderr_fd = -1, bool default_locale = false,
    		     const Pathname& root = "");

      /**
       * Start an external program by giving the arguments as an arry of char *pointers.
       * If environment is provided, varaiables will be added to the childs environment,
       * overwriting existing ones.
       *
       * Initial args starting with \c # are discarded but some are treated specially:
       * 	#/[path] - chdir to /[path] before executing
       *
       * Stdin redirection: If the \b 1st argument starts with a \b '<', the remaining
       * part is treated as file opened for reading on standard input (or \c /dev/null
       * if empty).
       * \code
       *   // cat file /tmp/x
       *   const char* argv[] = { "</tmp/x", "cat", NULL };
       *   ExternalProgram prog( argv );
       * \endcode
       */

      ExternalProgram();

      ExternalProgram (const Arguments &argv,
    		     Stderr_Disposition stderr_disp = Normal_Stderr,
    		     bool use_pty = false, int stderr_fd = -1, bool default_locale = false,
    		     const Pathname& root = "");

      ExternalProgram (const Arguments &argv, const Environment & environment,
    		     Stderr_Disposition stderr_disp = Normal_Stderr,
    		     bool use_pty = false, int stderr_fd = -1, bool default_locale = false,
    		     const Pathname& root = "");

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

      /** Wait for the progamm to complete. */
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

      /** The command we're executing. */
      const std::string & command() const
      { return _command; }

      /** Some detail telling why the execution failed, if it failed.
       * Empty if the command is still running or successfully completed.
       *
       * \li <tt>Can't open pty (%s).</tt>
       * \li <tt>Can't open pipe (%s).</tt>
       * \li <tt>Can't fork (%s).</tt>
       * \li <tt>Command exited with status %d.</tt>
       * \li <tt>Command was killed by signal %d (%s).</tt>
      */
      const std::string & execError() const
      { return _execError; }

      /**
       * origfd will be accessible as newfd and closed (unless they were equal)
       */
      static void renumber_fd (int origfd, int newfd);

    public:

      /**
       * Redirect all command output to an \c ostream.
       * Returns when the command has completed.
       * \code
       *   std::ostringstream s;
       *   ExternalProgram("pwd") >> s;
       *   SEC << s.str() << endl;
       * \endcode
       * \code
       *   std::ostringstream s;
       *   ExternalProgram prog("ls -l wrzl");
       *   prog >> s;
       *   if ( prog.close() == 0 )
       *     MIL << s.str() << endl;
       *   else
       *     ERR << prog.execError() << endl;
       * \endcode
       */
      std::ostream & operator>>( std::ostream & out_r );

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
      /** Store the command we're executing. */
      std::string _command;
      /** Remember execution errors like failed fork/exec. */
      std::string _execError;

      void start_program (const char *const *argv, const Environment & environment,
    			Stderr_Disposition stderr_disp = Normal_Stderr,
    			int stderr_fd = -1, bool default_locale = false,
    			const char* root = NULL);

    };


  namespace _ExternalProgram
  {
    /** Helper providing pipe FDs for \ref ExternalProgramWithStderr.
     * Moved to a basse class because the pipe needs to be initialized
     * before the \ref ExternalProgram base class is initialized.
     * \see \ref ExternalProgramWithStderr
     */
    struct EarlyPipe
    {
      enum { R=0, W=1 };
      EarlyPipe();
      ~EarlyPipe();
      void closeW()		{ if ( _fds[W] != -1 ) { ::close( _fds[W] ); _fds[W] = -1; } }
      FILE * stderr()		{ return _stderr; }
      protected:
	FILE * _stderr;
	int _fds[2];
    };
  }

  /** ExternalProgram extended to offer reading programs stderr.
   * \see \ref ExternalProgram
   */
  class ExternalProgramWithStderr : private _ExternalProgram::EarlyPipe, public ExternalProgram
  {
    public:
      ExternalProgramWithStderr( const Arguments & argv_r )
	: ExternalProgram( argv_r, Stderr_To_FileDesc, /*use_pty*/false, _fds[W] )
      { _initStdErr(); }

      ExternalProgramWithStderr( const Arguments & argv_r, const Environment & environment_r )
        : ExternalProgram( argv_r, environment_r, Stderr_To_FileDesc, /*use_pty*/false, _fds[W] )
      { _initStdErr(); }

    public:
      /** Return \c FILE* to read programms stderr (O_NONBLOCK set). */
      using _ExternalProgram::EarlyPipe::stderr;

      /** Read data up to \c delim_r from stderr (nonblocking).
       * \note If \c delim_r is '\0', we read as much data as possible.
       * \return \c false if data are not yet available (\c retval_r remains untouched then).
       */
      bool stderrGetUpTo( std::string & retval_r, const char delim_r, bool returnDelim_r = false );

      /** Read next complete line from stderr (nonblocking).
       * \return \c false if data are not yet available (\c retval_r remains untouched then).
       */
      bool stderrGetline( std::string & retval_r, bool returnDelim_r = false  )
      { return stderrGetUpTo( retval_r, '\n', returnDelim_r ); }

    private:
      /** Close write end of the pipe (childs end). */
      void _initStdErr()
      { closeW(); }

    private:
      std::string _buffer;
  };

} // namespace zypp

#endif // ZYPP_EXTERNALPROGRAM_H
