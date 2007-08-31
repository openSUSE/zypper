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

/** Define OnMediaLocation attributes using a common prefix for all attributes.
 * \code
 * defineOnMediaLocationAttr( attrPackageLocation, "Package", "location" )
 * \endcode
*/
#define defineOnMediaLocationAttr(OMLATTRPREFIX,KLASS,ATTRNAMEPREFIX )                                                                    \
    inline const Attribute & OMLATTRPREFIX##MediaNr()          { static Attribute a(KLASS,#ATTRNAMEPREFIX"MediaNr");          return a; } \
    inline const Attribute & OMLATTRPREFIX##Filename()         { static Attribute a(KLASS,#ATTRNAMEPREFIX"Filename");         return a; } \
    inline const Attribute & OMLATTRPREFIX##DownloadSize()     { static Attribute a(KLASS,#ATTRNAMEPREFIX"DownloadSize");     return a; } \
    inline const Attribute & OMLATTRPREFIX##Checksum()         { static Attribute a(KLASS,#ATTRNAMEPREFIX"Checksum");         return a; } \
    inline const Attribute & OMLATTRPREFIX##ChecksumType()     { static Attribute a(KLASS,#ATTRNAMEPREFIX"ChecksumType");     return a; } \
    inline const Attribute & OMLATTRPREFIX##OpenSize()         { static Attribute a(KLASS,#ATTRNAMEPREFIX"OpenSize");         return a; } \
    inline const Attribute & OMLATTRPREFIX##OpenChecksum()     { static Attribute a(KLASS,#ATTRNAMEPREFIX"OpenChecksum");     return a; } \
    inline const Attribute & OMLATTRPREFIX##OpenChecksumType() { static Attribute a(KLASS,#ATTRNAMEPREFIX"OpenChecksumType"); return a; }

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
    inline const Attribute & attrMessageText()                 { static Attribute a("Message","text");                 return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrPackageAuthors()              { static Attribute a("Package","authors",               Attribute::SHARED); return a; }
    inline const Attribute & attrPackageBuildhost()            { static Attribute a("Package","buildhost");            return a; }
    inline const Attribute & attrPackageChecksumType()         { static Attribute a("Package","checksumType");         return a; }
    inline const Attribute & attrPackageDistribution()         { static Attribute a("Package","distribution");         return a; }
    inline const Attribute & attrPackageGroup()                { static Attribute a("Package","group");                return a; }
    inline const Attribute & attrPackagePackager()             { static Attribute a("Package","packager");             return a; }
    inline const Attribute & attrPackageKeywords()             { static Attribute a("Package","keywords");             return a; }
    inline const Attribute & attrPackageLicense()              { static Attribute a("Package","license");              return a; }
    inline const Attribute & attrPackageOperatingSystem()      { static Attribute a("Package","operatingSystem");      return a; }
    inline const Attribute & attrPackagePostin()               { static Attribute a("Package","postin");               return a; }
    inline const Attribute & attrPackagePostun()               { static Attribute a("Package","postun");               return a; }
    inline const Attribute & attrPackagePrein()                { static Attribute a("Package","prein");                return a; }
    inline const Attribute & attrPackagePreun()                { static Attribute a("Package","preun");                return a; }
    inline const Attribute & attrPackageUrl()                  { static Attribute a("Package","url");                  return a; }
    inline const Attribute & attrPackageSourcePkgName()        { static Attribute a("Package","sourcePkgName");        return a; }
    inline const Attribute & attrPackageSourcePkgEdition()     { static Attribute a("Package","sourcePkgEdition");     return a; }
    // define attrPackageLocationMediaNr, attrPackageLocationFilename, etc.
    defineOnMediaLocationAttr( attrPackageLocation, "Package", "location" )
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrPatchAffectsPkgManager()      { static Attribute a("Patch","affectsPkgManager");      return a; }
    inline const Attribute & attrPatchCategory()               { static Attribute a("Patch","category");               return a; }
    inline const Attribute & attrPatchId()                     { static Attribute a("Patch","id");                     return a; }
    inline const Attribute & attrPatchRebootNeeded()           { static Attribute a("Patch","rebootNeeded");           return a; }
    inline const Attribute & attrPatchTimestamp()              { static Attribute a("Patch","timestamp");              return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrPatternCategory()             { static Attribute a("Pattern","category");             return a; }
    inline const Attribute & attrPatternIcon()                 { static Attribute a("Pattern","icon");                 return a; }
    inline const Attribute & attrPatternIsDefault()            { static Attribute a("Pattern","isDefault");            return a; }
    inline const Attribute & attrPatternOrder()                { static Attribute a("Pattern","order");                return a; }
    inline const Attribute & attrPatternUserVisible()          { static Attribute a("Pattern","userVisible");          return a; }
    inline const Attribute & attrPatternUiIncludes()           { static Attribute a("Pattern","uiIncludes");           return a; }
    inline const Attribute & attrPatternUiExtends()            { static Attribute a("Pattern","uiExtends");            return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrProductDistributionEdition()  { static Attribute a("Product","distributionEdition");  return a; }
    inline const Attribute & attrProductDistributionName()     { static Attribute a("Product","distributionName");     return a; }
    inline const Attribute & attrProductExtraUrls()            { static Attribute a("Product","extraUrls");            return a; }
    inline const Attribute & attrProductFlags()                { static Attribute a("Product","flags");                return a; }
    inline const Attribute & attrProductLongName()             { static Attribute a("Product","longName");             return a; }
    inline const Attribute & attrProductOptionalUrls()         { static Attribute a("Product","optionalUrls");         return a; }
    inline const Attribute & attrProductReleasenotesUrl()      { static Attribute a("Product","releasenotesUrl");      return a; }
    inline const Attribute & attrProductShortName()            { static Attribute a("Product","shortName");            return a; }
    inline const Attribute & attrProductType()                 { static Attribute a("Product","type");                 return a; }
    inline const Attribute & attrProductUpdateUrls()           { static Attribute a("Product","updateUrls");           return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrResObjectBuildTime()          { static Attribute a("ResObject","buildTime");                              return a; }
    inline const Attribute & attrResObjectDelnotify()          { static Attribute a("ResObject","delnotify",           Attribute::SHARED); return a; }
    inline const Attribute & attrResObjectDescription()        { static Attribute a("ResObject","description",         Attribute::SHARED); return a; }
    inline const Attribute & attrResObjectInsnotify()          { static Attribute a("ResObject","insnotify",           Attribute::SHARED); return a; }
    inline const Attribute & attrResObjectInstallOnly()        { static Attribute a("ResObject","installOnly");                            return a; }
    inline const Attribute & attrResObjectInstalledSize()      { static Attribute a("ResObject","installedSize");                          return a; }
    inline const Attribute & attrResObjectLicenseToConfirm()   { static Attribute a("ResObject","licenseToConfirm",    Attribute::SHARED); return a; }
    inline const Attribute & attrResObjectSummary()            { static Attribute a("ResObject","summary",             Attribute::SHARED); return a; }
    inline const Attribute & attrResObjectVendor()             { static Attribute a("ResObject","vendor");                                 return a; }
    ///////////////////////////////////////////////////////////////////
    inline const Attribute & attrScriptDoScript()              { static Attribute a("Script","doScript");                    return a; }
    // define attrScriptDoScriptLocationMediaNr, attrLocationFilename, etc.
    defineOnMediaLocationAttr( attrScriptDoScriptLocation, "Script", "doScriptLocation" )
    inline const Attribute & attrScriptUndoScript()            { static Attribute a("Script","undoScript");                  return a; }
    // define attrScriptUndoScriptLocationMediaNr, attrLocationFilename, etc.
    defineOnMediaLocationAttr( attrScriptUndoScriptLocation, "Script", "undoScriptLocation" )
    ///////////////////////////////////////////////////////////////////
    // define attrSrcPackageLocationMediaNr, attrSrcPackageLocationFilename, etc.
    defineOnMediaLocationAttr( attrSrcPackageLocation, "SrcPackage", "location" )
    ///////////////////////////////////////////////////////////////////
    //@}

    /////////////////////////////////////////////////////////////////
  } // namespace cache
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CACHE_CACHEATTRIBUTES_H
