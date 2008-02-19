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
  //	CLASS NAME : _Collector<_OutputIterator>
  //
  /** Functor feeding values to an output_iterator.
   *
   * \code
   * LocaleSet locales;
   * for_each( begin(), end(),
   *           Collector( std::inserter( locales_r, locales_r.begin() ) ) );
   * \endcode
   *
   * \see Convenience constructor \ref Collector.
   */
  template<class _OutputIterator>
  struct _Collector
  {
    _Collector( _OutputIterator iter_r ) : _iter( iter_r ) {}

    template<class _Tp>
    bool operator()( const _Tp & value_r ) const
    {
      *_iter++ = value_r;
      return true;
    }

    private:
      mutable _OutputIterator _iter;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates _Collector Convenience constructor. */
  template<class _OutputIterator>
  inline _Collector<_OutputIterator> Collector( _OutputIterator iter_r )
  { return _Collector<_OutputIterator>( iter_r ); }

  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace functor
///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_COLLECTOR_H
