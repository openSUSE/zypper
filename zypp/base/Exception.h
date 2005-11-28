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
  namespace exceptinon_detail
  { /////////////////////////////////////////////////////////////////

    /** Keep _FILE_, _FUNCTION_ and _LINE_.
     * Construct it using the \ref ZYPP_EX_CODELOCATION macro.
    */
    struct CodeLocation
    {
      friend std::ostream & operator<<( std::ostream & str, const CodeLocation & obj );

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
    #define ZYPP_EX_CODELOCATION ::zypp::exceptinon_detail::CodeLocation(__FILE__,__FUNCTION__,__LINE__)

    /** \relates CodeLocation Stream output */
    std::ostream & operator<<( std::ostream & str, const CodeLocation & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace exceptinon_detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Exception
  /** Exception stores message and \ref CodeLocation.
   *
   * Use \ref ZYPP_THROW to throw exceptions. Assuming this,
   * the ctor logs it's creation, as it't the point where the
   * exception is thrown. If you catch the Excetion, you may
   * call \ref caught, to drop a logling telling so.
   *
   * \todo That's a draft to have a common way of throwing exceptions.
   * Most probabely we'll finally use blocxx exceptions. Here, but not
   * in the remaining code of zypp. If we can we should try to wrap
   * the blocxx macros and typedef the classes in here.
   **/
  class Exception : public std::exception
  {
    friend std::ostream & operator<<( std::ostream & str, const Exception & obj );
    typedef exceptinon_detail::CodeLocation CodeLocation;
  public:

    /** Ctor taking CodeLocation and message.
     * Use \ref ZYPP_THROW to throw exceptions.
    */
    Exception( const CodeLocation & where_r, const std::string & msg_r );

    /** Dtor. */
    virtual ~Exception() throw();

    /** Return CodeLocation. */
    const CodeLocation & where() const
    { return _where; }

    /** Return message string. */
    const std::string & msg() const
    { return _msg; }

    /** Return message string. */
    virtual const char * what() const throw()
    { return _msg.c_str(); }

    /** Exception as string */
    std::string asString() const;

    /** Simply drops a log line. */
    void caught() const;

  public:
     /** Make a string from \a errno_r. */
    static std::string strErrno( int errno_r );
     /** Make a string from \a errno_r and \a msg_r. */
    static std::string strErrno( int errno_r, const std::string & msg_r );

  private:
    CodeLocation _where;
    std::string _msg;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Exception Stream output */
  std::ostream & operator<<( std::ostream & str, const Exception & obj );

  ///////////////////////////////////////////////////////////////////

  /** Actually throw an Exception. */
#define ZYPP_DOTHROW(EXCPT) throw( EXCPT )

  /** \defgroup ZYPP_THROW ZYPP_THROW macros
   * Macros for throwing Exception.
  */
  //@{
  /** Throw a message string. */
#define ZYPP_THROW(MSG)\
  ZYPP_DOTHROW( ::zypp::Exception( ZYPP_EX_CODELOCATION, MSG ) )

  /** Throw errno. */
#define ZYPP_THROW_ERRNO\
  ZYPP_DOTHROW( ::zypp::Exception( ZYPP_EX_CODELOCATION, ::zypp::Exception::strErrno(errno) ) )

  /** Throw errno provided as argument. */
#define ZYPP_THROW_ERRNO1(ERRNO)\
  ZYPP_DOTHROW( ::zypp::Exception( ZYPP_EX_CODELOCATION, ::zypp::Exception::strErrno(ERRNO) ) )

  /** Throw errno and a message string. */
#define ZYPP_THROW_ERRNO_MSG(MSG)\
  ZYPP_DOTHROW( ::zypp::Exception( ZYPP_EX_CODELOCATION, ::zypp::Exception::strErrno(errno,MSG) ) )

  /** Throw errno provided as argument and a message string */
#define ZYPP_THROW_ERRNO_MSG1(ERRNO,MSG)\
  ZYPP_DOTHROW( ::zypp::Exception( ZYPP_EX_CODELOCATION, ::zypp::Exception::strErrno(ERRNO,MSG) ) )
  //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_EXCEPTION_H
