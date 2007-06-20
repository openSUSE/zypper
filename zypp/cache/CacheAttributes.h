/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/cache/CacheAttributes.h
 *
*/
#ifndef ZYPP_CACHE_CACHEATTRIBUTES_H
#define ZYPP_CACHE_CACHEATTRIBUTES_H

#include "zypp/cache/Attribute.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace cache
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /** \name Predefined attributes.
     * \see \ref Attribute
    */
    //@{
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrMessageText()                 { static Attribute a("Message","text"); return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrPackageAuthors()              { static Attribute a("Package","authors"); return a; }
    inline const Attribute & attrPackageBuildhost()            { static Attribute a("Package","buildhost"); return a; }
    inline const Attribute & attrPackageChecksum()             { static Attribute a("Package","checksum"); return a; }
    inline const Attribute & attrPackageChecksumType()         { static Attribute a("Package","checksumType"); return a; }
    inline const Attribute & attrPackageDistribution()         { static Attribute a("Package","distribution"); return a; }
    inline const Attribute & attrPackageGroup()                { static Attribute a("Package","group"); return a; }
    inline const Attribute & attrPackageKeywords()             { static Attribute a("Package","keywords"); return a; }
    inline const Attribute & attrPackageLicense()              { static Attribute a("Package","license"); return a; }
    inline const Attribute & attrPackageLocation()             { static Attribute a("Package","location"); return a; }
    inline const Attribute & attrPackageOperatingSystem()      { static Attribute a("Package","operatingSystem"); return a; }
    inline const Attribute & attrPackagePostin()               { static Attribute a("Package","postin"); return a; }
    inline const Attribute & attrPackagePostun()               { static Attribute a("Package","postun"); return a; }
    inline const Attribute & attrPackagePrein()                { static Attribute a("Package","prein"); return a; }
    inline const Attribute & attrPackagePreun()                { static Attribute a("Package","preun"); return a; }
    inline const Attribute & attrPackageUrl()                  { static Attribute a("Package","url"); return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrPatchAffectsPkgManager()      { static Attribute a("Patch","affectsPkgManager"); return a; }
    inline const Attribute & attrPatchCategory()               { static Attribute a("Patch","category"); return a; }
    inline const Attribute & attrPatchRebootNeeded()           { static Attribute a("Patch","rebootNeeded"); return a; }
    inline const Attribute & attrPatchTimestamp()              { static Attribute a("Patch","timestamp"); return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrPatternCategory()             { static Attribute a("Pattern","category"); return a; }
    inline const Attribute & attrPatternIcon()                 { static Attribute a("Pattern","icon"); return a; }
    inline const Attribute & attrPatternIsDefault()            { static Attribute a("Pattern","isDefault"); return a; }
    inline const Attribute & attrPatternOrder()                { static Attribute a("Pattern","order"); return a; }
    inline const Attribute & attrPatternUserVisible()          { static Attribute a("Pattern","userVisible"); return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrProductDistributionEdition()  { static Attribute a("Product","distributionEdition"); return a; }
    inline const Attribute & attrProductDistributionName()     { static Attribute a("Product","distributionName"); return a; }
    inline const Attribute & attrProductExtraUrls()            { static Attribute a("Product","extraUrls"); return a; }
    inline const Attribute & attrProductFlags()                { static Attribute a("Product","flags"); return a; }
    inline const Attribute & attrProductLongName()             { static Attribute a("Product","longName"); return a; }
    inline const Attribute & attrProductOptionalUrls()         { static Attribute a("Product","optionalUrls"); return a; }
    inline const Attribute & attrProductReleasenotesUrl()      { static Attribute a("Product","releasenotesUrl"); return a; }
    inline const Attribute & attrProductShortName()            { static Attribute a("Product","shortName"); return a; }
    inline const Attribute & attrProductUpdateUrls()           { static Attribute a("Product","updateUrls"); return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrResObjectArchiveSize()        { static Attribute a("ResObject","archiveSize"); return a; }
    inline const Attribute & attrResObjectBuildTime()          { static Attribute a("ResObject","buildTime"); return a; }
    inline const Attribute & attrResObjectDelnotify()          { static Attribute a("ResObject","delnotify"); return a; }
    inline const Attribute & attrResObjectDescription()        { static Attribute a("ResObject","description"); return a; }
    inline const Attribute & attrResObjectInsnotify()          { static Attribute a("ResObject","insnotify"); return a; }
    inline const Attribute & attrResObjectInstallOnly()        { static Attribute a("ResObject","installOnly"); return a; }
    inline const Attribute & attrResObjectInstalledSize()      { static Attribute a("ResObject","installedSize"); return a; }
    inline const Attribute & attrResObjectLicenseToConfirm()   { static Attribute a("ResObject","licenseToConfirm"); return a; }
    inline const Attribute & attrResObjectSummary()            { static Attribute a("ResObject","summary"); return a; }
    inline const Attribute & attrResObjectVendor()             { static Attribute a("ResObject","vendor"); return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrScriptDoScript()              { static Attribute a("Script","doScript"); return a; }
    inline const Attribute & attrScriptDoScriptChecksum()      { static Attribute a("Script","doScriptChecksum"); return a; }
    inline const Attribute & attrScriptDoScriptChecksumType()  { static Attribute a("Script","doScriptChecksumType"); return a; }
    inline const Attribute & attrScriptDoScriptLocation()      { static Attribute a("Script","doScriptLocation"); return a; }
    inline const Attribute & attrScriptUndoScript()            { static Attribute a("Script","undoScript"); return a; }
    inline const Attribute & attrScriptUndoScriptChecksum()    { static Attribute a("Script","undoScriptChecksum"); return a; }
    inline const Attribute & attrScriptUndoScriptChecksumType(){ static Attribute a("Script","undoScriptChecksumType"); return a; }
    inline const Attribute & attrScriptUndoScriptLocation()    { static Attribute a("Script","undoScriptLocation"); return a; }
    ///////////////////////////////////////////////////////////////////
    //@}

    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CACHE_CACHEATTRIBUTES_H
