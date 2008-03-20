/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SolvAttr.h
 *
*/
#ifndef ZYPP_SolvAttr_H
#define ZYPP_SolvAttr_H

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
   */
  class SolvAttr : public IdStringType<SolvAttr>
  {
    public:
      /** \name Some builtin SolvAttr constants. */
      //@{
      /** Value representing \c noAttr (<tt>""</tt>)*/
      static const SolvAttr noAttr;

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

      //package
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
      static const SolvAttr filenames;
      static const SolvAttr srcpkgname;
      static const SolvAttr srcpkgedition;
      static const SolvAttr filelist;
      static const SolvAttr sourcearch;
      static const SolvAttr sourcename;
      static const SolvAttr sourceevr;
      static const SolvAttr headerend;

      // patch
      static const SolvAttr patchcategory;

      // patern
      static const SolvAttr isvisible;
      static const SolvAttr icon;
      static const SolvAttr order;
      static const SolvAttr isdefault;
      static const SolvAttr category;
      static const SolvAttr script;
      static const SolvAttr includes;
      static const SolvAttr extends;

      //@}

    public:
      /** Default ctor: \ref noAttr */
      SolvAttr() {}

      /** Ctor taking kind as string. */
      explicit SolvAttr( sat::detail::IdType id_r )  : _str( IdString(id_r).c_str() ) {}
      explicit SolvAttr( const IdString & idstr_r )  : _str( idstr_r.c_str() ) {}
      explicit SolvAttr( const std::string & str_r ) : _str( str_r ) {}
      explicit SolvAttr( const char * cstr_r )       : _str( cstr_r ) {}

    private:
      friend class IdStringType<SolvAttr>;
      IdString _str;
  };

  /////////////////////////////////////////////////////////////////
} // namespace sat
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SolvAttr_H
