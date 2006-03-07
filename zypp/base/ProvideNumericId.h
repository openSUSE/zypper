/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/ProvideNumericId.h
 *
*/
#ifndef ZYPP_BASE_PROVIDENUMERICID_H
#define ZYPP_BASE_PROVIDENUMERICID_H

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProvideNumericId
    //
    /** Base class for objects providing a numeric Id.
     * The ctor creates a NumericId from some static counter.
     *
     * \code
     * struct Foo : public base::ProvideNumericId<Foo,unsigned>
     * {};
     * Foo foo;
     * foo.numericId(); // returns foo's NumericId.
     * \endcode
    */
    template<class _Derived, class _NumericIdType>
      struct ProvideNumericId
      {
      public:
        /** \return The objects numeric Id. */
        _NumericIdType numericId() const
        { return _numericId; }

      protected:
        /** Default ctor */
        ProvideNumericId()
        : _numericId( nextId() )
        {}
        /** Copy ctor */
        ProvideNumericId( const ProvideNumericId & /*rhs*/ )
        : _numericId( nextId() )
        {}
        /** Assign */
        ProvideNumericId & operator=( const ProvideNumericId & /*rhs*/ )
        { return *this; }
        /** Dtor */
        ~ProvideNumericId()
        {}
      private:
        /**  */
        static _NumericIdType nextId()
        {
          static _NumericIdType _staticCounter = 0;
          return ++_staticCounter;
        }
        /**  */
        const _NumericIdType _numericId;
      };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_PROVIDENUMERICID_H
