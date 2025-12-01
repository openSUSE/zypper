#include <iostream>
#include <unordered_map>

#include "Parser.h"

#include <zypp/ResPool.h>
#include <zypp/base/Exception.h>
#include <zypp/misc/DefaultLoadSystem.h>

using namespace zypp;

//////////////////////////////////////////////////////////////////
namespace zypp::qf
{
  template <class ItemT>
  using AttrRenderer=std::function<void(const ItemT &,std::ostream &)>;

#define TAG MOUT << __PRETTY_FUNCTION__ << endl

  template <class T>
  void OUTS( T && t ) {
    MOUT << __PRETTY_FUNCTION__ << endl;
  }

  namespace
  {
    //////////////////////////////////////////////////////////////////

    struct TokenData
    {
      TokenData() {}
      virtual ~TokenData() {}

      virtual bool empty() const
      { return true; }

      virtual std::size_t size() const
      { return 0; }

      virtual std::string_view get() const
      { return "(none)"; }
    };

    template <class ItemT>
    struct TokenGetter : public TokenData
    {
      virtual void load( const ItemT & item_r )
      {}
    };

    //////////////////////////////////////////////////////////////////

    template <class ItemT>
    struct DataGetterBase
    {
      using TokenGetterT = TokenGetter<ItemT>;
      using TokenGetterPtrT = std::shared_ptr<TokenGetterT>;
    };

    //////////////////////////////////////////////////////////////////

    template <class ItemT>
    struct DummyTokenGetter : public TokenGetter<ItemT>
    {
      void load( const ItemT & item_r ) override
      {}
    };

    template <class ItemT>
    struct DummyDataGetter : public DataGetterBase<ItemT>
    {
      using TokenGetterT = typename DataGetterBase<ItemT>::TokenGetterT;
      using TokenGetterPtrT = typename DataGetterBase<ItemT>::TokenGetterPtrT;

      TokenGetterPtrT operator()( const Tag & tag_r ) const
      { return TokenGetterPtrT( new TokenGetterT() ); }
    };

    //////////////////////////////////////////////////////////////////

    void dotest()
    {
    }

#if 0

    std::map<std::string_view, auto> {
      { "ARCH",             PoolItem::arch },
      { "EDITION",          PoolItem::edition },
      { "NAME",             PoolItem::name },
      { "RELEASE",          PoolItem::edition },
      { "REQUIRES",         PoolItem::requires },
      { "REQUIRESNAME",     PoolItem::requires },
      { "REQUIRESOP",       PoolItem::requires },
      { "REQUIRESEDITION",  PoolItem::requires },
      { "VERSION",          PoolItem::edition },
    };



    // DataT getter( item_r )
    // "NAME" -> DataGetter<DataT>
    ARCH
    NAME
    RELEASE
    REQUIREFLAGS
    REQUIRENAME
    REQUIRENEVRS
    REQUIRES
    REQUIREVERSION
    VERSION

    template <class ItemT>
    Attrib getAttr( const ItemT & item_r, const String & attr_r )

    struct Attrib
    {
      template <class ItemT>
      Atrttib

    };



#endif
  } // namespace

  template <class ItemT, class DataGetterT=DummyDataGetter<ItemT>>
  struct Renderer
  {
    using TokenGetterT = typename DataGetterT::TokenGetterT;
    using TokenGetterPtrT = typename DataGetterT::TokenGetterPtrT;

    Renderer( Format && format_r )
    : _format { std::move(format_r) }
    { compile(); }

    Renderer( std::string_view qf_r )
    : _format { qf::parse( qf_r ) }
    { compile(); }

    void operator()( const ItemT & item_r ) const
    { operator()( item_r, DOUT ); }

    void operator()( const ItemT & item_r, std::ostream & str ) const
    { render( _format, item_r, str ); }

  private:
    // Format
    void render( const Format & format_r, const ItemT & item_r, std::ostream & str, unsigned idx_r=0 ) const
    {
      for ( const auto & tok : format_r.tokens ) {
        switch ( tok->_type )
        {
          case TokenType::String:
            render( static_cast<const String &>(*tok), str );
            break;
          case TokenType::Tag:
            render( static_cast<const Tag &>(*tok), item_r, str, idx_r );
            break;
          case TokenType::Array:
            render( static_cast<const Array &>(*tok), item_r, str );
            break;
          case TokenType::Conditional:
            render( static_cast<const Conditional &>(*tok), item_r, str );
            break;
        }
      }
    }

    // TokenType::String
    void render( const String & string_r, std::ostream & str ) const
    { str << string_r.value; }

    // TokenType::Tag
    void render( const Tag & tag_r, const ItemT & item_r, std::ostream & str, unsigned idx_r ) const
    {
      std::string_view val{ _cache.at( tag_r.name )->get() };
      if ( tag_r.fieldw  ) {
        const char * fieldw { tag_r.fieldw->c_str() };  // parser asserts at least one digit
        bool ladjust = ( *fieldw == '-' );
        if ( ladjust ) ++fieldw;
        std::size_t fw{ ::strtoul( fieldw, NULL, 10 ) };
        if ( val.size() < fw ) {
          char padchar = ( ladjust || fieldw[0] != '0' ? ' ' : '0' );
          if ( ladjust )
            str << val << std::string( fw-val.size(), padchar );
          else
            str << std::string( fw-val.size(), padchar ) << val;
          return;
        }
      }
      str << val;
    }

    // TokenType::Array
    void render( const Array & array_r, const ItemT & item_r, std::ostream & str ) const
    {
      render( array_r.format, item_r, str, 0 );
      render( array_r.format, item_r, str, 1 );
      render( array_r.format, item_r, str, 2 );
    }

    // TokenType::Conditional
    void render( const Conditional & conditional_r, const ItemT & item_r, std::ostream & str ) const
    { ; }

  private:

    void compile()
    { compile( _format ); }

    void compile( const Format & format_r )
    {
      for ( const auto & tok : format_r.tokens ) {
        switch ( tok->_type )
        {
          case TokenType::String:
            // NOP
            break;
          case TokenType::Tag: {
            const Tag & tag { static_cast<const Tag &>(*tok) };
            compile( tag.name );
          } break;
          case TokenType::Array: {
            const Array & array { static_cast<const Array &>(*tok) };
            compile( array.format );
          } break;
          case TokenType::Conditional: {
            const Conditional & conditional { static_cast<const Conditional &>(*tok) };
            compile( conditional.name );
            compile( conditional.Tformat );
            if ( conditional.Fformat )
              compile( *conditional.Fformat );
          } break;
        }
      }
    }

    void compile( std::string_view name_r )
    {
      MOUT << name_r << endl;
    }

  private:
    Format _format;
    std::unordered_map<std::string_view, TokenGetterPtrT> _cache;
  };

} // namespace zypp::qf


///////////////////////////////////////////////////////////////////
void process( std::string_view qf_r )
try {
  qf::Renderer<PoolItem> render { qf_r };

  static const bool once __attribute__ ((__unused__)) = [](){
    zypp::misc::defaultLoadSystem( misc::LS_NOREFRESH | misc::LS_NOREPOS  );
    return true;
  }();

  unsigned max = 3;
  for ( const auto & el : zypp::ResPool::instance() ) {
    if ( not max-- ) break;
    render(el);
  }
}
catch( const std::exception & ex ) {
  MOUT << "Oopsed: " << ex.what() << endl;
}

///////////////////////////////////////////////////////////////////
int main( int argc, const char ** argv )
{
  using namespace zypp;
  using namespace std::literals;
  ++argv,--argc;
  for ( const char * qf : {
    "", "\\", "\\\\", "\n", "\\n", "\\%", "\\x",
    "%{NAME} %{SIZE}\\n",
    "%-{NAME} %-0009{SIZE}\\n",
    "[%-05{FILENAMES} %09{FILESIZES}\n]",
    "[%{NAME} %{FILENAMES}\\n]",
    "[%{=NAME} %{FILENAMES}\\n]",
    "%{NAME} %{INSTALLTIME:date}\\n",
    "[%{FILEMODES:perms} %{FILENAMES}\\n]",
    "[%{REQUIRENAME} %{REQUIREFLAGS:depflags} %{REQUIREVERSION}\\n]",
    "\\%\\x\\n%|SOMETAG?{present}:{missing}|",
  } )
  {
      qf::Format result;
      qf::parse( qf, result )
      cout << "Parsed: " << result << endl;
  }

  //process( "[\"%15{name}-%-15{version}-%015{release}.%-015{arch}\"\n]" );

  return 0;
}


