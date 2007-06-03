
#include "zypp2/cache/CacheTypes.h"
#include "zypp2/cache/ResolvableQuery.h"
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

using namespace sqlite3x;
using namespace std;

namespace zypp { namespace cache {


struct ResolvableQuery::Impl
{
  Pathname _dbdir;
  string _fields;
  CacheTypes _type_cache;
  
  Impl( const Pathname &dbdir)
  : _dbdir(dbdir)
    , _type_cache(dbdir)
  {
    _fields = "id, name, version, release, epoch, arch, kind, installed_size, archive_size, install_only, build_time, install_time, repository_id";
  }

  ~Impl()
  {
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
                                    const std::string &name )
  {
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    return queryStringAttributeTranslationInternal( con, record_id, Locale(), klass, name);
  }


  std::string queryStringAttributeTranslation( const data::RecordId &record_id,
                                               const Locale &locale,
                                               const std::string &klass,
                                               const std::string &name )
  {
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    return queryStringAttributeTranslationInternal( con, record_id, locale, klass, name );
  }


  TranslatedText queryTranslatedStringAttribute( const data::RecordId &record_id,
                                                 const std::string &klass,
                                                 const std::string &name )
  {
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    return queryTranslatedStringAttributeInternal( con, record_id, klass, name );
  }


  int queryNumericAttribute( const data::RecordId &record_id,
                                 const std::string &klass,
                                 const std::string &name )
  {
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    return queryNumericAttributeInternal( con, record_id, klass, name);
  }

private:

  int queryNumericAttributeInternal( sqlite3_connection &con,
                                     const data::RecordId &record_id,
                                     const std::string &klass,
                                     const std::string &name )
  {
    con.executenonquery("BEGIN;");
    sqlite3_command cmd( con, "select a.value from numeric_attributes a,types t where a.weak_resolvable_id=:rid and a.attr_id=t.id and t.class=:tclass and t.name=:tname;");

    cmd.bind(":rid", record_id);

    cmd.bind(":tclass", klass);
    cmd.bind(":tname", name);

    return cmd.executeint();
  }

  TranslatedText queryTranslatedStringAttributeInternal( sqlite3_connection &con,
                                                         const data::RecordId &record_id,
                                                         const std::string &klass,
                                                         const std::string &name )
  {
    //con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");
    sqlite3_command cmd( con, "select a.text, l.name from text_attributes a,types l,types t where a.weak_resolvable_id=:rid and a.lang_id=l.id and a.attr_id=t.id and l.class=:lclass and t.class=:tclass and t.name=:tname;");

    cmd.bind(":rid", record_id);
    cmd.bind(":lclass", "lang");

    cmd.bind(":tclass", klass);
    cmd.bind(":tname", name);

    TranslatedText result;
    sqlite3_reader reader = cmd.executereader();
    while(reader.read())
    {
      result.setText( reader.getstring(0), Locale( reader.getstring(1) ) );
    }
    return result;
  }

  std::string queryStringAttributeInternal( sqlite3_connection &con,
                                            const data::RecordId &record_id,
                                            const std::string &klass,
                                            const std::string &name )
  {
    return queryStringAttributeTranslationInternal( con, record_id, Locale(), klass, name);
  }

  std::string queryStringAttributeTranslationInternal( sqlite3_connection &con,
                                                       const data::RecordId &record_id,
                                                       const Locale &locale,
                                                       const std::string &klass,
                                                       const std::string &name )
  {
    //con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");
    sqlite3_command cmd( con, "select a.text from text_attributes a,types l,types t where a.weak_resolvable_id=:rid and a.lang_id=l.id and a.attr_id=t.id and l.class=:lclass and l.name=:lname and t.class=:tclass and t.name=:tname;");

    cmd.bind(":rid", record_id);
    cmd.bind(":lclass", "lang");
    if (locale == Locale() )
      cmd.bind(":lname", "none");
    else
      cmd.bind(":lname", locale.code());

    cmd.bind(":tclass", klass);
    cmd.bind(":tname", name);

    return cmd.executestring();
  }
};

//////////////////////////////////////////////////////////////////////////////
// FORWARD TO IMPLEMENTATION
//////////////////////////////////////////////////////////////////////////////

ResolvableQuery::ResolvableQuery( const Pathname &dbdir)
  : _pimpl(new Impl(dbdir))
{
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
                                            const std::string &name )
{
  return _pimpl->queryNumericAttribute(record_id, klass, name);
}

bool ResolvableQuery::queryBooleanAttribute( const data::RecordId &record_id,
                                             const std::string &klass,
                                             const std::string &name )
{
  return _pimpl->queryNumericAttribute(record_id, klass, name);
}


std::string ResolvableQuery::queryStringAttribute( const data::RecordId &record_id,
                                                   const std::string &klass,
                                                   const std::string &name )
{
  return _pimpl->queryStringAttribute(record_id, klass, name);
}

//////////////////////////////////////////////////////////////////////////////

std::string ResolvableQuery::queryStringAttributeTranslation( const data::RecordId &record_id,
                                                              const Locale &locale,
                                                              const std::string &klass,
                                                              const std::string &name )
{
  return _pimpl->queryStringAttributeTranslation(record_id, locale, klass, name);
}

//////////////////////////////////////////////////////////////////////////////

TranslatedText ResolvableQuery::queryTranslatedStringAttribute( const data::RecordId &record_id,
                                                                const std::string &klass,
                                                                const std::string &name )
{
  return _pimpl->queryTranslatedStringAttribute(record_id, klass, name);
}

//////////////////////////////////////////////////////////////////////////////

} } // namespace zypp::cache
