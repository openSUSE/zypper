#ifndef CachedResolvableDataProvider_H
#define CachedResolvableDataProvider_H

#include "zypp2/cache/sqlite3x/sqlite3x.hpp"
#include "zypp/data/RecordId.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/Package.h"

using namespace sqlite3x;

namespace zypp
{
  namespace cache
  {

    template<class _res> class CachedDataQuery;

    template<> class CachedDataQuery<Package>
    {

    };

    template<class _Res> class DataProviderRestraits
    {
      public:

    };

    template<class _Res>
    class CachedDataProvider
    {
      CachedDataProvider<_Res>( const zypp::Pathname &dbdir, const data::RecordId id );
    };

//     template<> class CachedDataProvider<ResObject> : public DataProviderRestraits<ResObject>::DataType
//     {
//       public:
//       CachedDataProvider<Package>( sqlite::sqlite3_connection &con, const data::RecordId id )
//       {
//         //sqlite3_command( con, "select
//         //con.
//       }
//
//       private:
//       //DataType
//     };

//     template<> class CachedDataProvider<Package> : public DataProviderRestraits<Package>::DataType
//     {
//       public:
//       CachedDataProvider<Package>( sqlite3x::sqlite3_connection &con, const data::RecordId id )
//       {
//         //sqlite3_command( con, "select
//         //con.
//       }
//     };






  }
}

#endif
