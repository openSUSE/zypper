/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/VersionedCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_VERSIONEDCAP_H
#define ZYPP_CAPABILITY_VERSIONEDCAP_H

#include "zypp/capability/NamedCap.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : VersionedCap
    //
    /** A \ref NamedCap providing an Edition::MatchRange.
     * Overloads \ref encode and provides the \ref range.
     * Remaining stuff is handles in \ref NamedCap.
    */
    class VersionedCap : public NamedCap
    {
    public:
      /** Ctor */
      VersionedCap( const Resolvable::Kind & refers_r,
                    const std::string & name_r,
                    Rel op_r,
                    const Edition & edition_r )
      : NamedCap( refers_r, name_r )
      , _range( op_r, edition_r )
      {}

    public:
      /** Name Op Edition. */
      virtual std::string encode() const;

      /** Name only. */
      virtual std::string index() const;

    protected:
      /** Implementation dependent value. */
      virtual const Edition::MatchRange & range() const;

    private:
      /**  */
      Edition::MatchRange _range;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_VERSIONEDCAP_H
