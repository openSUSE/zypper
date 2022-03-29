#include "headervaluemap.h"
#include <zypp-core/base/String.h>

namespace zyppng {

  HeaderValueMap::Value HeaderValueMap::InvalidValue;

  HeaderValue::HeaderValue()
    : _val ( new std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>() )
  {}

  HeaderValue::HeaderValue( const HeaderValue &other )
    : _val ( new std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>( *other._val ) )
  {}

  HeaderValue::HeaderValue( HeaderValue &&other )
    : _val ( new std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>( std::move(*other._val) ) )
  {}

  HeaderValue::HeaderValue( const bool val )
    : _val ( new std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>(val) )
  {}

  HeaderValue::HeaderValue( const int32_t val )
    : _val ( new std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>(val) )
  {}

  HeaderValue::HeaderValue( const int64_t val )
    : _val ( new std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>(val) )
  {}

  HeaderValue::HeaderValue( const double val )
    : _val ( new std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>(val) )
  {}

  HeaderValue::HeaderValue( const std::string &val )
    : _val ( new std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>(val) )
  {}

  HeaderValue::HeaderValue(const char *val)
    : HeaderValue( zypp::str::asString (val) )
  {}

  HeaderValue::HeaderValue( std::string &&val )
    : _val ( new std::variant<std::monostate, std::string, int32_t, int64_t, double, bool>( std::move(val) ) )
  {}

  bool HeaderValue::valid() const
  {
    return ( _val->index () > 0 );
  }

  bool HeaderValue::isString() const
  {
    return std::holds_alternative<std::string>(*_val);
  }

  bool HeaderValue::isInt() const
  {
    return std::holds_alternative<int32_t>(*_val);
  }

  bool HeaderValue::isInt64() const
  {
    return std::holds_alternative<int64_t>(*_val);
  }

  bool HeaderValue::isDouble() const
  {
    return std::holds_alternative<double>(*_val);
  }

  bool HeaderValue::isBool() const
  {
    return std::holds_alternative<bool>(*_val);
  }

  const std::string &HeaderValue::asString() const
  {
    return std::get<std::string>(*_val);
  }

  int32_t HeaderValue::asInt() const
  {
    return std::get<int32_t>(*_val);
  }

  int64_t HeaderValue::asInt64() const
  {
    if ( std::holds_alternative<int32_t>(*_val) )
      return std::get<int32_t>( *_val );
    return std::get<int64_t>(*_val);
  }

  double HeaderValue::asDouble() const
  {
    return std::get<double>(*_val);
  }

  bool HeaderValue::asBool() const
  {
    return std::get<bool>(*_val);
  }

  HeaderValue::value_type &HeaderValue::asVariant()
  {
    return *_val;
  }

  const HeaderValue::value_type &HeaderValue::asVariant() const
  {
    return *_val;
  }

  HeaderValue &HeaderValue::operator=(const HeaderValue &other)
  {
    *_val = *other._val;
    return *this;
  }

  bool HeaderValue::operator==(const HeaderValue &other) const
  {
    return ( *_val == *other._val );
  }

  HeaderValue &HeaderValue::operator= ( HeaderValue &&other )
  {
    *_val = std::move( *other._val );
    return *this;
  }

  HeaderValue &HeaderValue::operator= ( const std::string &val )
  {
    *_val = val;
    return *this;
  }

  HeaderValue &HeaderValue::operator= ( int32_t val )
  {
    *_val = val;
    return *this;
  }

  HeaderValue &HeaderValue::operator= ( int64_t val )
  {
    *_val = val;
    return *this;
  }

  HeaderValue &HeaderValue::operator= ( double val )
  {
    *_val = val;
    return *this;
  }

  HeaderValue &HeaderValue::operator= ( bool val )
  {
    *_val = val;
    return *this;
  }


  HeaderValueMap::HeaderValueMap( std::initializer_list<HeaderValueMap::ValueMap::value_type> init )
    : _values( std::move(init) )
  { }

  bool HeaderValueMap::contains(const std::string &key) const
  {
    return _values.count (key) > 0 && _values.at(key).size () > 0 ;
  }

  void HeaderValueMap::set( const std::string &key, const Value &val )
  {
    auto i = _values.find (key);
    if ( i == _values.end() ) {
      _values.insert ( std::make_pair(key, std::vector<Value>{val}) );
    } else {
      i->second = std::vector<Value>{val};
    }
  }

  void HeaderValueMap::set(const std::string &key, Value &&val)
  {
    auto i = _values.find (key);
    if ( i == _values.end() ) {
      _values.insert ( std::make_pair(key, std::vector<Value>{std::move(val)}) );
    } else {
      i->second = std::vector<Value>{std::move(val)};
    }
  }

  void HeaderValueMap::add(const std::string &key, const Value &val)
  {
    auto i = _values.find (key);
    if ( i == _values.end() ) {
      _values.insert ( std::make_pair(key, std::vector<Value>{val}) );
    } else {
      i->second.push_back(val);
    }
  }

  void HeaderValueMap::clear()
  {
    _values.clear();
  }

  HeaderValueMap::ValueMap::size_type HeaderValueMap::size() const noexcept
  {
    return _values.size();
  }

  std::vector<HeaderValueMap::Value> &HeaderValueMap::values(const std::string &key)
  {
    return _values[key];
  }

  const std::vector<HeaderValueMap::Value> &HeaderValueMap::values(const std::string &key) const
  {
    return _values.at(key);
  }

  HeaderValueMap::Value &HeaderValueMap::operator[](const std::string &key)
  {
    if ( !contains(key) )
      return InvalidValue;
    return _values[key].back();
  }

  HeaderValueMap::const_iterator HeaderValueMap::erase(const const_iterator &i)
  {
    auto yi = _values.erase(i.base());
    return HeaderValueMap::const_iterator(yi);
  }

  bool HeaderValueMap::erase(const std::string &key)
  {
    return ( _values.erase(key) > 0 );
  }

}
