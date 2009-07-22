/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/InstanceId.h
 *
*/
#ifndef ZYPP_INSTANCEID_H
#define ZYPP_INSTANCEID_H

#include <string>

#include "zypp/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : InstanceId
  //
  /**
   * Build string to identify/retrieve a specific \a Solvable.
   *
   * <tt>"[<namespace>:]<name>-<version>-<release>.<arch>@<repoalias>"</tt>
   *
   * Any namespace that prepends the InstanceIds must be
   * passed to the ctor. Conversion to/from instanceId can
   * be done via function call \c operator().
   *
   * \code
   *   InstanceId instanceId( "SUSE:" ); // using a namespace
   *
   *   ResPool pool( ResPool::instance() );
   *   for_( it, pool.begin(), pool.end() )
   *   {
   *     std::cout << instanceId(*it) << endl;
   *   }
   * \endcode
   */
  class InstanceId
  {
    public:
      /** Default ctor empty empty namespace */
      InstanceId()
      {}

      /** Ctor taking namespace */
      InstanceId( const std::string & namespace_r )
      : _namespace( namespace_r )
      {}

    public:
      /** \ref Solvable to \ref InstanceId string. */
      std::string getIdFor( sat::Solvable slv_r ) const;
      /** \ref PoolItem to \ref InstanceId string. */
      std::string getIdFor( const PoolItem & pi_r ) const
      { return getIdFor( pi_r.satSolvable() ); }

      /** \ref InstanceId string to \ref Solvable. */
      sat::Solvable findSolvable( const std::string str_r ) const
      { return findPoolItem( str_r ).satSolvable(); }
      /** \ref InstanceId string to \ref PoolItem. */
      PoolItem findPoolItem( const std::string str_r ) const;

    public:
      /** \ref Solvable to \ref InstanceId string. */
      std::string operator()( sat::Solvable slv_r ) const
      { return getIdFor( slv_r ); }

      /** \ref PoolItem to \ref InstanceId string. */
      std::string operator()( const PoolItem & pi_r ) const
      { return getIdFor( pi_r ); }

      /** \ref InstanceId string to \ref PoolItem. */
      PoolItem operator()( const std::string str_r ) const
      { return findPoolItem( str_r ); }

      /** Quick test whether the InstanceId string would refer
       * to a system (installed) Solvable. */
      bool isSystemId( const std::string str_r ) const;

    public:
      /** The namespace in use. */
      const std::string & getNamespace() const
      { return _namespace; }

      /** Set a new namespace. */
      void setNamespace( const std::string & namespace_r )
      { _namespace = namespace_r; }

      /** Set no (empty) namespace. */
      void unsetNamespace()
      { _namespace.clear(); }

   private:
      std::string _namespace;
  };
  /////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_INSTANCEID_H
