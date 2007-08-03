/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/ModaliasCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_MODALIASCAP_H
#define ZYPP_CAPABILITY_MODALIASCAP_H

#include "zypp/base/Deprecated.h"
#include "zypp/capability/CapabilityImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(ModaliasCap)
    
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ModaliasCap
    //
    /** A Capability resolved by a query to target::modalias.
     *
     * \note ModaliasCap is special as it is self evaluating, and does not
     * comapre to the \a rhs (or \a lhs). This is currently solved by
     * treating a ModaliasCap with an empty name as evaluate command.
     *
     * \ref matches returns \c CapMatch::irrelevant, if either both sides
     * are evaluate commands, or both are not.
     *
     * Otherwise the result of the query to target::modalias is returned.
     * Either from \a lhs or \a rhs, dependent on which one is the
     * evaluate command.
    */
    class ModaliasCap : public CapabilityImpl
    {
    public:
      typedef ModaliasCap Self;
      typedef ModaliasCap_Ptr Ptr;
      typedef ModaliasCap_constPtr constPtr;
      
    public:
      /** Ctor */
      ModaliasCap( const Resolvable::Kind & refers_r, const std::string & name_r );

      /** Ctor */
      ModaliasCap( const Resolvable::Kind & refers_r,
                   const std::string & name_r,
                   Rel op_r,
                   const std::string & value_r );

    public:
      /**  */
      virtual const Kind & kind() const;

      /** Query target::Modalias. */
      virtual CapMatch matches( const CapabilityImpl::constPtr & rhs ) const;

      /** <tt>modalias(name) [op value]</tt> */
      virtual std::string encode() const;

      /** <tt>modalias()</tt> */
      virtual std::string index() const;

    public:
      const std::string & pkgname() const
      { return _pkgname; }

      void setPkgname( const std::string &pn )
      { _pkgname = pn; }
      
      ZYPP_DEPRECATED const std::string & querystring() const
      { return _name; }

      const std::string & name() const
      { return _name; }
      
      Rel op() const
      { return _op; }
      
      const std::string & value() const
      { return _value; }
      
    private:
      /** Empty ModaliasCap <tt>modalias()</tt> */
      bool isEvalCmd() const;

      /** Query target::Modalias. */
      bool evaluate() const;

    private:
      /**  */
      std::string _pkgname;
      std::string _name;
      Rel _op;
      std::string _value;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_MODALIASCAP_H
