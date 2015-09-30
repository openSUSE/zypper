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
     * The only assertion is that \c 0 is not used as an Id,
     * \b unless the derived class explicitly requests this by
     * using \ref ProvideNumericId( const void *const ).
     *
     * Why should you want to use \c 0 as an Id? E.g if your class
     * provides some (singleton) No-object. Might be desirable to
     * make the No-object have No-Id.
     *
     * \code
     * struct Foo : public base::ProvideNumericId<Foo,unsigned>
     * {};
     * Foo foo;
     * foo.numericId(); // returns foo's NumericId.
     * \endcode
    */
    template<class TDerived, class TNumericIdType>
      struct ProvideNumericId
      {
      public:
        /** \return The objects numeric Id. */
        TNumericIdType numericId() const
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
      protected:
        /** No-Id ctor (0).
         * Explicitly request Id \c 0. Use it with care!
        */
        ProvideNumericId( const void *const )
        : _numericId( 0 )
        {}
      private:
        /** Provide the next Id to use. */
        static TNumericIdType nextId()
        {
          static TNumericIdType _staticCounter = 0;
          // Assert not returning 0
          return ++_staticCounter;
        }
        /**  */
        const TNumericIdType _numericId;
      };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_PROVIDENUMERICID_H
