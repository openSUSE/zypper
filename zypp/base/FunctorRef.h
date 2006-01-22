/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/FunctorRef.h
 *
*/
#ifndef ZYPP_BASE_FUNCTORREF_H
#define ZYPP_BASE_FUNCTORREF_H

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace functor
  { /////////////////////////////////////////////////////////////////

    /** An unary functor forwarding to some other <tt>_Functor &</tt>.
     * \ingroup g_Functor
     *
     * Most algorithms take functor arguments by value. That's inconvenient
     * if the functor wants to collect and return data. Creating and
     * passing a \ref FunctorRef to the algorithm, may help you out of this.
     *
     * \code
     *   // Counts invokations of operator().
     *   struct Counter
     *   {
     *     template<class _Tp>
     *       void operator()( Tp )
     *       { ++_value; }
     *
     *     Counter() : _value( 0 ) {}
     *     unsigned _value;
     *   };
     *
     *   Counter counter;
     *   // Invokations of FunctorRef are forwarded to counter:
     *   std::for_each( c.begin, c.end(), functorRef(counter) );
    * \endcode
    */
    template <class _Functor>
      class FunctorRef : public std::unary_function<typename _Functor::argument_type,
                                                    typename _Functor::result_type>
    {
    public:
      FunctorRef( _Functor & f_r )
      : _f( f_r )
      {}

      result_type operator()( argument_type a1 ) const
      {
        return _f.operator()( a1 );
      }

    private:
      _Functor & _f;
    };

    /** Convenience function creating a \ref FunctorRef. */
    template <class _Functor>
      FunctorRef<_Functor> functorRef( _Functor & f_r )
      { return FunctorRef<_Functor>( f_r ); }

    /////////////////////////////////////////////////////////////////
  } // namespace functor
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_FUNCTORREF_H
