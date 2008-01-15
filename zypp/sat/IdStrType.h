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
     * comparison of string types and stream output).
     *
     * To disable any comparison, declare (but do not define) \ref _doCompare
     * in your class.
     * \code
     * class NoCompare : : public sat::IdStrType<NoCompare>
     * {
     *   private:
      *   static int _doCompare( const char * lhs,  const char * rhs );
     *
     * };
     * \endcode
     *
     * If you need a different than the default lexicographical
     * order, write your own \ref _doCompare.
     *
     * \code
     *    // uses default lexicographical order
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
     *    // uses case insensitive comparison order
     *    class NoCaseCmp : public sat::IdStrType<NoCaseCmp>
     *    {
     *      public:
     *        NoCaseCmp() {}
     *        explicit NoCaseCmp( const char * cstr_r ) : _str( cstr_r )  {}
     *      private:
     *        static int _doCompare( const char * lhs,  const char * rhs )
     *        {
     *          if ( lhs == rhs ) return 0;
     *          if ( lhs && rhs ) return ::strcasecmp( lhs, rhs );
     *          return( lhs ? 1 : -1 );
     *        }
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
        static int compare( const Derived & lhs,    const Derived & rhs )      { return compare( lhs.idStr(), rhs.idStr() ); }
        static int compare( const Derived & lhs,    const IdStr & rhs )        { return compare( lhs.idStr(), rhs ); }
        static int compare( const Derived & lhs,    const std::string & rhs )  { return Derived::_doCompare( lhs.c_str(), rhs.c_str() ); }
        static int compare( const Derived & lhs,    const char * rhs )         { return Derived::_doCompare( lhs.c_str(), rhs );}

        static int compare( const IdStr & lhs,       const Derived & rhs )     { return compare( lhs, rhs.idStr() ); }
        static int compare( const IdStr & lhs,       const IdStr & rhs )       { return lhs.compareEQ( rhs ) ? 0 :
                                                                                        Derived::_doCompare( lhs.c_str(), rhs.c_str() ); }
        static int compare( const IdStr & lhs,       const std::string & rhs ) { return Derived::_doCompare( lhs.c_str(), rhs.c_str() ); }
        static int compare( const IdStr & lhs,       const char * rhs )        { return Derived::_doCompare( lhs.c_str(), rhs ); }

        static int compare( const std::string & lhs, const Derived & rhs )     { return Derived::_doCompare( lhs.c_str(), rhs.c_str() );}
        static int compare( const std::string & lhs, const IdStr & rhs )       { return Derived::_doCompare( lhs.c_str(), rhs.c_str() ); }
        static int compare( const std::string & lhs, const std::string & rhs ) { return Derived::_doCompare( lhs.c_str(), rhs.c_str() ); }
        static int compare( const std::string & lhs, const char * rhs )        { return Derived::_doCompare( lhs.c_str(), rhs ); }

        static int compare( const char * lhs,        const Derived & rhs )     { return Derived::_doCompare( lhs, rhs.c_str() );}
        static int compare( const char * lhs,        const IdStr & rhs )       { return Derived::_doCompare( lhs, rhs.c_str() ); }
        static int compare( const char * lhs,        const std::string & rhs ) { return Derived::_doCompare( lhs, rhs.c_str() ); }
        static int compare( const char * lhs,        const char * rhs )        { return Derived::_doCompare( lhs, rhs ); }

      public:
        int compare( const Derived & rhs )     const { return compare( idStr(), rhs.idStr() ); }
        int compare( const IdStrType & rhs )   const { return compare( idStr(), rhs.idStr() ); }
        int compare( const IdStr & rhs )       const { return compare( idStr(), rhs ); }
        int compare( const std::string & rhs ) const { return Derived::_doCompare( c_str(), rhs.c_str() ); }
        int compare( const char * rhs )        const { return Derived::_doCompare( c_str(), rhs ); }

      private:
        static int _doCompare( const char * lhs,  const char * rhs )
        {
          if ( lhs == rhs ) return 0;
          if ( lhs && rhs ) return ::strcmp( lhs, rhs );
          return( lhs ? 1 : -1 );
        }

      private:

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
