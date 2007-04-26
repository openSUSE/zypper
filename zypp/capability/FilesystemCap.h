/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/FilesystemCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_FILESYSTEMCAP_H
#define ZYPP_CAPABILITY_FILESYSTEMCAP_H

#include "zypp/capability/CapabilityImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(FilesystemCap)
    
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : FilesystemCap
    //
    /** A Capability resolved by a query to /etc/sysconfig/storage:USED_FS_LIST
     *
     * \note FilesystemCap is special as it is self evaluating, and does not
     * comapre to the \a rhs (or \a lhs). This is currently solved by
     * treating a FilesystemCap with an empty name as evaluate command.
     *
     * \ref matches returns \c CapMatch::irrelevant, if either both sides
     * are evaluate commands, or both are not.
     *
     * Otherwise the result of the query /etc/sysconfig/storage:USED_FS_LIST
     * is returned. Either from \a lhs or \a rhs, dependent on which one is the
     * evaluate command.
    */
    class FilesystemCap : public CapabilityImpl
    {
    public:
      typedef FilesystemCap Self;
      typedef FilesystemCap_Ptr Ptr;
      typedef FilesystemCap_constPtr constPtr;
      
    public:
      /** Ctor */
      FilesystemCap( const Resolvable::Kind & refers_r, const std::string & name_r );

    public:
      /**  */
      virtual const Kind & kind() const;

      /** Query USED_FS_LIST. */
      virtual CapMatch matches( const CapabilityImpl::constPtr & rhs ) const;

      /** <tt>filesystem(name)</tt> */
      virtual std::string encode() const;

      /** <tt>filesystem()</tt> */
      virtual std::string index() const;

    public:
      const std::string & name() const
      { return _name; }

    private:
      /** Empty FilesystemCap <tt>filesystem()</tt> */
      bool isEvalCmd() const;

      /** Query USED_FS_LIST. */
      bool evaluate() const;

    private:
      /**  */
      std::string _name;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_FILESYSTEMCAP_H
