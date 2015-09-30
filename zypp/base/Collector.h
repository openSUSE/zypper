/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Collector.h
 *
*/
#ifndef ZYPP_BASE_COLLECTOR_H
#define ZYPP_BASE_COLLECTOR_H

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace functor
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Collector<TOutputIterator>
  //
  /** Functor feeding values to an output_iterator.
   *
   * \code
   * LocaleSet locales;
   * for_each( begin(), end(),
   *           collector( std::inserter( locales_r, locales_r.begin() ) ) );
   * \endcode
   *
   * \see Convenience constructor \ref collector.
   */
  template<class TOutputIterator>
  struct Collector
  {
    Collector( TOutputIterator iter_r ) : _iter( iter_r ) {}

    template<class Tp>
    bool operator()( const Tp & value_r ) const
    {
      *_iter++ = value_r;
      return true;
    }

    private:
      mutable TOutputIterator _iter;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Collector Convenience constructor. */
  template<class TOutputIterator>
  inline Collector<TOutputIterator> collector( TOutputIterator iter_r )
  { return Collector<TOutputIterator>( iter_r ); }

  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace functor
///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_COLLECTOR_H
