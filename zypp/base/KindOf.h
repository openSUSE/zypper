/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/KindOf.h
 *
*/
#ifndef ZYPP_BASE_KINDOF_H
#define ZYPP_BASE_KINDOF_H

#include <iosfwd>
#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : KindOf<_Tp>
    //
    /**
    */
    template<class _Tp>
      class KindOf
      {
      public:
        /** Ctor */
        KindOf()
        {}
        /** Ctor */
        explicit
        KindOf( const std::string & value_r )
        : _value( value_r )
        {}
        /** Dtor */
        ~KindOf()
        {}
      public:
        /** */
        const std::string & asString() const
        { return _value; }
      private:
        std::string _value;
      };
    ///////////////////////////////////////////////////////////////////

    template<class _Tp>
      inline std::ostream & operator<<( std::ostream & str, const KindOf<_Tp> & obj )
      { return str << obj.asString(); }

    ///////////////////////////////////////////////////////////////////

    template<class _Tp>
      inline bool operator==( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return lhs.asString() == rhs.asString(); }

    template<class _Tp>
      inline bool operator==( const KindOf<_Tp> & lhs, const std::string & rhs )
      { return lhs.asString() == rhs; }

    template<class _Tp>
      inline bool operator==( const std::string & lhs, const KindOf<_Tp> & rhs )
      { return lhs == rhs.asString(); }


    template<class _Tp>
      inline bool operator!=( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return !( lhs == rhs ); }

    template<class _Tp>
      inline bool operator!=( const KindOf<_Tp> & lhs, const std::string & rhs )
      { return !( lhs == rhs ); }

    template<class _Tp>
      inline bool operator!=( const std::string & lhs, const KindOf<_Tp> & rhs )
      { return !( lhs == rhs ); }


    template<class _Tp>
      inline bool operator<( const KindOf<_Tp> & lhs, const KindOf<_Tp> & rhs )
      { return lhs.asString() < rhs.asString(); }

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_KINDOF_H
