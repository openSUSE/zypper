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
extern "C"
{
#ifdef _RPM_5
#undef RPM_NULL_TYPE
#define RPM_NULL_TYPE rpmTagType(0)
typedef rpmuint32_t rpm_count_t;
#else
#ifdef _RPM_4_4
typedef int32_t rpm_count_t;
#endif
#endif
}

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/NonCopyable.h"

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

  /** Helper for header data retieval.
   * With \c _RPM_4_X use \c ::headerGet; with older \c _RPM_4_4
   * use the meanwhile deprecated \c ::headerGetEntry.
   * \ingroup g_RAII
   */
  struct HeaderEntryGetter : private base::NonCopyable
  {
    public:
      HeaderEntryGetter( const Header & h_r, rpmTag & tag_r );
      ~HeaderEntryGetter();
      rpmTagType  type();
      rpm_count_t cnt();
      void *      val();
    private:
#ifdef _RPM_4_X
     ::rpmtd		_rpmtd;
#else
      rpmTagType	_type;
      rpm_count_t	_cnt;
      void *		_val;
#endif //_RPM_4_X
 };

#ifdef _RPM_4_X
  inline HeaderEntryGetter::HeaderEntryGetter( const Header & h_r, rpmTag & tag_r )
    : _rpmtd( ::rpmtdNew() )
  { ::headerGet( h_r, tag_r, _rpmtd, HEADERGET_DEFAULT ); }
  inline HeaderEntryGetter::~HeaderEntryGetter()
  { ::rpmtdFreeData( _rpmtd ); ::rpmtdFree( _rpmtd ); }
  inline rpmTagType	HeaderEntryGetter::type()	{ return rpmtdType( _rpmtd ); }
  inline rpm_count_t	HeaderEntryGetter::cnt()	{ return _rpmtd->count; }
  inline void *		HeaderEntryGetter::val()	{ return _rpmtd->data; }
#else
  inline HeaderEntryGetter::HeaderEntryGetter( const Header & h_r, rpmTag & tag_r )
    : _type( RPM_NULL_TYPE )
    , _cnt( 0 )
    , _val( 0 )
  { ::headerGetEntry( h_r, tag_r, hTYP_t(&_type), &_val, &_cnt ); }
  inline HeaderEntryGetter::~HeaderEntryGetter()
  { if ( _val && _type == RPM_STRING_ARRAY_TYPE ) free( _val ); }
  inline rpmTagType	HeaderEntryGetter::type()	{ return _type; }
  inline rpm_count_t	HeaderEntryGetter::cnt()	{ return _cnt; }
  inline void *		HeaderEntryGetter::val()	{ return _val; }
#endif //_RPM_4_X

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : BinHeader::intList
//
///////////////////////////////////////////////////////////////////

unsigned BinHeader::intList::set( void * val_r, unsigned cnt_r, rpmTagType type_r )
{
  if ( val_r )
    switch ( _type )
    {
#if RPM_CHAR_TYPE != RPM_INT8_TYPE
      case RPM_CHAR_TYPE:
	std::vector<long>( (char*)val_r, ((char*)val_r)+cnt_r ).swap( _data );
	break;
#endif
      case RPM_INT8_TYPE:
	std::vector<long>( (int8_t*)val_r, ((int8_t*)val_r)+cnt_r ).swap( _data );
	break;
      case RPM_INT16_TYPE:
	std::vector<long>( (int16_t*)val_r, ((int16_t*)val_r)+cnt_r ).swap( _data );
	break;
      case RPM_INT32_TYPE:
	std::vector<long>( (int32_t*)val_r, ((int32_t*)val_r)+cnt_r ).swap( _data );
	break;
	#ifdef _RPM_4_X
      case RPM_INT64_TYPE:
	std::vector<long>( (int64_t*)val_r, ((int64_t*)val_r)+cnt_r ).swap( _data );
	break;
	#endif
      default:
	std::vector<long>( cnt_r, 0L ).swap( _data );
	break;
    }
  else
    _data.clear();
  return _data.size();
}

///////////////////////////////////////////////////////////////////
//
//        CLASS NAME : BinHeader::stringList
//
///////////////////////////////////////////////////////////////////

unsigned BinHeader::stringList::set( char ** val_r, unsigned cnt_r )
{
  if ( val_r )
    std::vector<std::string>( val_r, val_r+cnt_r ).swap( _data );
  else
    _data.clear();
  return _data.size();
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
    headerLink( _h );
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
    headerFree( _h );
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
    HeaderEntryGetter headerget( _h, tag_r );

    if ( headerget.val() )
    {
      switch ( headerget.type() )
      {
      case RPM_NULL_TYPE:
        return lst_r.set( 0, 0, headerget.type() );
#if RPM_CHAR_TYPE != RPM_INT8_TYPE
      case RPM_CHAR_TYPE:
#endif
      case RPM_INT8_TYPE:
      case RPM_INT16_TYPE:
      case RPM_INT32_TYPE:
        return lst_r.set( headerget.val(), headerget.cnt(), headerget.type() );

      default:
        INT << "RPM_TAG MISSMATCH: RPM_INT32_TYPE " << tag_r << " got type " << headerget.type() << endl;
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
    HeaderEntryGetter headerget( _h, tag_r );

    if ( headerget.val() )
    {
      switch ( headerget.type() )
      {
      case RPM_NULL_TYPE:
        return lst_r.set( 0, 0 );
      case RPM_STRING_ARRAY_TYPE:
        return lst_r.set( (char**)headerget.val(), headerget.cnt() );

      default:
        INT << "RPM_TAG MISSMATCH: RPM_STRING_ARRAY_TYPE " << tag_r << " got type " << headerget.type() << endl;
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
    HeaderEntryGetter headerget( _h, tag_r );

    if ( headerget.val() )
    {
      switch ( headerget.type() )
      {
      case RPM_NULL_TYPE:
        return 0;
#if RPM_CHAR_TYPE != RPM_INT8_TYPE
      case RPM_CHAR_TYPE:
        return *((char*)headerget.val());
#endif
      case RPM_INT8_TYPE:
        return *((int8_t*)headerget.val());
      case RPM_INT16_TYPE:
        return *((int16_t*)headerget.val());
      case RPM_INT32_TYPE:
        return *((int32_t*)headerget.val());

      default:
        INT << "RPM_TAG MISSMATCH: RPM_INT32_TYPE " << tag_r << " got type " << headerget.type() << endl;
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
    HeaderEntryGetter headerget( _h, tag_r );

    if ( headerget.val() )
    {
      switch ( headerget.type() )
      {
      case RPM_NULL_TYPE:
        return "";
      case RPM_STRING_TYPE:
        return (char*)headerget.val();

     default:
        INT << "RPM_TAG MISSMATCH: RPM_STRING_TYPE " << tag_r << " got type " << headerget.type() << endl;
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
