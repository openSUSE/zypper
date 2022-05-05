/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_MEDIA_NG_HEADERVALUEMAP_H_INCLUDED
#define ZYPP_MEDIA_NG_HEADERVALUEMAP_H_INCLUDED

#include <variant>
#include <string>
#include <map>
#include <boost/iterator/iterator_adaptor.hpp>
#include <zypp-core/base/PtrTypes.h>

namespace zyppng {

  class HeaderValue
  {
  public:
    using value_type = std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>;

    HeaderValue();

    HeaderValue( const HeaderValue &other );
    HeaderValue( HeaderValue &&other );

    HeaderValue( const bool val );
    HeaderValue( const int32_t val );
    HeaderValue( const int64_t val );
    HeaderValue( const double val );
    HeaderValue( std::string &&val );
    HeaderValue( const std::string &val );
    HeaderValue( const char *val );

    bool valid () const;

    bool isString () const;
    bool isInt () const;
    bool isInt64 () const;
    bool isDouble () const;
    bool isBool () const;

    const std::string &asString () const;
    int32_t asInt   () const;
    int64_t asInt64 () const;
    double  asDouble() const;
    bool asBool () const;

    value_type &asVariant ();
    const value_type &asVariant () const;

    HeaderValue &operator= ( const HeaderValue &other );
    HeaderValue &operator= ( HeaderValue &&other );
    HeaderValue &operator= ( const std::string &val );
    HeaderValue &operator= ( int32_t val );
    HeaderValue &operator= ( int64_t val );
    HeaderValue &operator= ( double val );
    HeaderValue &operator= ( bool val );

    bool operator== ( const HeaderValue &other ) const;

  private:
    zypp::RWCOW_pointer<value_type> _val;
  };

  class HeaderValueMap
  {
  public:
    using Value = HeaderValue;
    using ValueMap = std::map<std::string, std::vector<Value>>;

    static Value InvalidValue;

    class const_iterator
      : public boost::iterator_adaptor<
          HeaderValueMap::const_iterator                             // Derived
          , ValueMap::const_iterator // Base
          , const std::pair<std::string, Value> // Value
          , boost::use_default  // CategoryOrTraversal
          >
    {
    public:
      const_iterator()
        : const_iterator::iterator_adaptor_() {}

      explicit const_iterator( const ValueMap::const_iterator &val )
      { this->base_reference() = val; }

      const_iterator( const HeaderValueMap::const_iterator &other )
        : const_iterator::iterator_adaptor_( other.base() ) {}

      const std::string &key () const {
        return this->base_reference()->first;
      }

      const Value &value() const {
        auto &l = base_reference ()->second;
        if ( l.empty() ) {
          return InvalidValue;
        }
        return l.back();
      }

    private:
      friend class boost::iterator_core_access;
      void increment() {
        this->base_reference() = ++this->base_reference();
      }

      std::pair<std::string, Value> dereference() const
      {
        return  std::make_pair( key(), value() );
      }
    };

    HeaderValueMap() = default;
    HeaderValueMap( std::initializer_list<ValueMap::value_type> init );

    bool contains( const std::string &key ) const;
    bool contains( const std::string_view &key ) const {
      return contains(std::string(key));
    }

    void set( const std::string &key, const Value &val );
    void set( const std::string &key, Value &&val );
    void add( const std::string &key, const Value &val);
    void clear ();
    ValueMap::size_type size() const noexcept;

    std::vector<Value> &values ( const std::string &key );
    const std::vector<Value> &values ( const std::string &key ) const;

    std::vector<Value> &values ( const std::string_view &key ) {
      return values( std::string(key) );
    }

    const std::vector<Value> &values ( const std::string_view &key ) const {
      return values( std::string(key) );
    }

    /*!
     * Returns the last entry with key \a str in the list of values
     * or the default value specified in \a defaultVal
     */
    Value value ( const std::string_view &str, const Value &defaultVal = Value() ) const;
    Value value ( const std::string &str, const Value &defaultVal = Value() ) const;

    Value &operator[]( const std::string &key );
    Value &operator[]( const std::string_view &key );
    const Value &operator[]( const std::string &key ) const;
    const Value &operator[]( const std::string_view &key ) const;

    const_iterator erase( const const_iterator &i );
    bool erase( const std::string &key );

    const_iterator begin() const {
      return const_iterator( _values.begin() );
    }
    const_iterator end() const {
      return const_iterator( _values.end() );
    }

    ValueMap::iterator beginList() {
      return _values.begin();
    }
    ValueMap::iterator endList() {
      return _values.end();
    }

    ValueMap::const_iterator beginList() const {
      return _values.begin();
    }
    ValueMap::const_iterator endList() const {
      return _values.end();
    }

    ValueMap::const_iterator cbeginList() const {
      return _values.cbegin();
    }
    ValueMap::const_iterator cendList() const {
      return _values.cend();
    }

  private:
    ValueMap _values;
  };
}

namespace zypp {
  template<>
  inline zyppng::HeaderValue::value_type* rwcowClone<zyppng::HeaderValue::value_type>( const zyppng::HeaderValue::value_type * rhs )
  { return new zyppng::HeaderValue::value_type(*rhs); }
}


#endif
