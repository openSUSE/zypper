/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/IdStrType.h
 *
*/
#ifndef ZYPP_SAT_IDSTRTYPE_H
#define ZYPP_SAT_IDSTRTYPE_H

#include "zypp/sat/IdStr.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : IdStrType<Derived>
    //
    /** Base class for creating \ref IdStr based types.
     *
     * Just by deriving from \ref IdStrType a class provides all
     * the operations an \ref IdStr does. (incl. conversion to string types,
     * comparison with string types and stream output).
     *
     * To disable any comparison, declare (but do not define) \ref _doCompareC
     * in your class. If you need a different than the default lexicographical
     * order, write your own \ref _doCompareC. If you can provide optimized
     * comparison against IdStr or your class itself, \b additionally provide
     * _doCompareI, and/or _doCompareD.
     *
     * \code
     *    class CaseCmp : public sat::IdStrType<CaseCmp>
     *    {
     *      public:
     *        CaseCmp() {}
     *        explicit CaseCmp( const char * cstr_r ) : _str( cstr_r )  {}
     *      private:
     *        friend class sat::IdStrType<CaseCmp>;
     *        sat::IdStr _str;
     *    };
     *
     *    class NoCaseCmp : public sat::IdStrType<NoCaseCmp>
     *    {
     *      public:
     *        NoCaseCmp() {}
     *        explicit NoCaseCmp( const char * cstr_r ) : _str( cstr_r )  {}
     *      private:
     *        int _doCompareC( const char * rhs )  const
     *        { return ::strcasecmp( _str.c_str(), rhs ); }
     *      private:
     *        friend class sat::IdStrType<NoCaseCmp>;
     *        sat::IdStr _str;
     *    };
     *
     *    CaseCmp   ca( "a" );
     *    NoCaseCmp na( "a" );
     *    DBG << "ca == a ? " << (ca == "a") << endl;   // ca == a ? 1
     *    DBG << "ca == A ? " << (ca == "A") << endl;   // ca == A ? 0
     *    DBG << "na == a ? " << (na == "a") << endl;   // na == a ? 1
     *    DBG << "na == A ? " << (na == "A") << endl;   // na == A ? 1
     * \endcode
     * \ingroup g_CRTP
    */
    template <class Derived>
    class IdStrType : private base::SafeBool<Derived>
    {
      typedef typename base::SafeBool<Derived>::bool_type bool_type;

      protected:
        IdStrType() {}
        IdStrType(const IdStrType &) {}
        void operator=(const IdStrType &) {}

      private:
        const Derived & self() const { return *static_cast<const Derived*>( this ); }

      public:
        const IdStr & idStr()       const { return self()._str; }

        bool          empty()       const { return idStr().empty(); }
        unsigned      size()        const { return idStr().size(); }
        const char *  c_str()       const { return idStr().c_str(); }
        std::string   string()      const { return idStr().string(); }
        std::string   asString()    const { return idStr().asString(); }

      public:
        /** Evaluate in a boolean context <tt>( ! empty() )</tt>. */
        using base::SafeBool<Derived>::operator bool_type;

      public:
        int compare( const Derived & rhs )     const { return self()._doCompareD( rhs ); }
        int compare( const IdStrType & rhs )   const { return self()._doCompareD( rhs.self() ); }
        int compare( const IdStr & rhs )       const { return self()._doCompareI( rhs ); }
        int compare( const char * rhs )        const { return self()._doCompareC( rhs ); }
        int compare( const std::string & rhs ) const { return self()._doCompareC( rhs.c_str() ); }

      private:
        int _doCompareD( const Derived & rhs ) const { return self()._doCompareI( rhs.idStr() ); }
        int _doCompareI( const IdStr & rhs )   const { return idStr().compareEQ( rhs ) ? 0 : self()._doCompareC( rhs.c_str() ); }
        int _doCompareC( const char * rhs )    const { return idStr().compare( rhs ); }

        friend base::SafeBool<Derived>::operator bool_type() const;
        bool boolTest() const { return ! empty(); }
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates IdStrType Stream output */
    template <class Derived>
    inline std::ostream & operator<<( std::ostream & str, const IdStrType<Derived> & obj )
    { return str << obj.c_str(); }

    /** \relates IdStrType Equal */
    template <class Derived>
    inline bool operator==( const IdStrType<Derived> & lhs, const IdStrType<Derived> & rhs )
    { return lhs.idStr().compareEQ( rhs.idStr() ); }
    /** \overload */
    template <class Derived>
    inline bool operator==( const IdStrType<Derived> & lhs, const IdStr & rhs )
    { return lhs.compare( rhs ) == 0; }
    /** \overload */
    template <class Derived>
    inline bool operator==( const IdStrType<Derived> & lhs, const char * rhs )
    { return lhs.compare( rhs ) == 0; }
    /** \overload */
    template <class Derived>
    inline bool operator==( const IdStrType<Derived> & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) == 0; }
    /** \overload */
    template <class Derived>
    inline bool operator==( const IdStr & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) == 0; }
    /** \overload */
    template <class Derived>
    inline bool operator==( const char * lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) == 0; }
    /** \overload */
    template <class Derived>
    inline bool operator==( const std::string & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) == 0; }

    /** \relates IdStrType NotEqual */
    template <class Derived>
    inline bool operator!=( const IdStrType<Derived> & lhs, const IdStrType<Derived> & rhs )
    { return lhs.idStr().compareEQ( rhs.idStr() ); }
    /** \overload */
    template <class Derived>
    inline bool operator!=( const IdStrType<Derived> & lhs, const IdStr & rhs )
    { return lhs.compare( rhs ) != 0; }
    /** \overload */
    template <class Derived>
    inline bool operator!=( const IdStrType<Derived> & lhs, const char * rhs )
    { return lhs.compare( rhs ) != 0; }
    /** \overload */
    template <class Derived>
    inline bool operator!=( const IdStrType<Derived> & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) != 0; }
    /** \overload */
    template <class Derived>
    inline bool operator!=( const IdStr & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) != 0; }
    /** \overload */
    template <class Derived>
    inline bool operator!=( const char * lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) != 0; }
    /** \overload */
    template <class Derived>
    inline bool operator!=( const std::string & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) != 0; }

    /** \relates IdStrType Less */
    template <class Derived>
    inline bool operator<( const IdStrType<Derived> & lhs, const IdStrType<Derived> & rhs )
    { return lhs.compare( rhs ) < 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<( const IdStrType<Derived> & lhs, const IdStr & rhs )
    { return lhs.compare( rhs ) < 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<( const IdStrType<Derived> & lhs, const char * rhs )
    { return lhs.compare( rhs ) < 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<( const IdStrType<Derived> & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) < 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<( const IdStr & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) >= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<( const char * lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) >= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<( const std::string & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) >= 0; }

    /** \relates IdStrType LessEqual */
    template <class Derived>
    inline bool operator<=( const IdStrType<Derived> & lhs, const IdStrType<Derived> & rhs )
    { return lhs.compare( rhs ) <= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<=( const IdStrType<Derived> & lhs, const IdStr & rhs )
    { return lhs.compare( rhs ) <= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<=( const IdStrType<Derived> & lhs, const char * rhs )
    { return lhs.compare( rhs ) <= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<=( const IdStrType<Derived> & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) <= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<=( const IdStr & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) > 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<=( const char * lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) > 0; }
    /** \overload */
    template <class Derived>
    inline bool operator<=( const std::string & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) > 0; }

    /** \relates IdStrType Greater */
    template <class Derived>
    inline bool operator>( const IdStrType<Derived> & lhs, const IdStrType<Derived> & rhs )
    { return lhs.compare( rhs ) > 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>( const IdStrType<Derived> & lhs, const IdStr & rhs )
    { return lhs.compare( rhs ) > 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>( const IdStrType<Derived> & lhs, const char * rhs )
    { return lhs.compare( rhs ) > 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>( const IdStrType<Derived> & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) > 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>( const IdStr & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) <= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>( const char * lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) <= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>( const std::string & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) <= 0; }

    /** \relates IdStrType GreaterEqual */
    template <class Derived>
    inline bool operator>=( const IdStrType<Derived> & lhs, const IdStrType<Derived> & rhs )
    { return lhs.compare( rhs ) >= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>=( const IdStrType<Derived> & lhs, const IdStr & rhs )
    { return lhs.compare( rhs ) >= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>=( const IdStrType<Derived> & lhs, const char * rhs )
    { return lhs.compare( rhs ) >= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>=( const IdStrType<Derived> & lhs, const std::string & rhs )
    { return lhs.compare( rhs ) >= 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>=( const IdStr & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) < 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>=( const char * lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) < 0; }
    /** \overload */
    template <class Derived>
    inline bool operator>=( const std::string & lhs, const IdStrType<Derived> & rhs )
    { return rhs.compare( lhs ) < 0; }

   /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_IDSTRTYPE_H
