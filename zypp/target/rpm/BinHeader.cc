/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/rpm/BinHeader.cc
 *
*/
#include "librpm.h"

#include <iostream>

#include "zypp/base/Logger.h"

#include "zypp/target/rpm/BinHeader.h"

using namespace std;

#undef Y2LOG
#define Y2LOG "BinHeader"

namespace zypp
{
namespace target
{
namespace rpm
{

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : BinHeader::intList
//
///////////////////////////////////////////////////////////////////

BinHeader::intList::intList()
    : cnt( 0 ), val( 0 ), type( RPM_NULL_TYPE )
{}

unsigned BinHeader::intList::set( void * val_r, unsigned cnt_r, rpmTagType type_r )
{
  val = val_r;
  cnt = val ? cnt_r : 0;
  type = type_r;
  return cnt;
}

int BinHeader::intList::operator[]( const unsigned idx_r ) const
{
  if ( idx_r < cnt )
  {
    switch ( type )
    {
    case RPM_CHAR_TYPE:
      return ((char*)val)[idx_r];
    case RPM_INT8_TYPE:
      return ((int8_t*)val)[idx_r];
    case RPM_INT16_TYPE:
      return ((int16_t*)val)[idx_r];
    case RPM_INT32_TYPE:
      return ((int32_t*)val)[idx_r];
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : BinHeader::stringList
//
///////////////////////////////////////////////////////////////////

void BinHeader::stringList::clear()
{
  if ( val )
    free( val );
  val = 0;
  cnt = 0;
}

BinHeader::stringList::stringList()
    : cnt( 0 ), val( 0 )
{}

unsigned BinHeader::stringList::set( char ** val_r, unsigned cnt_r )
{
  clear();
  val = val_r;
  cnt = val ? cnt_r : 0;
  return cnt;
}

std::string BinHeader::stringList::operator[]( const unsigned idx_r ) const
{
  return( idx_r < cnt ? val[idx_r] : "" );
}

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : BinHeader
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : BinHeader::BinHeader
//        METHOD TYPE : Constructor
//
BinHeader::BinHeader( Header h_r )
    : _h( h_r )
{
  if ( _h )
  {
    ::headerLink( _h );
  }
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : BinHeader::BinHeader
//        METHOD TYPE : Constructor
//
BinHeader::BinHeader( BinHeader::Ptr & rhs )
{
  INT << "INJECT from " << rhs;
  if ( ! (rhs && rhs->_h) )
  {
    _h = 0;
  }
  else
  {
    _h = rhs->_h;  // ::headerLink already done in rhs
    rhs->_h = 0;
  }
  INT << ": " << *this << "   (" << rhs << ")" << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : BinHeader::~BinHeader
//        METHOD TYPE : Destructor
//
BinHeader::~BinHeader()
{
  if ( _h )
  {
    ::headerFree( _h );
  }
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : BinHeader::assertHeader
//        METHOD TYPE : void
//
bool BinHeader::assertHeader()
{
  if ( !_h )
  {
    _h = ::headerNew();
    if ( !_h )
    {
      INT << "OOPS: NULL HEADER created!" << endl;
      return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : BinHeader::has_tag
//        METHOD TYPE : bool
//
//        DESCRIPTION :
//
bool BinHeader::has_tag( tag tag_r ) const
{
  return( !empty() && ::headerIsEntry( _h, tag_r ) );
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : BinHeader::int_list
//        METHOD TYPE : unsigned
//
//        DESCRIPTION :
//
unsigned BinHeader::int_list( tag tag_r, intList & lst_r ) const
{
  if ( !empty() )
  {
    rpmTagType type = RPM_NULL_TYPE;
    rpm_count_t cnt = 0;
    void * val  = 0;
    ::headerGetEntry( _h, tag_r, hTYP_t(&type), &val, &cnt );

    if ( val )
    {
      switch ( type )
      {
      case RPM_NULL_TYPE:
        return lst_r.set( 0, 0, type );
      case RPM_CHAR_TYPE:
      case RPM_INT8_TYPE:
      case RPM_INT16_TYPE:
      case RPM_INT32_TYPE:
        return lst_r.set( val, cnt, type );

      case RPM_STRING_ARRAY_TYPE:
        free( val );
        // fall through
      default:
        INT << "RPM_TAG MISSMATCH: RPM_INT32_TYPE " << tag_r << " got type " << type << endl;
      }
    }
  }
  return lst_r.set( 0, 0, RPM_NULL_TYPE );
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : BinHeader::string_list
//        METHOD TYPE : unsigned
//
//        DESCRIPTION :
//
unsigned BinHeader::string_list( tag tag_r, stringList & lst_r ) const
{
  if ( !empty() )
  {
    rpmTagType type = RPM_NULL_TYPE;
    rpm_count_t cnt = 0;
    void * val  = 0;
    ::headerGetEntry( _h, tag_r, hTYP_t(&type), &val, &cnt );

    if ( val )
    {
      switch ( type )
      {
      case RPM_NULL_TYPE:
        return lst_r.set( 0, 0 );
      case RPM_STRING_ARRAY_TYPE:
        return lst_r.set( (char**)val, cnt );

      default:
        INT << "RPM_TAG MISSMATCH: RPM_STRING_ARRAY_TYPE " << tag_r << " got type " << type << endl;
      }
    }
  }
  return lst_r.set( 0, 0 );
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : BinHeader::int_val
//        METHOD TYPE : int
//
//        DESCRIPTION :
//
int BinHeader::int_val( tag tag_r ) const
{
  if ( !empty() )
  {
    rpmTagType type = RPM_NULL_TYPE;
    rpm_count_t cnt = 0;
    void * val  = 0;
    ::headerGetEntry( _h, tag_r, hTYP_t(&type), &val, &cnt );

    if ( val )
    {
      switch ( type )
      {
      case RPM_NULL_TYPE:
        return 0;
      case RPM_CHAR_TYPE:
        return *((char*)val);
      case RPM_INT8_TYPE:
        return *((int8_t*)val);
      case RPM_INT16_TYPE:
        return *((int16_t*)val);
      case RPM_INT32_TYPE:
        return *((int32_t*)val);

      case RPM_STRING_ARRAY_TYPE:
        free( val );
        // fall through
      default:
        INT << "RPM_TAG MISSMATCH: RPM_INT32_TYPE " << tag_r << " got type " << type << endl;
      }
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : BinHeader::string_val
//        METHOD TYPE : std::string
//
//        DESCRIPTION :
//
std::string BinHeader::string_val( tag tag_r ) const
{
  if ( !empty() )
  {
    rpmTagType type = RPM_NULL_TYPE;
    rpm_count_t cnt = 0;
    void * val  = 0;
    ::headerGetEntry( _h, tag_r, hTYP_t(&type), &val, &cnt );

    if ( val )
    {
      switch ( type )
      {
      case RPM_NULL_TYPE:
        return "";
      case RPM_STRING_TYPE:
        return (char*)val;

      case RPM_STRING_ARRAY_TYPE:
        free( val );
        // fall through
      default:
        INT << "RPM_TAG MISSMATCH: RPM_STRING_TYPE " << tag_r << " got type " << type << endl;
      }
    }
  }
  return "";
}

///////////////////////////////////////////////////////////////////
//
//
//        METHOD NAME : BinHeader::stringList_val
//        METHOD TYPE : std::list<std::string>
//
//        DESCRIPTION :
//
std::list<std::string> BinHeader::stringList_val( tag tag_r ) const
{
  std::list<std::string> ret;

  if ( !empty() )
  {
    stringList lines;
    unsigned count = string_list( tag_r, lines );
    for ( unsigned i = 0; i < count; ++i )
    {
      ret.push_back( lines[i] );
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////
//
//
//      METHOD NAME : BinHeader::dumpOn
//      METHOD TYPE : ostream &
//
//      DESCRIPTION :
//
ostream & BinHeader::dumpOn( ostream & str ) const
{
  ReferenceCounted::dumpOn( str );
  return str << '{' << (void*)_h << '}';
}

} // namespace rpm
} // namespace target
} // namespace zypp
