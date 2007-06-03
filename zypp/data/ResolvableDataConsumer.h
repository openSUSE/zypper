/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_ResolvableDataConsumer_H
#define ZYPP_ResolvableDataConsumer_H


#include "zypp/data/RecordId.h"
#include "zypp/data/ResolvableData.h"


///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace data
{ /////////////////////////////////////////////////////////////////

  class ResolvableDataConsumer
  {
    public:

    ResolvableDataConsumer();
    virtual ~ResolvableDataConsumer();

    virtual void consumePackage( const data::RecordId &repository_id, const data::Package_Ptr &) = 0;
    virtual void consumeSourcePackage( const data::RecordId &repository_id, const data::SrcPackage_Ptr &) = 0;
    virtual void consumeProduct( const data::RecordId &repository_id, const data::Product_Ptr &) = 0;
    virtual void consumePatch( const data::RecordId &repository_id, const data::Patch_Ptr &) = 0;
    virtual void consumePackageAtom( const data::RecordId &repository_id, const data::PackageAtom_Ptr & ) = 0;
    virtual void consumeMessage( const data::RecordId &repository_id, const data::Message_Ptr &) = 0;
    virtual void consumeScript( const data::RecordId &repository_id, const data::Script_Ptr &) = 0;
    virtual void consumePattern( const data::RecordId &repository_id, const data::Pattern_Ptr &) = 0;

    virtual void consumeChangelog( const data::RecordId & repository_id, const data::Resolvable_Ptr &, const Changelog & ) = 0;
    virtual void consumeFilelist( const data::RecordId & repository_id, const data::Resolvable_Ptr &, const data::Filenames & ) = 0;
  };

} // namespace parser
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_ResolvableDataConsumer_H
