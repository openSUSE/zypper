/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/FileCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_FILECAP_H
#define ZYPP_CAPABILITY_FILECAP_H

#include "zypp/capability/CapabilityImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(FileCap)
    
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : FileCap
    //
    /** A \c filename matching if some Resolvable provides it.
     *
     * \todo Check whether we have to look into the Resolable filelist as well.
    */
    class FileCap : public CapabilityImpl
    {
    public:
      typedef FileCap Self;
      typedef FileCap_Ptr       Ptr;
      typedef FileCap_constPtr  constPtr;

      /** Ctor */
      FileCap( const Resolvable::Kind & refers_r, const std::string & fname_r )
      : CapabilityImpl( refers_r )
      , _fname( fname_r )
      {}

    public:
      std::string filename() const
      { return _fname; }
      
      /**  */
      virtual const Kind & kind() const;

      /** Same kind, refers and filename. */
      virtual CapMatch matches( const CapabilityImpl::constPtr & rhs ) const;

      /** Filename. */
      virtual std::string encode() const;

    private:
      /**  */
      std::string _fname;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_FILECAP_H
