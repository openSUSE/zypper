#include <iterator>
#include <algorithm>
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Logger.h"
#include "zypp/cache/CacheTypes.h"
#include "zypp/cache/ResolvableQuery.h"
#include "zypp/Package.h"
#include "zypp/cache/sqlite3x/sqlite3x.hpp"

using namespace sqlite3x;
using namespace std;
using namespace zypp;

typedef shared_ptr<sqlite3_command> sqlite3_command_ptr;

namespace zypp { namespace cache {

struct ResolvableQuery::Impl
{
  Pathname _dbdir;
  string _fields;
  CacheTypes _type_cache;
  sqlite3_connection _con;
  sqlite3_command_ptr _cmd_attr_str;
  sqlite3_command_ptr _cmd_attr_tstr;
  sqlite3_command_ptr _cmd_attr_num;

  Impl( const Pathname &dbdir)
  : _dbdir(dbdir)
  , _type_cache(dbdir)
  {
    _con.open((dbdir + "zypp.db").asString().c_str());
    _con.executenonquery("PRAGMA cache_size=8000;");

    _cmd_attr_tstr.reset( new sqlite3_command( _con, "select a.text, l.name from text_attributes a,types l,types t where a.weak_resolvable_id=:rid and a.lang_id=l.id and a.attr_id=t.id and l.class=:lclass and t.class=:tclass and t.name=:tname;") );


    _cmd_attr_str.reset( new sqlite3_command( _con, "select a.text from text_attributes a,types l,types t where a.weak_resolvable_id=:rid and a.lang_id=l.id and a.attr_id=t.id and l.class=:lclass and l.name=:lname and t.class=:tclass and t.name=:tname;"));


    _cmd_attr_num.reset( new sqlite3_command( _con, "select a.value from numeric_attributes a,types t where a.weak_resolvable_id=:rid and a.attr_id=t.id and t.class=:tclass and t.name=:tname;"));

    MIL << "Creating Resolvable query impl" << endl;
    _fields = "id, name, version, release, epoch, arch, kind, installed_size, archive_size, install_only, build_time, install_time, repository_id";
  }

  ~Impl()
  {
      MIL << "Destroying Resolvable query impl" << endl;
  }

  data::ResObject_Ptr fromRow( sqlite3_reader &reader )
  {
    data::ResObject_Ptr ptr (new data::ResObject);

    ptr->name = reader.getstring(1);
    ptr->edition = Edition( reader.getstring(2), reader.getstring(3), reader.getint(4));
    ptr->arch = _type_cache.archFor(reader.getint(5));

    // TODO get the rest of the data

    return ptr;
  }

  
  void query( const data::RecordId &id,
                  ProcessResolvable fnc )
  {
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    //con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");
    sqlite3_command cmd( con, "select " + _fields + " from resolvables where id=:id;");
    cmd.bind(":id", id);
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      fnc( id, fromRow(reader) );
    }
    con.executenonquery("COMMIT;");
  }


  void query( const std::string &s,
              ProcessResolvable fnc  )
  {  
    
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    //con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");
    sqlite3_command cmd( con, "select " + _fields + " from resolvables where name like '%:name%';");
    cmd.bind(":name", s);
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      fnc( reader.getint64(0), fromRow(reader) );
    }
    con.executenonquery("COMMIT;");
  }


  std::string queryStringAttribute( const data::RecordId &record_id,
                                    const std::string &klass,
                                    const std::string &name,
                                    const std::string &default_value )
  {
    string value;
    return queryStringAttributeTranslationInternal( _con, record_id, Locale(), klass, name, default_value);
  }


  std::string queryStringAttributeTranslation( const data::RecordId &record_id,
                                               const Locale &locale,
                                               const std::string &klass,
                                               const std::string &name,
                                               const std::string &default_value )
  {
    return queryStringAttributeTranslationInternal( _con, record_id, locale, klass, name, default_value );
  }


  TranslatedText queryTranslatedStringAttribute( const data::RecordId &record_id,
                                                 const std::string &klass,
                                                 const std::string &name,
                                                 const TranslatedText &default_value )
  {
    return queryTranslatedStringAttributeInternal( _con, record_id, klass, name, default_value );
  }


  bool queryBooleanAttribute( const data::RecordId &record_id,
                              const std::string &klass,
                              const std::string &name,
                              bool default_value )
  {
    return ( queryNumericAttributeInternal( _con, record_id, klass, name, default_value) > 0 );
  }
      
  int queryNumericAttribute( const data::RecordId &record_id,
                             const std::string &klass,
                             const std::string &name,
                             int default_value )
  {
    return queryNumericAttributeInternal( _con, record_id, klass, name, default_value);
  }

private:

  int queryNumericAttributeInternal( sqlite3_connection &con,
                                     const data::RecordId &record_id,
                                     const std::string &klass,
                                     const std::string &name,
                                     int default_value )
  {
    //con.executenonquery("BEGIN;");
    _cmd_attr_num->bind(":rid", record_id);

    _cmd_attr_num->bind(":tclass", klass);
    _cmd_attr_num->bind(":tname", name);

    sqlite3_reader reader = _cmd_attr_num->executereader();
    if ( reader.read() )
      return reader.getint(0);

    return default_value;
  }
  
  TranslatedText queryTranslatedStringAttributeInternal( sqlite3_connection &con,
                                                         const data::RecordId &record_id,
                                                         const std::string &klass,
                                                         const std::string &name,
                                                         const TranslatedText &default_value )
  {
    //con.executenonquery("PRAGMA cache_size=8000;");
    //con.executenonquery("BEGIN;");

    _cmd_attr_tstr->bind(":rid", record_id);
    _cmd_attr_tstr->bind(":lclass", "lang");

    _cmd_attr_tstr->bind(":tclass", klass);
    _cmd_attr_tstr->bind(":tname", name);

    TranslatedText result;
    sqlite3_reader reader = _cmd_attr_tstr->executereader();
    
    int c = 0;
    while(reader.read())
    {
      result.setText( reader.getstring(0), Locale( reader.getstring(1) ) );
      c++;
    }

    if ( c>0 )
      return result;
    
    return default_value;
  }

  std::string queryStringAttributeInternal( sqlite3_connection &con,
                                            const data::RecordId &record_id,
                                            const std::string &klass,
                                            const std::string &name,
                                            const std::string &default_value )
  {
    return queryStringAttributeTranslationInternal( con, record_id, Locale(), klass, name, default_value );
  }

  std::string queryStringAttributeTranslationInternal( sqlite3_connection &con,
                                                       const data::RecordId &record_id,
                                                       const Locale &locale,
                                                       const std::string &klass,
                                                       const std::string &name,
                                                        const std::string &default_value )
  {
    //con.executenonquery("BEGIN;");
    _cmd_attr_str->bind(":rid", record_id);
    _cmd_attr_str->bind(":lclass", "lang");
    if (locale == Locale() )
      _cmd_attr_str->bind(":lname", "none");
    else
      _cmd_attr_str->bind(":lname", locale.code());

    _cmd_attr_str->bind(":tclass", klass);
    _cmd_attr_str->bind(":tname", name);

    sqlite3_reader reader = _cmd_attr_str->executereader();
    
    if ( reader.read() )
      return reader.getstring(0);
    
    return default_value;
  }
};

//////////////////////////////////////////////////////////////////////////////
// FORWARD TO IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////

ResolvableQuery::ResolvableQuery( const Pathname &dbdir)
  : _pimpl(new Impl(dbdir))
{
  //MIL << "Creating Resolvable query" << endl;
}

ResolvableQuery::~ResolvableQuery()
{
  //MIL << "Destroying Resolvable query" << endl;
}

//////////////////////////////////////////////////////////////////////////////

void ResolvableQuery::query( const data::RecordId &id, ProcessResolvable fnc  )
{
  _pimpl->query(id, fnc);
}

//////////////////////////////////////////////////////////////////////////////

void ResolvableQuery::query( const std::string &s, ProcessResolvable fnc  )
{
  _pimpl->query(s, fnc);
}

//////////////////////////////////////////////////////////////////////////////

int ResolvableQuery::queryNumericAttribute( const data::RecordId &record_id,
                                            const std::string &klass,
                                            const std::string &name,
                                            int default_value )
{
  return _pimpl->queryNumericAttribute(record_id, klass, name, default_value);
}

bool ResolvableQuery::queryBooleanAttribute( const data::RecordId &record_id,
                                             const std::string &klass,
                                             const std::string &name,
                                             bool default_value )
{
  return _pimpl->queryNumericAttribute(record_id, klass, name, default_value);
}


std::string ResolvableQuery::queryStringAttribute( const data::RecordId &record_id,
                                                   const std::string &klass,
                                                   const std::string &name,
                                                   const std::string &default_value )
{
  return _pimpl->queryStringAttribute(record_id, klass, name, default_value);
}

//////////////////////////////////////////////////////////////////////////////

std::string ResolvableQuery::queryStringAttributeTranslation( const data::RecordId &record_id,
                                                              const Locale &locale,
                                                              const std::string &klass,
                                                              const std::string &name,
                                                              const std::string &default_value )
{
  return _pimpl->queryStringAttributeTranslation(record_id, locale, klass, name, default_value );
}

//////////////////////////////////////////////////////////////////////////////

TranslatedText ResolvableQuery::queryTranslatedStringAttribute( const data::RecordId &record_id,
                                                                const std::string &klass,
                                                                const std::string &name,
                                                                const TranslatedText &default_value )
{
  return _pimpl->queryTranslatedStringAttribute(record_id, klass, name, default_value );
}

//////////////////////////////////////////////////////////////////////////////

} } // namespace zypp::cache
