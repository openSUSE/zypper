/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/SetTracker.h
 */
#ifndef ZYPP_BASE_SETTRACKER_H
#define ZYPP_BASE_SETTRACKER_H

#include <iosfwd>
#include <utility>
#include <algorithm>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace base
  {
    ///////////////////////////////////////////////////////////////////
    /// \class SetTracker
    /// \brief Track added/removed set items based on an initial set.
    ///
    /// The class maintains the \ref current set of items and also records
    /// the changes compared to the initial set (\ref added and \ref removed
    /// items) if you use the tracking API.
    ///
    /// It is also possible to directly manipulate the three sets.
    ///
    /// \note The tracking API expects the template arg to have set semantic.
    ///////////////////////////////////////////////////////////////////
    template <class _Set>
    struct SetTracker
    {
      typedef  _Set			set_type;
      typedef typename _Set::key_type	key_type;
      typedef typename _Set::value_type	value_type;

      /** Default Ctor: empty set */
      SetTracker()
      {}

      /** Ctor taking an initial set */
      SetTracker( set_type initial_r )
      : _current( std::move(initial_r) )
      {}

      /// \name Tracking API
      //@{
      /** (Re-)Start tracking the current set (discards previously tracked changes).
       * \return \c False (set did not change)
       */
      bool setInitial()
      { _added.clear(); _removed.clear(); return false; }

      /** Start tracking a new set (discards previously tracked changes).
       * \return Whether the set did change (new!=current)
       */
      bool setInitial( set_type new_r )
      {
	setInitial();
	bool changed = ( new_r != _current );
	if ( changed )
	{
	  _current = std::move(new_r);
	}
	return changed;
      }


      /** Set a \a new_r set and track changes.
       * \return Whether the set has changed
       */
      bool set( set_type new_r )
      {
	bool changed = ( new_r != _current );
	if ( changed )
	{
	  // build the initial (cur-add+rem) set in _current
	  setDifference( _current, _added, _removed );
	  _current.swap( _removed );
	  _added.clear();
	  _removed.clear();

	  const set_type & initial( _current );
	  setDifference( initial, new_r, _removed );
	  setDifference( new_r, initial, _added );
	  _current.swap( new_r );
	}
	return changed;
      }

      /** Add an element to the set and track changes.
       * \return Whether the set has changed
       */
      bool add( const value_type & val_r )
      {
	bool done = _current.insert( val_r ).second;
	if ( done )
	{
	  if ( ! _removed.erase( val_r ) )
	    _added.insert( val_r );
	}
	return done;
      }

      /** Remove an element from the set and track changes.
       * \return Whether the set has changed
       */
      bool remove( const value_type & val_r )
      {
	bool done = _current.erase( val_r );
	if ( done )
	{
	  if ( ! _added.erase( val_r ) )
	    _removed.insert( val_r );
	}
	return done;
      }
      //@}

      /// \name Query and retrieval
      //@{
      /** Whether \a val_r is in the set. */
      bool contains( const key_type & key_r ) const	{ return find( _current, key_r ); }

      /** Whether \a val_r is tracked as added. */
      bool wasAdded( const key_type & key_r ) const	{ return find( _added, key_r ); }

      /** Whether \a val_r is tracked as removed. */
      bool wasRemoved( const key_type & key_r ) const	{ return find( _removed, key_r ); }


      /** Return the current set. */
      const set_type & current() const			{ return _current; }

      /** Return the set of added items. */
      const set_type & added() const			{ return _added; }

      /** Return the set of removed items. */
      const set_type & removed() const			{ return _removed; }
      //@}

      /// \name Direct manipulation
      //@{
      /** Return the current set. */
      set_type & current()				{ return _current; }

      /** Return the set of added items. */
      set_type & added()				{ return _added; }

      /** Return the set of removed items. */
      set_type & removed() 				{ return _removed; }
      //@}

    private:

      static bool find( const set_type & set_r, const key_type & key_r )
      { return set_r.find( key_r ) != set_r.end(); }

      template <class _ORDERED_SET, typename enable_if = typename _ORDERED_SET::key_compare>
      static void setDifference( const _ORDERED_SET & lhs, const _ORDERED_SET & rhs, _ORDERED_SET & result_r )
      {
	// std::set_difference requires ordered sets!
	std::set_difference( lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
			     std::inserter( result_r, result_r.end() ),
			     typename _ORDERED_SET::key_compare() );
      }

      template <class _UNORDERED_SET, typename enable_if = typename _UNORDERED_SET::hasher, typename = void>
      static void setDifference( const _UNORDERED_SET & lhs, const _UNORDERED_SET & rhs, _UNORDERED_SET & result_r )
      {
	// std::set_difference requires ordered sets!
	for ( const auto & l : lhs )
	{ if ( rhs.find( l ) == rhs.end() ) result_r.insert( l ); }
      }

    private:
      set_type _current;
      set_type _added;
      set_type _removed;
    };

    /** \relates SetTracker Stream output */
    template <class _Set>
    std::ostream & operator<<( std::ostream & str, const SetTracker<_Set> & obj )
    { return str << "set(" << obj.current().size() << "|+" << obj.added().size() << "|-" << obj.removed().size() << ')'; }

  } // namespace base
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_SETTRACKER_H
