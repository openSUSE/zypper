/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/Exception.h
 *
*/
#ifndef ZYPP_BASE_EXCEPTION_H
#define ZYPP_BASE_EXCEPTION_H

#include <iosfwd>
#include <string>
#include <list>
#include <stdexcept>

#include "zypp/base/Errno.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace exception_detail
  { /////////////////////////////////////////////////////////////////

    /** Keep _FILE_, _FUNCTION_ and _LINE_.
     * Construct it using the \ref ZYPP_EX_CODELOCATION macro.
    */
    struct CodeLocation
    {
      friend std::ostream & operator<<( std::ostream & str, const CodeLocation & obj );

      /** Ctor */
      CodeLocation()
      : _line( 0 )
      {}

      /** Ctor */
      CodeLocation( const std::string & file_r,
                    const std::string & func_r,
                    unsigned            line_r )
      : _file( file_r ), _func( func_r ), _line( line_r )
      {}

      /** Location as string */
      std::string asString() const;

    private:
      std::string _file;
      std::string _func;
      unsigned    _line;
    };
    ///////////////////////////////////////////////////////////////////

    /** Create CodeLocation object storing the current location. */
    //#define ZYPP_EX_CODELOCATION ::zypp::exception_detail::CodeLocation(__FILE__,__FUNCTION__,__LINE__)
#define ZYPP_EX_CODELOCATION ::zypp::exception_detail::CodeLocation(( *__FILE__ == '/' ? strrchr( __FILE__, '/' ) + 1 : __FILE__ ),__FUNCTION__,__LINE__)

    /** \relates CodeLocation Stream output */
    std::ostream & operator<<( std::ostream & str, const CodeLocation & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace exception_detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Exception
  /** Base class for Exception.
   *
   * Exception offers to store a message string passed to the ctor.
   * Derived classes may provide additional information. Overload
   * \ref dumpOn to provide a proper error text.
   *
   * \li Use \ref ZYPP_THROW to throw exceptions.
   * \li Use \ref ZYPP_CAUGHT If you caught an exceptions in order to handle it.
   * \li Use \ref ZYPP_RETHROW to rethrow a caught exception.
   *
   * The use of these macros is not mandatory. but \c ZYPP_THROW and
   * \c ZYPP_RETHROW will adjust the code location information stored in
   * the Exception. All three macros will drop a line in the logfile.

   * \code
   *  43   try
   *  44     {
   *  45       try
   *  46         {
   *  47           ZYPP_THROW( Exception("Something bad happened.") );
   *  48         }
   *  49       catch ( Exception & excpt )
   *  50         {
   *  51           ZYPP_RETHROW( excpt );
   *  52         }
   *  53
   *  54     }
   *  55   catch ( Exception & excpt )
   *  56     {
   *  57       ZYPP_CAUGHT( excpt );
   *  58     }
   * \endcode
   * The above produces the following log lines:
   * \code
   *  Main.cc(main):47 THROW:    Main.cc(main):47: Something bad happened.
   *  Main.cc(main):51 RETHROW:  Main.cc(main):47: Something bad happened.
   *  Main.cc(main):57 CAUGHT:   Main.cc(main):51: Something bad happened.
   * \endcode
   *
   *
   * Class Exception now offers a history list of message strings.
   * These messages should describe what lead to the exception.
   *
   * The Exceptions message itself is NOT included in the history.
   *
   * Rethrow, remembering an old exception:
   * \code
   * try
   * {
   *   ....
   * }
   * catch( const Exception & olderr_r )
   * {
   *    ZYPP_CAUGHT( olderr_r )
   *    HighLevelException newerr( "Something failed." );
   *    newerr.rember( olderr_r );
   *    ZYPP_THROW( newerr );
   * }
   * \endcode
   *
   * Print an Exception followed by it's history if available:
   * \code
   * Exception error;
   * ERR << error << endl << error.historyAsString();
   * \endcode
   *
   * \todo That's a draft to have a common way of throwing exceptions.
   * Most probabely we'll finally use blocxx exceptions. Here, but not
   * in the remaining code of zypp. If we can we should try to wrap
   * the blocxx macros and typedef the classes in here.
   **/
  class Exception : public std::exception
  {
    friend std::ostream & operator<<( std::ostream & str, const Exception & obj );

  public:
    typedef exception_detail::CodeLocation CodeLocation;
    typedef std::list<std::string>         History;
    typedef History::const_iterator        HistoryIterator;
    typedef History::size_type             HistorySize;

    /** Default ctor.
     * Use \ref ZYPP_THROW to throw exceptions.
    */
    Exception();

    /** Ctor taking a message.
     * Use \ref ZYPP_THROW to throw exceptions.
    */
    Exception( const std::string & msg_r );

    /** Ctor taking a message and an exception to remember as history
     * \see \ref remember
     * Use \ref ZYPP_THROW to throw exceptions.
    */
    Exception( const std::string & msg_r, const Exception & history_r );

    /** Dtor. */
    virtual ~Exception() throw();

    /** Return CodeLocation. */
    const CodeLocation & where() const
    { return _where; }

    /** Exchange location on rethrow. */
    void relocate( const CodeLocation & where_r ) const
    { _where = where_r; }

    /** Return the message string provided to the ctor.
     * \note This is not necessarily the complete error message.
     * The whole error message is provided by \ref asString or
     * \ref dumpOn.
    */
    const std::string & msg() const
    { return _msg; }

    /** Error message provided by \ref dumpOn as string. */
    std::string asString() const;

    /** Translated error message as string suitable for the user.
     * \see \ref asUserStringHistory
    */
    std::string asUserString() const;

  public:
    /** \name History list of message strings.
     * Maintain a simple list of individual error messages, that lead
     * to this Exception. The Exceptions message itself is not included
     * in the history. The History list stores the most recent message
     * fist.
     */
    //@{

    /** Store an other Exception as history. */
    void remember( const Exception & old_r );

    /** Add some message text to the history. */
    void addHistory( const std::string & msg_r );

    /** Iterator pointing to the most recent message. */
    HistoryIterator historyBegin() const
    { return _history.begin(); }

    /** Iterator pointing behind the last message. */
    HistoryIterator historyEnd() const
    { return _history.end(); }

    /** Whether the history list is empty. */
    bool historyEmpty() const
    { return _history.empty(); }

    /** The size of the history list. */
    HistorySize historySize() const
    { return _history.size(); }

    /** The history as string. Empty if \ref historyEmpty.
     * Otherwise:
     * \code
     * History:
     *  - most recent message
     *  - 2nd message
     * ...
     *  - oldest message
     * \endcode
    */
    std::string historyAsString() const;

    /** A single (multiline) string composed of \ref asUserString  and  \ref historyAsString. */
    std::string asUserHistory() const;
    //@}

  protected:

    /** Overload this to print a proper error message. */
    virtual std::ostream & dumpOn( std::ostream & str ) const;

  public:
     /** Make a string from \a errno_r. */
    static std::string strErrno( int errno_r );
     /** Make a string from \a errno_r and \a msg_r. */
    static std::string strErrno( int errno_r, const std::string & msg_r );

  public:
    /** Drop a logline on throw, catch or rethrow.
     * Used by \ref ZYPP_THROW macros.
    */
    static void log( const Exception & excpt_r, const CodeLocation & where_r,
                     const char *const prefix_r );

  private:
    mutable CodeLocation _where;
    std::string          _msg;
    History              _history;

    /** Return message string. */
    virtual const char * what() const throw()
    { return _msg.c_str(); }

    /** Called by <tt>std::ostream & operator\<\<</tt>.
     * Prints \ref CodeLocation and the error message provided by
     * \ref dumpOn.
    */
    std::ostream & dumpError( std::ostream & str ) const;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Exception Stream output */
  std::ostream & operator<<( std::ostream & str, const Exception & obj );

  ///////////////////////////////////////////////////////////////////

  /** Helper for \ref ZYPP_THROW. */
  template<class TExcpt>
    void _ZYPP_THROW( const TExcpt & excpt_r, const exception_detail::CodeLocation & where_r ) __attribute__((noreturn));
  template<class TExcpt>
    void _ZYPP_THROW( const TExcpt & excpt_r, const exception_detail::CodeLocation & where_r )
    {
      excpt_r.relocate( where_r );
      Exception::log( excpt_r, where_r, "THROW:   " );
      throw( excpt_r );
    }

  /** Helper for \ref ZYPP_THROW. */
  template<class TExcpt>
    void _ZYPP_CAUGHT( const TExcpt & excpt_r, const exception_detail::CodeLocation & where_r )
    {
      Exception::log( excpt_r, where_r, "CAUGHT:  " );
    }

  /** Helper for \ref ZYPP_THROW. */
  template<class TExcpt>
    void _ZYPP_RETHROW( const TExcpt & excpt_r, const exception_detail::CodeLocation & where_r ) __attribute__((noreturn));
  template<class TExcpt>
    void _ZYPP_RETHROW( const TExcpt & excpt_r, const exception_detail::CodeLocation & where_r )
    {
      Exception::log( excpt_r, where_r, "RETHROW: " );
      excpt_r.relocate( where_r );
      throw;
    }

  ///////////////////////////////////////////////////////////////////

  /** \defgroup ZYPP_THROW ZYPP_THROW macros
   * Macros for throwing Exception.
   * \see \ref zypp::Exception for an example.
  */
  //@{
  /** Drops a logline and throws the Exception. */
#define ZYPP_THROW(EXCPT)\
  _ZYPP_THROW( EXCPT, ZYPP_EX_CODELOCATION )

  /** Drops a logline telling the Exception was caught (in order to handle it). */
#define ZYPP_CAUGHT(EXCPT)\
  _ZYPP_CAUGHT( EXCPT, ZYPP_EX_CODELOCATION )

  /** Drops a logline and rethrows, updating the CodeLocation. */
#define ZYPP_RETHROW(EXCPT)\
  _ZYPP_RETHROW( EXCPT, ZYPP_EX_CODELOCATION )


  /** Throw Exception built from a message string. */
#define ZYPP_THROW_MSG(EXCPTTYPE, MSG)\
  ZYPP_THROW( EXCPTTYPE( MSG ) )

  /** Throw Exception built from errno. */
#define ZYPP_THROW_ERRNO(EXCPTTYPE)\
  ZYPP_THROW( EXCPTTYPE( ::zypp::Exception::strErrno(errno) ) )

  /** Throw Exception built from errno provided as argument. */
#define ZYPP_THROW_ERRNO1(EXCPTTYPE, ERRNO)\
  ZYPP_THROW( EXCPTTYPE( ::zypp::Exception::strErrno(ERRNO) ) )

  /** Throw Exception built from errno and a message string. */
#define ZYPP_THROW_ERRNO_MSG(EXCPTTYPE, MSG)\
  ZYPP_THROW( EXCPTTYPE( ::zypp::Exception::strErrno(errno,MSG) ) )

  /** Throw Exception built from errno provided as argument and a message string */
#define ZYPP_THROW_ERRNO_MSG1(EXCPTTYPE, ERRNO,MSG)\
  ZYPP_THROW( EXCPTTYPE( ::zypp::Exception::strErrno(ERRNO,MSG) ) )
  //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_EXCEPTION_H
