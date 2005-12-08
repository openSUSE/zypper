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
#include <stdexcept>

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
    #define ZYPP_EX_CODELOCATION ::zypp::exception_detail::CodeLocation(__FILE__,__FUNCTION__,__LINE__)

    /** \relates CodeLocation Stream output */
    std::ostream & operator<<( std::ostream & str, const CodeLocation & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace exception_detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Exception
  /** Exception stores message and \ref CodeLocation.
   *
   * Use \ref ZYPP_THROW to throw exceptions.
   * \code
   *  43   try
   *  44     {
   *  45       try
   *  46         {
   *  47           ZYPP_THROW( "Something bad happened." );
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
   * \todo That's a draft to have a common way of throwing exceptions.
   * Most probabely we'll finally use blocxx exceptions. Here, but not
   * in the remaining code of zypp. If we can we should try to wrap
   * the blocxx macros and typedef the classes in here.
   *
   * \todo maybe location and message stack.
   **/
  class Exception : public std::exception
  {
    friend std::ostream & operator<<( std::ostream & str, const Exception & obj );
  public:
    typedef exception_detail::CodeLocation CodeLocation;

    /** Ctor taking a message.
     * Use \ref ZYPP_THROW to throw exceptions.
    */
    Exception( const std::string & msg_r );

    /** Dtor. */
    virtual ~Exception() throw();

    /** Return CodeLocation. */
    const CodeLocation & where() const
    { return _where; }

    /** Exchange location on rethrow. */
    void relocate( const CodeLocation & where_r ) const
    { _where = where_r; }

    /** Return message string. */
    const std::string & msg() const
    { return _msg; }

    /** Return message string. */
    virtual const char * what() const throw()
    { return _msg.c_str(); }

    /** Exception as string */
    std::string asString() const;

  public:
     /** Make a string from \a errno_r. */
    static std::string strErrno( int errno_r );
     /** Make a string from \a errno_r and \a msg_r. */
    static std::string strErrno( int errno_r, const std::string & msg_r );

  public:
    /** Drop a logline. */
    static void log( const Exception & excpt_r, const CodeLocation & where_r,
                     const char *const prefix_r );

  private:
    mutable CodeLocation _where;
    std::string _msg;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Exception Stream output */
  std::ostream & operator<<( std::ostream & str, const Exception & obj );

  ///////////////////////////////////////////////////////////////////

  /** Helper for \ref ZYPP_THROW. */
  template<class _Excpt>
    void _ZYPP_THROW( const _Excpt & excpt_r, const exception_detail::CodeLocation & where_r )
    {
      excpt_r.relocate( where_r );
      Exception::log( excpt_r, where_r, "THROW:   " );
      throw( excpt_r );
    }

  /** Helper for \ref ZYPP_THROW. */
  template<class _Excpt>
    void _ZYPP_CAUGHT( const _Excpt & excpt_r, const exception_detail::CodeLocation & where_r )
    {
      Exception::log( excpt_r, where_r, "CAUGHT:  " );
    }

  /** Helper for \ref ZYPP_THROW. */
  template<class _Excpt>
    void _ZYPP_RETHROW( const _Excpt & excpt_r, const exception_detail::CodeLocation & where_r )
    {
      Exception::log( excpt_r, where_r, "RETHROW: " );
      excpt_r.relocate( where_r );
      throw excpt_r;
    }

  ///////////////////////////////////////////////////////////////////

  /** \defgroup ZYPP_THROW ZYPP_THROW macros
   * Macros for throwing Exception.
   * \see \ref zypp::Exception for an example.
  */
  //@{
  /** Drops a logline and throws the Exception. */
#define ZYPP_DOTHROW(EXCPT)\
  _ZYPP_THROW( EXCPT, ZYPP_EX_CODELOCATION )

  /** Drops a logline telling the Exception was caught (in order to handle it). */
#define ZYPP_CAUGHT(EXCPT)\
  _ZYPP_CAUGHT( EXCPT, ZYPP_EX_CODELOCATION )

  /** Drops a logline and rethrows, updating the CodeLocation. */
#define ZYPP_RETHROW(EXCPT)\
  _ZYPP_RETHROW( EXCPT, ZYPP_EX_CODELOCATION )


  /** Throw Exception built from a message string. */
#define ZYPP_THROW(EXCPTTYPE, MSG)\
  ZYPP_DOTHROW( EXCPTTYPE( MSG ) )

  /** Throw Exception built from errno. */
#define ZYPP_THROW_ERRNO(EXCPTTYPE)\
  ZYPP_DOTHROW( EXCPTTYPE( ::zypp::Exception::strErrno(errno) ) )

  /** Throw Exception built from errno provided as argument. */
#define ZYPP_THROW_ERRNO1(EXCPTTYPE, ERRNO)\
  ZYPP_DOTHROW( EXCPTTYPE( ::zypp::Exception::strErrno(ERRNO) ) )

  /** Throw Exception built from errno and a message string. */
#define ZYPP_THROW_ERRNO_MSG(EXCPTTYPE, MSG)\
  ZYPP_DOTHROW( EXCPTTYPE( ::zypp::Exception::strErrno(errno,MSG) ) )

  /** Throw Exception built from errno provided as argument and a message string */
#define ZYPP_THROW_ERRNO_MSG1(EXCPTTYPE, ERRNO,MSG)\
  ZYPP_DOTHROW( EXCPTTYPE( ::zypp::Exception::strErrno(ERRNO,MSG) ) )
  //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_EXCEPTION_H
