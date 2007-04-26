/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/HalCap.h
 *
*/
#ifndef ZYPP_CAPABILITY_HALCAP_H
#define ZYPP_CAPABILITY_HALCAP_H

#include "zypp/capability/CapabilityImpl.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    DEFINE_PTR_TYPE(HalCap)
    
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : HalCap
    //
    /** A Capability resolved by a query to target::hal.
     *
     * \note HalCap is special as it is self evaluating, and does not
     * comapre to the \a rhs (or \a lhs). This is currently solved by
     * treating a HalCap with an empty name as evaluate command.
     *
     * \ref matches returns \c CapMatch::irrelevant, if either both sides
     * are evaluate commands, or both are not.
     *
     * Otherwise the result of the query to target::hal is returned.
     * Either from \a lhs or \a rhs, dependent on which one is the
     * evaluate command.
    */
    class HalCap : public CapabilityImpl
    {
    public:
      typedef HalCap Self;
      typedef HalCap_Ptr       Ptr;
      typedef HalCap_constPtr  constPtr;

    public:
      /** Ctor */
      HalCap( const Resolvable::Kind & refers_r, const std::string & name_r )
      : CapabilityImpl( refers_r )
      , _name( name_r )
      {}

      /** Ctor */
      HalCap( const Resolvable::Kind & refers_r,
              const std::string & name_r,
              Rel op_r,
              const std::string & value_r )
      : CapabilityImpl( refers_r )
      , _name( name_r )
      , _op( op_r )
      , _value( value_r )
      {}

    public:
      /**  */
      virtual const Kind & kind() const;

      /** Query target::Hal. */
      virtual CapMatch matches( const CapabilityImpl::constPtr & rhs ) const;

      /** <tt>hal(name) [op value]</tt> */
      virtual std::string encode() const;

      /** <tt>hal(name)</tt> */
      virtual std::string index() const;
      
    public:
      const std::string & name() const
      { return _name; }
      
      Rel op() const
      { return _op; }
      
      const std::string & value() const
      { return _value; }

    private:
      /** Empty HalCap <tt>hal()</tt> */
      bool isEvalCmd() const;

      /** Query target::Hal. */
      bool evaluate() const;

    private:
      /**  */
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
#endif // ZYPP_CAPABILITY_HALCAP_H
