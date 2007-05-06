/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/RpmException.h
 *
*/
#ifndef ZYPP_TARGET_RPM_RPMEXCEPTION_H
#define ZYPP_TARGET_RPM_RPMEXCEPTION_H

#include <iosfwd>

#include <string>

#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"
#include "zypp/Url.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace target
{
///////////////////////////////////////////////////////////////
namespace rpm
{
///////////////////////////////////////////////////////////////
//
//        CLASS NAME : RpmException
/** Just inherits Exception to separate media exceptions
 *
 **/
class RpmException : public Exception
{
public:
  /** Ctor taking message.
   * Use \ref ZYPP_THROW to throw exceptions.
  */
  RpmException()
      : Exception( "Rpm Exception" )
  {}
  /** Ctor taking message.
   * Use \ref ZYPP_THROW to throw exceptions.
  */
  RpmException( const std::string & msg_r )
      : Exception( msg_r )
  {}
  /** Dtor. */
  virtual ~RpmException() throw()
  {};
};

class GlobalRpmInitException : public RpmException
{
public:
  /** Ctor taking message.
   * Use \ref ZYPP_THROW to throw exceptions.
  */
  GlobalRpmInitException()
      : RpmException("Global RPM initialization failed")
  {}
  /** Dtor. */
  virtual ~GlobalRpmInitException() throw()
  {};
private:
};

class RpmInvalidRootException : public RpmException
{
public:
  /** Ctor taking message.
   * Use \ref ZYPP_THROW to throw exceptions.
  */
  RpmInvalidRootException( const Pathname & root_r,
                           const Pathname & dbpath_r )
      : RpmException()
      , _root(root_r.asString())
      , _dbpath(dbpath_r.asString())
  {}
  /** Dtor. */
  virtual ~RpmInvalidRootException() throw()
  {};
  std::string root() const
  {
    return _root;
  }
  std::string dbpath() const
  {
    return _dbpath;
  }
protected:
  virtual std::ostream & dumpOn( std::ostream & str ) const;
private:
  std::string _root;
  std::string _dbpath;
};

class RpmAccessBlockedException : public RpmException
{
public:
  RpmAccessBlockedException( const Pathname & root_r,
                             const Pathname & dbpath_r )
      : RpmException()
      , _root(root_r.asString())
      , _dbpath(dbpath_r.asString())
  {}
  virtual ~RpmAccessBlockedException() throw()
  {};
  std::string root() const
  {
    return _root;
  }
  std::string dbpath() const
  {
    return _dbpath;
  }
protected:
  virtual std::ostream & dumpOn( std::ostream & str ) const;
private:
  std::string _root;
  std::string _dbpath;
};

class RpmSubprocessException : public RpmException
{
public:
  RpmSubprocessException(const std::string & errmsg_r)
      : RpmException()
      , _errmsg(errmsg_r)
  {}
  virtual ~RpmSubprocessException() throw()
  {};
protected:
  virtual std::ostream & dumpOn( std::ostream & str ) const;
private:
  std::string _errmsg;
};

class RpmInitException : public RpmException
{
public:
  RpmInitException(const Pathname & root_r,
                   const Pathname & dbpath_r)
      : RpmException()
      , _root(root_r.asString())
      , _dbpath(dbpath_r.asString())
  {}
  virtual ~RpmInitException() throw()
  {};
protected:
  virtual std::ostream & dumpOn( std::ostream & str ) const;
private:
  std::string _root;
  std::string _dbpath;
};

class RpmDbOpenException : public RpmException
{
public:
  RpmDbOpenException(const Pathname & root_r,
                     const Pathname & dbpath_r)
      : RpmException()
      , _root(root_r.asString())
      , _dbpath(dbpath_r.asString())
  {}
  virtual ~RpmDbOpenException() throw()
  {};
protected:
  virtual std::ostream & dumpOn( std::ostream & str ) const;
private:
  std::string _root;
  std::string _dbpath;
};

class RpmDbAlreadyOpenException : public RpmException
{
public:
  RpmDbAlreadyOpenException(const Pathname & old_root_r,
                            const Pathname & old_dbpath_r,
                            const Pathname & new_root_r,
                            const Pathname & new_dbpath_r)
      : RpmException()
      , _old_root(old_root_r.asString())
      , _old_dbpath(old_dbpath_r.asString())
      , _new_root(new_root_r.asString())
      , _new_dbpath(new_dbpath_r.asString())
  {}
  virtual ~RpmDbAlreadyOpenException() throw()
  {};
protected:
  virtual std::ostream & dumpOn( std::ostream & str ) const;
private:
  std::string _old_root;
  std::string _old_dbpath;
  std::string _new_root;
  std::string _new_dbpath;
};

class RpmDbNotOpenException : public RpmException
{
public:
  RpmDbNotOpenException()
      : RpmException()
  {}
  virtual ~RpmDbNotOpenException() throw()
  {};
protected:
  virtual std::ostream & dumpOn( std::ostream & str ) const;
private:
};

class RpmDbConvertException : public RpmException
{
public:
  RpmDbConvertException()
      : RpmException()
  {}
  virtual ~RpmDbConvertException() throw()
  {};
protected:
  virtual std::ostream & dumpOn( std::ostream & str ) const;
private:
};

class RpmNullDatabaseException : public RpmException
{
public:
  RpmNullDatabaseException()
      : RpmException()
  {}
  virtual ~RpmNullDatabaseException() throw()
  {};
protected:
  virtual std::ostream & dumpOn( std::ostream & str ) const;
private:
};



/////////////////////////////////////////////////////////////////
} // namespace rpm
} // namespace target
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_RPM_RPMEXCEPTION_H
