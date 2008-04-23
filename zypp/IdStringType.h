/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/IdStringType.h
 *
*/
#ifndef ZYPP_IDSTRINGTYPE_H
#define ZYPP_IDSTRINGTYPE_H

#include "zypp/IdString.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : IdStringType<Derived>
  //
  /** Base class for creating \ref IdString based types.
   *
   * Just by deriving from \ref IdStringType a class provides all
   * the operations an \ref IdString does. (incl. conversion to string types,
   * comparison of string types and stream output).
   *
   * To disable any comparison, declare (but do not define) \ref _doCompare
   * in your class.
   * \code
   * class NoCompare : public IdStringType<NoCompare>
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
   *    class CaseCmp : public IdStringType<CaseCmp>
   *    {
   *      public:
   *        CaseCmp() {}
   *        explicit CaseCmp( const char * cstr_r ) : _str( cstr_r )  {}
   *      private:
   *        friend class IdStringType<CaseCmp>;
   *        IdString _str;
   *    };
   *
   *    // uses case insensitive comparison order
   *    class NoCaseCmp : public IdStringType<NoCaseCmp>
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
   *        friend class IdStringType<NoCaseCmp>;
   *        IdString _str;
   *    };
   *
   *    CaseCmp   ca( "a" );
   *    NoCaseCmp na( "a" );
   *    DBG << "ca == a ? " << (ca == "a") << endl;   // ca == a ? 1
   *    DBG << "ca == A ? " << (ca == "A") << endl;   // ca == A ? 0
   *    DBG << "na == a ? " << (na == "a") << endl;   // na == a ? 1
   *    DBG << "na == A ? " << (na == "A") << endl;   // na == A ? 1
   * \endcode
   * \todo allow redefinition of order vis _doCompare not only for char* but on any level
   * \ingroup g_CRTP
   */
  template <class Derived>
  class IdStringType : protected sat::detail::PoolMember,
                       private base::SafeBool<Derived>
  {
    typedef typename base::SafeBool<Derived>::bool_type bool_type;

    protected:
      IdStringType() {}
      IdStringType(const IdStringType &) {}
      void operator=(const IdStringType &) {}
      ~IdStringType() {}

    private:
      const Derived & self() const { return *static_cast<const Derived*>( this ); }

    public:
      const IdString & idStr()    const { return self()._str; }

      bool          empty()       const { return idStr().empty(); }
      unsigned      size()        const { return idStr().size(); }
      const char *  c_str()       const { return idStr().c_str(); }
      std::string   asString()    const { return idStr().asString(); }

      IdString::IdType id()       const { return idStr().id(); }

    public:
      /** Evaluate in a boolean context <tt>( ! empty() )</tt>. */
      using base::SafeBool<Derived>::operator bool_type;

    public:
      static int compare( const Derived & lhs,    const Derived & rhs )      { return compare( lhs.idStr(), rhs.idStr() ); }
      static int compare( const Derived & lhs,    const IdString & rhs )     { return compare( lhs.idStr(), rhs ); }
      static int compare( const Derived & lhs,    const std::string & rhs )  { return Derived::_doCompare( lhs.c_str(), rhs.c_str() ); }
      static int compare( const Derived & lhs,    const char * rhs )         { return Derived::_doCompare( lhs.c_str(), rhs );}

      static int compare( const IdString & lhs,   const Derived & rhs )      { return compare( lhs, rhs.idStr() ); }
      static int compare( const IdString & lhs,   const IdString & rhs )     { return lhs.compareEQ( rhs ) ? 0 :
                                                                                      Derived::_doCompare( lhs.c_str(), rhs.c_str() ); }
      static int compare( const IdString & lhs,   const std::string & rhs )  { return Derived::_doCompare( lhs.c_str(), rhs.c_str() ); }
      static int compare( const IdString & lhs,   const char * rhs )         { return Derived::_doCompare( lhs.c_str(), rhs ); }

      static int compare( const std::string & lhs, const Derived & rhs )     { return Derived::_doCompare( lhs.c_str(), rhs.c_str() );}
      static int compare( const std::string & lhs, const IdString & rhs )    { return Derived::_doCompare( lhs.c_str(), rhs.c_str() ); }
      static int compare( const std::string & lhs, const std::string & rhs ) { return Derived::_doCompare( lhs.c_str(), rhs.c_str() ); }
      static int compare( const std::string & lhs, const char * rhs )        { return Derived::_doCompare( lhs.c_str(), rhs ); }

      static int compare( const char * lhs,        const Derived & rhs )     { return Derived::_doCompare( lhs, rhs.c_str() );}
      static int compare( const char * lhs,        const IdString & rhs )    { return Derived::_doCompare( lhs, rhs.c_str() ); }
      static int compare( const char * lhs,        const std::string & rhs ) { return Derived::_doCompare( lhs, rhs.c_str() ); }
      static int compare( const char * lhs,        const char * rhs )        { return Derived::_doCompare( lhs, rhs ); }

    public:
      int compare( const Derived & rhs )      const { return compare( idStr(), rhs.idStr() ); }
      int compare( const IdStringType & rhs ) const { return compare( idStr(), rhs.idStr() ); }
      int compare( const IdString & rhs )     const { return compare( idStr(), rhs ); }
      int compare( const std::string & rhs )  const { return Derived::_doCompare( c_str(), rhs.c_str() ); }
      int compare( const char * rhs )         const { return Derived::_doCompare( c_str(), rhs ); }

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

  /** \relates IdStringType Stream output */
  template <class Derived>
  inline std::ostream & operator<<( std::ostream & str, const IdStringType<Derived> & obj )
  { return str << obj.c_str(); }

  /** \relates IdStringType Equal */
  template <class Derived>
  inline bool operator==( const IdStringType<Derived> & lhs, const IdStringType<Derived> & rhs )
  { return lhs.idStr().compareEQ( rhs.idStr() ); }
  /** \overload */
  template <class Derived>
  inline bool operator==( const IdStringType<Derived> & lhs, const IdString & rhs )
  { return lhs.compare( rhs ) == 0; }
  /** \overload */
  template <class Derived>
  inline bool operator==( const IdStringType<Derived> & lhs, const char * rhs )
  { return lhs.compare( rhs ) == 0; }
  /** \overload */
  template <class Derived>
  inline bool operator==( const IdStringType<Derived> & lhs, const std::string & rhs )
  { return lhs.compare( rhs ) == 0; }
  /** \overload */
  template <class Derived>
  inline bool operator==( const IdString & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) == 0; }
  /** \overload */
  template <class Derived>
  inline bool operator==( const char * lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) == 0; }
  /** \overload */
  template <class Derived>
  inline bool operator==( const std::string & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) == 0; }

  /** \relates IdStringType NotEqual */
  template <class Derived>
  inline bool operator!=( const IdStringType<Derived> & lhs, const IdStringType<Derived> & rhs )
  { return ! lhs.idStr().compareEQ( rhs.idStr() ); }
  /** \overload */
  template <class Derived>
  inline bool operator!=( const IdStringType<Derived> & lhs, const IdString & rhs )
  { return lhs.compare( rhs ) != 0; }
  /** \overload */
  template <class Derived>
  inline bool operator!=( const IdStringType<Derived> & lhs, const char * rhs )
  { return lhs.compare( rhs ) != 0; }
  /** \overload */
  template <class Derived>
  inline bool operator!=( const IdStringType<Derived> & lhs, const std::string & rhs )
  { return lhs.compare( rhs ) != 0; }
  /** \overload */
  template <class Derived>
  inline bool operator!=( const IdString & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) != 0; }
  /** \overload */
  template <class Derived>
  inline bool operator!=( const char * lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) != 0; }
  /** \overload */
  template <class Derived>
  inline bool operator!=( const std::string & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) != 0; }

  /** \relates IdStringType Less */
  template <class Derived>
  inline bool operator<( const IdStringType<Derived> & lhs, const IdStringType<Derived> & rhs )
  { return lhs.compare( rhs ) < 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<( const IdStringType<Derived> & lhs, const IdString & rhs )
  { return lhs.compare( rhs ) < 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<( const IdStringType<Derived> & lhs, const char * rhs )
  { return lhs.compare( rhs ) < 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<( const IdStringType<Derived> & lhs, const std::string & rhs )
  { return lhs.compare( rhs ) < 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<( const IdString & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) >= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<( const char * lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) >= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<( const std::string & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) >= 0; }

  /** \relates IdStringType LessEqual */
  template <class Derived>
  inline bool operator<=( const IdStringType<Derived> & lhs, const IdStringType<Derived> & rhs )
  { return lhs.compare( rhs ) <= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<=( const IdStringType<Derived> & lhs, const IdString & rhs )
  { return lhs.compare( rhs ) <= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<=( const IdStringType<Derived> & lhs, const char * rhs )
  { return lhs.compare( rhs ) <= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<=( const IdStringType<Derived> & lhs, const std::string & rhs )
  { return lhs.compare( rhs ) <= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<=( const IdString & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) > 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<=( const char * lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) > 0; }
  /** \overload */
  template <class Derived>
  inline bool operator<=( const std::string & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) > 0; }

  /** \relates IdStringType Greater */
  template <class Derived>
  inline bool operator>( const IdStringType<Derived> & lhs, const IdStringType<Derived> & rhs )
  { return lhs.compare( rhs ) > 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>( const IdStringType<Derived> & lhs, const IdString & rhs )
  { return lhs.compare( rhs ) > 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>( const IdStringType<Derived> & lhs, const char * rhs )
  { return lhs.compare( rhs ) > 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>( const IdStringType<Derived> & lhs, const std::string & rhs )
  { return lhs.compare( rhs ) > 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>( const IdString & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) <= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>( const char * lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) <= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>( const std::string & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) <= 0; }

  /** \relates IdStringType GreaterEqual */
  template <class Derived>
  inline bool operator>=( const IdStringType<Derived> & lhs, const IdStringType<Derived> & rhs )
  { return lhs.compare( rhs ) >= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>=( const IdStringType<Derived> & lhs, const IdString & rhs )
  { return lhs.compare( rhs ) >= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>=( const IdStringType<Derived> & lhs, const char * rhs )
  { return lhs.compare( rhs ) >= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>=( const IdStringType<Derived> & lhs, const std::string & rhs )
  { return lhs.compare( rhs ) >= 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>=( const IdString & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) < 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>=( const char * lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) < 0; }
  /** \overload */
  template <class Derived>
  inline bool operator>=( const std::string & lhs, const IdStringType<Derived> & rhs )
  { return rhs.compare( lhs ) < 0; }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_IDSTRINGTYPE_H
