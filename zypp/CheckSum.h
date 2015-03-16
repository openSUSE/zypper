/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CheckSum.h
 *
*/
#ifndef ZYPP_CHECKSUM_H
#define ZYPP_CHECKSUM_H

#include <iosfwd>
#include <string>
#include <sstream>

#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  struct CheckSumException : public Exception
  {
    CheckSumException( const std::string & msg )
      : Exception( msg )
    {}
  };

  class CheckSum
  {
  public:
    /** Default Ctor: empty checksum. */
    CheckSum()
    {}
    /**
     * Creates a checksum for algorithm \param type.
     * \throws CheckSumException if the checksum is invalid and can't be constructed
     */
    CheckSum( const std::string & type, const std::string & checksum );
    /**
     * Creates a checksum auto probing the algorithm type.
     * \throws CheckSumException if the checksum is invalid and can't be constructed
     */
    CheckSum( const std::string & checksum )
      : CheckSum( std::string(), checksum )
    {}

    /**
     * Reads the content of \param input_r and computes the checksum.
     */
    CheckSum( const std::string & type, std::istream & input_r );

#ifndef SWIG // Swig treats it as syntax error0
    /** Ctor from temporary istream */
    CheckSum( const std::string & type, std::istream && input_r )
      : CheckSum( type, input_r )
    {}
#endif

  public:
    static const std::string & md5Type();
    static const std::string & shaType();
    static const std::string & sha1Type();
    static const std::string & sha224Type();
    static const std::string & sha256Type();
    static const std::string & sha384Type();
    static const std::string & sha512Type();

    /** \name Creates a checksum for algorithm \param type. */
    //@{
    static CheckSum md5( const std::string & checksum )		{ return  CheckSum( md5Type(), checksum); }
    static CheckSum sha( const std::string & checksum )		{ return  CheckSum( shaType(), checksum); }
    static CheckSum sha1( const std::string & checksum )	{ return  CheckSum( sha1Type(), checksum); }
    static CheckSum sha224( const std::string & checksum )	{ return  CheckSum( sha224Type(), checksum); }
    static CheckSum sha256( const std::string & checksum )	{ return  CheckSum( sha256Type(), checksum); }
    static CheckSum sha384( const std::string & checksum )	{ return  CheckSum( sha384Type(), checksum); }
    static CheckSum sha512( const std::string & checksum )	{ return  CheckSum( sha512Type(), checksum); }
    //@}

    /** \name Reads the content of \param input_r and computes the checksum. */
    //@{
    static CheckSum md5( std::istream & input_r )		{ return  CheckSum( md5Type(), input_r ); }
    static CheckSum sha( std::istream & input_r )		{ return  CheckSum( sha1Type(), input_r ); }
    static CheckSum sha1( std::istream & input_r )		{ return  CheckSum( sha1Type(), input_r ); }
    static CheckSum sha224( std::istream & input_r )		{ return  CheckSum( sha224Type(), input_r ); }
    static CheckSum sha256( std::istream & input_r )		{ return  CheckSum( sha256Type(), input_r ); }
    static CheckSum sha384( std::istream & input_r )		{ return  CheckSum( sha384Type(), input_r ); }
    static CheckSum sha512( std::istream & input_r )		{ return  CheckSum( sha512Type(), input_r ); }
#ifndef SWIG // Swig treats it as syntax error
    static CheckSum md5( std::istream && input_r )		{ return  CheckSum( md5Type(), input_r ); }
    static CheckSum sha( std::istream && input_r )		{ return  CheckSum( sha1Type(), input_r ); }
    static CheckSum sha1( std::istream && input_r )		{ return  CheckSum( sha1Type(), input_r ); }
    static CheckSum sha224( std::istream && input_r )		{ return  CheckSum( sha224Type(), input_r ); }
    static CheckSum sha256( std::istream && input_r )		{ return  CheckSum( sha256Type(), input_r ); }
    static CheckSum sha384( std::istream && input_r )		{ return  CheckSum( sha384Type(), input_r ); }
    static CheckSum sha512( std::istream && input_r )		{ return  CheckSum( sha512Type(), input_r ); }
#endif
    //@}

    /** \name Reads the content of \param input_r and computes the checksum. */
    //@{
    static CheckSum md5FromString( const std::string & input_r )	{ return md5( std::stringstream( input_r ) ); }
    static CheckSum shaFromString( const std::string & input_r )	{ return sha( std::stringstream( input_r ) ); }
    static CheckSum sha1FromString( const std::string & input_r )	{ return sha1( std::stringstream( input_r ) ); }
    static CheckSum sha224FromString( const std::string & input_r )	{ return sha224( std::stringstream( input_r ) ); }
    static CheckSum sha256FromString( const std::string & input_r )	{ return sha256( std::stringstream( input_r ) ); }
    static CheckSum sha384FromString( const std::string & input_r )	{ return sha384( std::stringstream( input_r ) ); }
    static CheckSum sha512FromString( const std::string & input_r )	{ return sha512( std::stringstream( input_r ) ); }
    //@}

  public:
    std::string type() const;
    std::string checksum() const;
    bool empty() const;

  public:
    std::string asString() const;

  private:
    std::string _type;
    std::string _checksum;
  };

  /** \relates CheckSum Stream output. */
  std::ostream & operator<<( std::ostream & str, const CheckSum & obj );

  /** \relates CheckSum XML output. */
  std::ostream & dumpAsXmlOn( std::ostream & str, const CheckSum & obj );

  /** \relates CheckSum */
  bool operator==( const CheckSum & lhs, const CheckSum & rhs );

  /** \relates CheckSum */
  bool operator!=( const CheckSum & lhs, const CheckSum & rhs );

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CHECKSUM_H
