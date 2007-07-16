#include "Tools.h"
#include "ExplicitMap.h"
#include <boost/call_traits.hpp>

#include <iostream>
#include <fstream>
#include <map>

#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
#include "zypp/base/Exception.h"
#include "zypp/base/InputStream.h"
#include "zypp/base/DefaultIntegral.h"
#include <zypp/base/Function.h>
#include <zypp/base/Iterator.h>

#include <zypp/Pathname.h>
#include <zypp/Edition.h>
#include <zypp/CheckSum.h>
#include <zypp/Date.h>

#include "zypp/parser/TagParser.h"
#include "zypp/parser/susetags/PackagesFileReader.h"
#include "zypp/parser/susetags/PackagesLangFileReader.h"
#include "zypp/parser/susetags/PatternFileReader.h"
#include "zypp/parser/susetags/ContentFileReader.h"
#include "zypp/parser/susetags/RepoIndex.h"
#include "zypp/parser/susetags/RepoParser.h"
#include "zypp/cache/CacheStore.h"

using namespace std;
using namespace zypp;
using namespace zypp::parser;
using namespace zypp::parser::susetags;

///////////////////////////////////////////////////////////////////

struct DummyConsumer : public zypp::data::ResolvableDataConsumer
                     , private base::NonCopyable
{
  std::string idString( const data::ResObject_Ptr & res_r, ResolvableTraits::KindType kind_r )
  {
    std::string ret( kind_r.asString() );
    ret += ":";
    ret += res_r->name;
    ret += "-";
    ret += res_r->edition.asString();
    ret += ".";
    ret += res_r->arch.asString();
    return ret;
  }

  data::RecordId                       _lastId;
  std::map<std::string,data::RecordId> _idMap;
  std::map<data::RecordId,std::string> _reverseMap;

  bool hasEntry( const std::string & id_r )
  {
    return _idMap.find( id_r ) != _idMap.end();
  }

  data::RecordId newId( const data::ResObject_Ptr & res_r, ResolvableTraits::KindType kind_r )
  {
    std::string id( idString( res_r, kind_r ) );
    if ( hasEntry( id ) )
      ZYPP_THROW(Exception(id));
    _idMap[id] = ++_lastId;
    _reverseMap[_lastId] = id;
    MIL << "NEW_ID " << _lastId << " - " << id << endl;
    logNew( res_r, kind_r );
    return _lastId;
  }

  data::RecordId getId( const data::ResObject_Ptr & res_r, ResolvableTraits::KindType kind_r )
  {
    std::string id( idString( res_r, kind_r ) );
    if ( ! hasEntry( id ) )
      ZYPP_THROW(Exception(id));
    data::RecordId ret = _idMap[id];
    DBG << ret << " " << id << endl;
    return ret;
  }

  std::string lookup( data::RecordId id_r )
  {
    if ( id_r == data::noRecordId )
    {
      return "";
    }

    if ( _reverseMap.find( id_r ) != _reverseMap.end() )
    {
      return _reverseMap[id_r];
    }

    WAR << "Lookup id " << id_r << "failed" << endl;
    return std::string();
  }

  void logNew( const data::ResObject_Ptr & res_r, ResolvableTraits::KindType kind_r )
  {
    std::string shr( lookup( res_r->shareDataWith ) );
    if ( ! shr.empty() )
    {
      DBG << "  SHR: " << shr << endl;
    }
    //DBG << "  SUM: " << res_r->summary << endl;
    if ( 0&&kind_r == ResTraits<Package>::kind )
    {
      data::Package_Ptr p = dynamic_pointer_cast<data::Package>(res_r);
      DBG << "  PKG: " << p << endl;
      DBG << "  LOC: " << p->repositoryLocation << endl;
    }
    static unsigned n = 20;
    if ( 0&&! --n )
      throw;
  }

  public:

  virtual data::RecordId consumePackage( const data::RecordId & repository_id, const data::Package_Ptr & res_r )
  {
    return newId( res_r, ResTraits<Package>::kind );
  }

  virtual data::RecordId consumeSourcePackage( const data::RecordId & repository_id, const data::SrcPackage_Ptr & res_r )
  {
    return newId( res_r, ResTraits<SrcPackage>::kind );
  }

  virtual data::RecordId consumeProduct      ( const data::RecordId & repository_id, const data::Product_Ptr & res_r )
  {
    return newId( res_r, ResTraits<Product>::kind );
  }

  virtual data::RecordId consumePatch        ( const data::RecordId & repository_id, const data::Patch_Ptr & res_r )
  {
    return newId( res_r, ResTraits<Patch>::kind );
  }

  virtual data::RecordId consumePackageAtom  ( const data::RecordId & repository_id, const data::PackageAtom_Ptr & res_r )
  {
    return newId( res_r, ResTraits<Atom>::kind );
  }

  virtual data::RecordId consumeMessage      ( const data::RecordId & repository_id, const data::Message_Ptr & res_r )
  {
    return newId( res_r, ResTraits<Message>::kind );
  }

  virtual data::RecordId consumeScript       ( const data::RecordId & repository_id, const data::Script_Ptr & res_r )
  {
    return newId( res_r, ResTraits<Script>::kind );
  }

  virtual data::RecordId consumePattern      ( const data::RecordId & repository_id, const data::Pattern_Ptr & res_r )
  {
    return newId( res_r, ResTraits<Pattern>::kind );
  }

  virtual data::RecordId consumeChangelog    ( const data::RecordId & repository_id, const data::Resolvable_Ptr &, const Changelog & )
  { return data::RecordId(); }

  virtual data::RecordId consumeFilelist     ( const data::RecordId & repository_id, const data::Resolvable_Ptr &, const data::Filenames & )
  { return data::RecordId(); }

  void updatePackageLang( const data::RecordId & resolvable_id,
			  const data::Packagebase_Ptr & data_r )
  {
    SEC << lookup( resolvable_id ) << endl;
    INT << "  " << data_r->summary.text() << endl;
    INT << "  " << data_r->summary.text( Locale("en") ) << endl;
    if ( data_r->licenseToConfirm.locales().size() )
    {
      INT << "  " << data_r->licenseToConfirm.text() << endl;
      INT << "  " << data_r->licenseToConfirm.text( Locale("en") ) << endl;
      INT << "  " << data_r->licenseToConfirm.locales() << endl;
    }
 }
};

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  DummyConsumer dummy;
  RepoParser parser( data::RecordId(), dummy );

  parser.parse( "REPO" );

  INT << "===[END]============================================" << endl << endl;
  return 0;
}

