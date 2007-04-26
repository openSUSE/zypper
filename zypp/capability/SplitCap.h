/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/SplitCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_SPLITCAP_H
#define ZYPP_CAPABILITY_SPLITCAP_H

#include "zypp/capability/CapabilityImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(SplitCap)
    
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SplitCap
    //
    /** A pseudo Capability indicating a package split.
     *
     * <tt>packagename:/absolute/path</tt> means: If \c packagename is
     * installed on system, and has <tt>/absolute/path</tt> in it's
     * filelist, and is to be replaced/updated, then this package
     * (the one providing the SplitCap) should be installed too.
     *
     * \todo implement matches().
     * \todo See where and how we handle it. Currently the info is
     * just stored here. Splits <tt>packagename:/absolute/path</tt>
     * are shipped as \c provides (SuSE packages file), but have to
     * be freshens, and implemented as ConditionalCap.
    */
    class SplitCap : public CapabilityImpl
    {
    public:
      typedef SplitCap Self;
      typedef SplitCap_Ptr Ptr;
      typedef SplitCap_constPtr constPtr;
      
      /** Ctor */
      SplitCap( const Resolvable::Kind & refers_r,
                const std::string & name_r,
                const std::string & path_r )
      : CapabilityImpl( refers_r )
      , _name( name_r )
      , _path( path_r )
      {}
    public:
      /**  */
      virtual const Kind & kind() const;

      virtual bool relevant() const
      { return false; }

      /** */
      virtual CapMatch matches( const CapabilityImpl::constPtr & rhs ) const;

      /**  <tt>name:/path</tt> */
      virtual std::string encode() const;

      const std::string & name() const
      { return _name; }

      const std::string & path() const
      { return _path; }

    private:
      /**  */
      std::string _name;
      /**  */
      std::string _path;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_SPLITCAP_H
