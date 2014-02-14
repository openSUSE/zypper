/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SolvAttr.cc
 *
*/
extern "C"
{
#include <solv/knownid.h>
}

#include <iostream>

#include "zypp/base/String.h"
#include "zypp/sat/SolvAttr.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace sat
{ /////////////////////////////////////////////////////////////////

  const SolvAttr SolvAttr::allAttr( detail::noId );
  const SolvAttr SolvAttr::noAttr;

#warning STILL ATTRIBUTES HERE WHICH ARE NOT PROVIDED BY SOLV FILES
// At least the ones that do nat have a solv/knownid.

  const SolvAttr SolvAttr::name         ( SOLVABLE_NAME );
  const SolvAttr SolvAttr::edition      ( SOLVABLE_EVR );
  const SolvAttr SolvAttr::arch         ( SOLVABLE_ARCH );
  const SolvAttr SolvAttr::vendor	( SOLVABLE_VENDOR );

  const SolvAttr SolvAttr::provides	( SOLVABLE_PROVIDES );
  const SolvAttr SolvAttr::obsoletes	( SOLVABLE_OBSOLETES );
  const SolvAttr SolvAttr::conflicts	( SOLVABLE_CONFLICTS );
  const SolvAttr SolvAttr::requires	( SOLVABLE_REQUIRES );
  const SolvAttr SolvAttr::recommends	( SOLVABLE_RECOMMENDS );
  const SolvAttr SolvAttr::suggests	( SOLVABLE_SUGGESTS );
  const SolvAttr SolvAttr::supplements	( SOLVABLE_SUPPLEMENTS );
  const SolvAttr SolvAttr::enhances	( SOLVABLE_ENHANCES );

  const SolvAttr SolvAttr::summary      ( SOLVABLE_SUMMARY );       // translated
  const SolvAttr SolvAttr::description  ( SOLVABLE_DESCRIPTION );   // translated
  const SolvAttr SolvAttr::insnotify    ( SOLVABLE_MESSAGEINS );    // translated
  const SolvAttr SolvAttr::delnotify    ( SOLVABLE_MESSAGEDEL );    // translated
  const SolvAttr SolvAttr::eula		( SOLVABLE_EULA );          // translated
  const SolvAttr SolvAttr::cpeid        ( SOLVABLE_CPEID );
  const SolvAttr SolvAttr::installtime  ( SOLVABLE_INSTALLTIME );
  const SolvAttr SolvAttr::buildtime    ( SOLVABLE_BUILDTIME );
  const SolvAttr SolvAttr::installsize  ( SOLVABLE_INSTALLSIZE );
  const SolvAttr SolvAttr::downloadsize ( SOLVABLE_DOWNLOADSIZE );
  const SolvAttr SolvAttr::diskusage    ( SOLVABLE_DISKUSAGE );

  //package
  const SolvAttr SolvAttr::checksum     ( SOLVABLE_CHECKSUM );
  const SolvAttr SolvAttr::medianr	( SOLVABLE_MEDIANR );
  const SolvAttr SolvAttr::mediafile	( SOLVABLE_MEDIAFILE );
  const SolvAttr SolvAttr::mediadir	( SOLVABLE_MEDIADIR );
  const SolvAttr SolvAttr::changelog    ( "changelog" );
  const SolvAttr SolvAttr::buildhost    ( SOLVABLE_BUILDHOST );
  const SolvAttr SolvAttr::distribution ( SOLVABLE_DISTRIBUTION );
  const SolvAttr SolvAttr::license      ( SOLVABLE_LICENSE );
  const SolvAttr SolvAttr::packager     ( SOLVABLE_PACKAGER );
  const SolvAttr SolvAttr::group        ( SOLVABLE_GROUP );
  const SolvAttr SolvAttr::keywords     ( SOLVABLE_KEYWORDS );
  const SolvAttr SolvAttr::sourcesize   ( "sourcesize" );
  const SolvAttr SolvAttr::authors      ( SOLVABLE_AUTHORS );
  const SolvAttr SolvAttr::filelist     ( SOLVABLE_FILELIST );
  const SolvAttr SolvAttr::sourcearch   ( SOLVABLE_SOURCEARCH );
  const SolvAttr SolvAttr::sourcename   ( SOLVABLE_SOURCENAME );
  const SolvAttr SolvAttr::sourceevr    ( SOLVABLE_SOURCEEVR );
  const SolvAttr SolvAttr::headerend    ( SOLVABLE_HEADEREND );
  const SolvAttr SolvAttr::url          ( SOLVABLE_URL );

  // patch
  const SolvAttr SolvAttr::patchcategory            ( SOLVABLE_PATCHCATEGORY );
  const SolvAttr SolvAttr::rebootSuggested          ( UPDATE_REBOOT );
  const SolvAttr SolvAttr::restartSuggested         ( UPDATE_RESTART );
  const SolvAttr SolvAttr::reloginSuggested         ( UPDATE_RELOGIN );
  const SolvAttr SolvAttr::message                  ( UPDATE_MESSAGE );
  const SolvAttr SolvAttr::severity                 ( UPDATE_SEVERITY );
  const SolvAttr SolvAttr::updateCollection         ( UPDATE_COLLECTION );
  const SolvAttr SolvAttr::updateCollectionName     ( UPDATE_COLLECTION_NAME );
  const SolvAttr SolvAttr::updateCollectionEvr      ( UPDATE_COLLECTION_EVR );
  const SolvAttr SolvAttr::updateCollectionArch     ( UPDATE_COLLECTION_ARCH );
  const SolvAttr SolvAttr::updateCollectionFilename ( UPDATE_COLLECTION_FILENAME );
  const SolvAttr SolvAttr::updateCollectionFlags    ( UPDATE_COLLECTION_FLAGS );
  const SolvAttr SolvAttr::updateReference          ( UPDATE_REFERENCE );
  const SolvAttr SolvAttr::updateReferenceType      ( UPDATE_REFERENCE_TYPE );
  const SolvAttr SolvAttr::updateReferenceHref      ( UPDATE_REFERENCE_HREF );
  const SolvAttr SolvAttr::updateReferenceId        ( UPDATE_REFERENCE_ID );
  const SolvAttr SolvAttr::updateReferenceTitle     ( UPDATE_REFERENCE_TITLE );

  //pattern
  const SolvAttr SolvAttr::isvisible    ( SOLVABLE_ISVISIBLE );
  const SolvAttr SolvAttr::icon         ( SOLVABLE_ICON );
  const SolvAttr SolvAttr::order        ( SOLVABLE_ORDER );
  const SolvAttr SolvAttr::isdefault    ( "isdefault" );
  const SolvAttr SolvAttr::category     ( SOLVABLE_CATEGORY );    // translated
  const SolvAttr SolvAttr::script       ( "script" );
  const SolvAttr SolvAttr::includes     ( SOLVABLE_INCLUDES );
  const SolvAttr SolvAttr::extends      ( SOLVABLE_EXTENDS );

  // product
  const SolvAttr SolvAttr::productReferenceFile  ( PRODUCT_REFERENCEFILE );
  const SolvAttr SolvAttr::productProductLine    ( PRODUCT_PRODUCTLINE );
  const SolvAttr SolvAttr::productShortlabel     ( PRODUCT_SHORTLABEL );
  const SolvAttr SolvAttr::productDistproduct    ( PRODUCT_DISTPRODUCT );
  const SolvAttr SolvAttr::productDistversion    ( PRODUCT_DISTVERSION );
  const SolvAttr SolvAttr::productType           ( PRODUCT_TYPE );
  const SolvAttr SolvAttr::productFlags          ( PRODUCT_FLAGS );
  const SolvAttr SolvAttr::productEndOfLife      ( PRODUCT_ENDOFLIFE );
  const SolvAttr SolvAttr::productRegisterTarget ( PRODUCT_REGISTER_TARGET );
  const SolvAttr SolvAttr::productRegisterRelease( PRODUCT_REGISTER_RELEASE );
  const SolvAttr SolvAttr::productUrl            ( PRODUCT_URL );
  const SolvAttr SolvAttr::productUrlType        ( PRODUCT_URL_TYPE );
  /** array of repoids, hopefully label s too */
  const SolvAttr SolvAttr::productUpdates		( PRODUCT_UPDATES );
  const SolvAttr SolvAttr::productUpdatesRepoid		( PRODUCT_UPDATES_REPOID );

  // repository
  const SolvAttr SolvAttr::repositoryDeltaInfo		( REPOSITORY_DELTAINFO );
  const SolvAttr SolvAttr::repositoryAddedFileProvides	( REPOSITORY_ADDEDFILEPROVIDES );
  const SolvAttr SolvAttr::repositoryRpmDbCookie	( REPOSITORY_RPMDBCOOKIE );
  const SolvAttr SolvAttr::repositoryTimestamp		( REPOSITORY_TIMESTAMP );
  const SolvAttr SolvAttr::repositoryExpire		( REPOSITORY_EXPIRE );
  /** array of repositoryProductLabel repositoryProductCpeid pairs */
  const SolvAttr SolvAttr::repositoryUpdates		( REPOSITORY_UPDATES );
  /** array of repositoryProductLabel repositoryProductCpeid pairs */
  const SolvAttr SolvAttr::repositoryDistros		( REPOSITORY_DISTROS );
  const SolvAttr SolvAttr::repositoryProductLabel	( REPOSITORY_PRODUCT_LABEL );
  const SolvAttr SolvAttr::repositoryProductCpeid	( REPOSITORY_PRODUCT_CPEID );
  const SolvAttr SolvAttr::repositoryRepoid		( REPOSITORY_REPOID );
  const SolvAttr SolvAttr::repositoryKeywords		( REPOSITORY_KEYWORDS );
  const SolvAttr SolvAttr::repositoryRevision		( REPOSITORY_REVISION );
  const SolvAttr SolvAttr::repositoryToolVersion	( REPOSITORY_TOOLVERSION );



  /////////////////////////////////////////////////////////////////

  SolvAttr SolvAttr::parent() const
  {
    switch( id() )
    {
      case UPDATE_COLLECTION_NAME:
      case UPDATE_COLLECTION_EVR:
      case UPDATE_COLLECTION_ARCH:
      case UPDATE_COLLECTION_FILENAME:
      case UPDATE_COLLECTION_FLAGS:
        return updateCollection;
        break;

      case UPDATE_REFERENCE_TYPE:
      case UPDATE_REFERENCE_HREF:
      case UPDATE_REFERENCE_ID:
      case UPDATE_REFERENCE_TITLE:
        return updateReference;
        break;
    }
    return noAttr;
  }

} // namespace sat
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
