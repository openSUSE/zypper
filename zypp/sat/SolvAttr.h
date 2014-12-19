/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/SolvAttr.h
 *
*/
#ifndef ZYPP_SAT_SOLVATTR_H
#define ZYPP_SAT_SOLVATTR_H

#include <iosfwd>
#include <string>

#include "zypp/base/String.h"
#include "zypp/IdStringType.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace sat
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SolvAttr
  //
  /** Solvable attribute keys.
   *
   * Attributes associated with individual solvables,
   * or with the repository as a whole.
   *
   * \note If you add well known subsructure attributes, update \ref parent.
   *
   * \see \ref LookupAttr
   */
  class SolvAttr : public IdStringType<SolvAttr>
  {
    public:
      /** \name Some builtin SolvAttr constants. */
      //@{
      /** Value to request searching all Attributes (0). */
      static const SolvAttr allAttr;
      /** Value representing \c noAttr (<tt>""</tt>)*/
      static const SolvAttr noAttr;

      /** \name special solvable attributes which are part of the ::Solvable struct */
      //@{
      static const SolvAttr name;
      static const SolvAttr edition;
      static const SolvAttr arch;
      static const SolvAttr vendor;
      //@}

      /** \name dependency attributes */
      //@{
      static const SolvAttr provides;
      static const SolvAttr obsoletes;
      static const SolvAttr conflicts;
      static const SolvAttr requires;
      static const SolvAttr recommends;
      static const SolvAttr suggests;
      static const SolvAttr supplements;
      static const SolvAttr enhances;

      /** \name common */
      //@{
      static const SolvAttr summary;
      static const SolvAttr description;
      static const SolvAttr insnotify;
      static const SolvAttr delnotify;
      static const SolvAttr eula;
      static const SolvAttr installtime;
      static const SolvAttr buildtime;
      static const SolvAttr installsize;
      static const SolvAttr downloadsize;
      static const SolvAttr diskusage;
      static const SolvAttr cpeid;
      //@}

      /** \name package */
      //@{
      static const SolvAttr checksum;
      static const SolvAttr mediadir;
      static const SolvAttr medianr;
      static const SolvAttr mediafile;
      static const SolvAttr changelog;
      static const SolvAttr buildhost;
      static const SolvAttr distribution;
      static const SolvAttr license;
      static const SolvAttr packager;
      static const SolvAttr group;
      static const SolvAttr keywords;
      static const SolvAttr sourcesize;
      static const SolvAttr authors;
      static const SolvAttr filelist;
      static const SolvAttr sourcearch;
      static const SolvAttr sourcename;
      static const SolvAttr sourceevr;
      static const SolvAttr headerend;
      static const SolvAttr url;
      //@}

       /** \name patch */
      //@{
      static const SolvAttr patchcategory;
      static const SolvAttr rebootSuggested;
      static const SolvAttr restartSuggested;
      static const SolvAttr reloginSuggested;
      static const SolvAttr message;
      static const SolvAttr severity;
      static const SolvAttr updateCollection;		// SUB-STRUCTURE:
      static const SolvAttr updateCollectionName;	// name
      static const SolvAttr updateCollectionEvr;	// evr
      static const SolvAttr updateCollectionArch;	// arch
      static const SolvAttr updateCollectionFilename;	// filename
      static const SolvAttr updateCollectionFlags;	// flags
      static const SolvAttr updateReference;		// SUB-STRUCTURE:
      static const SolvAttr updateReferenceType;	// type
      static const SolvAttr updateReferenceHref;	// href
      static const SolvAttr updateReferenceId;		// id
      static const SolvAttr updateReferenceTitle;	// title
      //@}

      /** \name pattern */
      //@{
      static const SolvAttr isvisible;
      static const SolvAttr icon;
      static const SolvAttr order;
      static const SolvAttr isdefault;
      static const SolvAttr category;
      static const SolvAttr script;
      static const SolvAttr includes;
      static const SolvAttr extends;
      //@}

      /** \name product */
      //@{
      static const SolvAttr productReferenceFile;
      static const SolvAttr productProductLine;
      static const SolvAttr productShortlabel;
      static const SolvAttr productDistproduct;
      static const SolvAttr productDistversion;
      static const SolvAttr productType;
      static const SolvAttr productFlags;
      static const SolvAttr productEndOfLife;
      static const SolvAttr productRegisterTarget;
      static const SolvAttr productRegisterRelease;
      static const SolvAttr productRegisterFlavor;
      static const SolvAttr productUrl;
      static const SolvAttr productUrlType;
      static const SolvAttr productUpdates;		// SUB-STRUCTURE:
      static const SolvAttr productUpdatesRepoid;	// repoid
      //@}

      /** \name repository */
      //@{
      static const SolvAttr repositoryDeltaInfo;
      static const SolvAttr repositoryAddedFileProvides;
      static const SolvAttr repositoryRpmDbCookie;
      static const SolvAttr repositoryTimestamp;
      static const SolvAttr repositoryExpire;
      static const SolvAttr repositoryUpdates;
      static const SolvAttr repositoryDistros;
      static const SolvAttr repositoryProductLabel;
      static const SolvAttr repositoryProductCpeid;
      static const SolvAttr repositoryRepoid;
      static const SolvAttr repositoryKeywords;
      static const SolvAttr repositoryRevision;
      static const SolvAttr repositoryToolVersion;
      //@}

      //@}
    public:
      /** Default ctor: \ref noAttr */
      SolvAttr() {}

      /** Ctor taking kind as string. */
      explicit SolvAttr( sat::detail::IdType id_r )  : _str( id_r ) {}
      explicit SolvAttr( const IdString & idstr_r )  : _str( idstr_r ) {}
      explicit SolvAttr( const std::string & str_r ) : _str( str_r ) {}
      explicit SolvAttr( const char * cstr_r )       : _str( cstr_r ) {}

      /** Return the parent of well know sub-structure attributes (\ref SolvAttr::noAttr if none).
       * \li \ref updateCollection
       * \li \ref updateReference
       */
      SolvAttr parent() const;

      /** Whether this is a well know sub-structure attribute. */
      bool hasParent() const
      { return parent() != noAttr; }

    private:
      friend class IdStringType<SolvAttr>;
      IdString _str;
  };

  /////////////////////////////////////////////////////////////////
} // namespace sat
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_sat_SolvAttr_H
