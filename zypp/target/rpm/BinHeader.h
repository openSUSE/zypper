/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/BinHeader.h
 *
*/
#ifndef ZYPP_TARGET_RPM_BINHEADER_H
#define ZYPP_TARGET_RPM_BINHEADER_H

extern "C"
{
#include <stdint.h>
}

#include <iosfwd>
#include <string>
#include <vector>
#include <list>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/target/rpm/librpm.h"

namespace zypp
{
namespace target
{
namespace rpm
{
///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : BinHeader
/**
 *
 **/
class BinHeader : public base::ReferenceCounted, private base::NonCopyable
{

public:

  typedef intrusive_ptr<BinHeader> Ptr;

  typedef intrusive_ptr<const BinHeader> constPtr;

  typedef rpmTag tag;

  class intList;

  class stringList;

private:

  Header _h;

  bool assertHeader();

public:

  BinHeader( Header h_r = 0 );

  /**
   * <B>Dangerous!<\B> This one takes the header out of rhs
   * and leaves rhs empty.
   **/
  BinHeader( BinHeader::Ptr & rhs );

  virtual ~BinHeader();

public:

  bool empty() const
  {
    return( _h == NULL );
  }

  bool has_tag( tag tag_r ) const;

  unsigned int_list( tag tag_r, intList & lst_r ) const;

  unsigned string_list( tag tag_r, stringList & lst_r ) const;

  int int_val( tag tag_r ) const;

  std::string string_val( tag tag_r ) const;

public:

  std::list<std::string> stringList_val( tag tag_r ) const;

public:

  virtual std::ostream & dumpOn( std::ostream & str ) const;
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : BinHeader::intList
/**
 *
 **/
class BinHeader::intList : private base::NonCopyable
{
  public:
    intList()
      : _type( RPM_NULL_TYPE )
    {}

    bool empty() const
    { return _data.empty(); }

    unsigned size() const
    { return _data.size(); }

    long operator[]( const unsigned idx_r ) const
    { return idx_r < _data.size() ? _data[idx_r] : 0; }

  private:
    friend class BinHeader;
    unsigned set( void * val_r, unsigned cnt_r, rpmTagType type_r );

  private:
    std::vector<long> _data;
    rpmTagType _type;
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : BinHeader::stringList
/**
 *
 **/
class BinHeader::stringList : private base::NonCopyable
{
  public:
    bool empty() const
    { return _data.empty(); }

    unsigned size() const
    { return _data.size(); }

    std::string operator[]( const unsigned idx_r ) const
    { return idx_r < _data.size() ? _data[idx_r] : std::string(); }

  private:
    friend class BinHeader;
    unsigned set( char ** val_r, unsigned cnt_r );

  private:
    std::vector<std::string> _data;
};

///////////////////////////////////////////////////////////////////

} // namespace rpm
} // namespace target
} // namespace zypp

#endif // ZYPP_TARGET_RPM_BINHEADER_H
