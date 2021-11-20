/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPP_FLAGTYPES_H_INCLUDED
#define ZYPP_FLAGTYPES_H_INCLUDED

#include "zyppflags.h"
#include "exceptions.h"
#include "output/Out.h"
#include "issue.h"

#include <zypp/ResKind.h>
#include <zypp/TriBool.h>
#include <zypp/Pathname.h>
#include <zypp/Date.h>
#include <zypp/Patch.h>
#include <set>
#include <type_traits>

#include <boost/optional.hpp>

class Out;

namespace zypp {

class Kind;

namespace ZyppFlags {

/**
 * Helper function that just returns a empty default value
 */
boost::optional<std::string> noDefaultValue();

/**
 * Returns a \sa ZyppFlags::Value instance handling flags taking a string parameter
 */
Value StringType ( std::string *target, const boost::optional<const char *> &defValue = boost::optional<const char *> (), std::string hint = ARG_STRING );

/**
 * Returns a \sa ZyppFlags::Value instance handling flags taking a int parameter
 */
Value IntType    ( int *target, const boost::optional<int> &defValue = boost::optional<int>()  );

/**
 * Returns a \sa ZyppFlags::Value instance counting how many times a parameter was seen
 * If \a failOnOverflow is true (default) using the switch too often will throw a exception
 */
Value CounterType    ( int *target, const boost::optional<int> &defValue = boost::optional<int>(), const boost::optional<int> &maxValue = boost::optional<int>(), bool failOnOverflow = true  );

/**
 * Returns a \sa ZyppFlags::Value instance handling flags taking package or ressource types
 */
Value KindSetType ( std::set<ResKind> *target );

/**
 * Returns a \sa ZyppFlags::Value instance handling flags that fill a vector of strings
 */
Value StringVectorType (std::vector<std::string> *target, std::string hint = "STRING"  );

/**
 * Specifies how to handle when a boolean flag was seen on commandline
 */
enum StoreFlag : int{
  StoreFalse,  //< set the boolean flag to false
  StoreTrue    //< set the boolean flag to true
};

/**
 * Creates a boolean flag. Can either set or unset a value convertible to bool controlled by \a store.
 * The value in \a defVal is only used for generating the help
 */
template <typename T>
Value BoolCompatibleType ( T &target, StoreFlag store = StoreTrue, const boost::optional<T> &defVal = boost::optional<T>() )
{
  return Value (
    [defVal]() -> boost::optional<std::string>{
      if (!defVal)
        return boost::optional<std::string>();
      return std::string( (*defVal) ? "true" : "false" );
    },
   [&target, store]( const CommandOption &, const boost::optional<std::string> &){
      target = (store == StoreTrue);
    }
  );
}

/**
 * Creates a boolean flag. Can either set or unset a boolean value controlled by \a store.
 * The value in \a defVal is only used for generating the help
 */
Value BoolType   ( bool *target, StoreFlag store = StoreTrue, const boost::optional<bool> &defValue = boost::optional<bool>()  );


Value TriBoolType   ( TriBool &target, StoreFlag store = StoreTrue, const boost::optional<TriBool> &defValue = boost::optional<TriBool>()  );

template <typename T>
T argValueConvert ( const CommandOption &, const boost::optional<std::string> & );

template <>
zypp::Date argValueConvert ( const CommandOption &, const boost::optional<std::string> &in );

template <>
inline std::string argValueConvert ( const CommandOption &, const boost::optional<std::string> &in ) {
  return *in;
}

template <>
int argValueConvert ( const CommandOption &, const boost::optional<std::string> &in );

template <template<typename ...> class Container, typename T >
Value GenericContainerType  ( Container<T> &target_r, std::string hint = std::string(), const std::string &sep = "", const std::string &defaultVal = std::string() ) {
  DefValueFun defValueFun;
  if (!defaultVal.empty()) {
    defValueFun = [defaultVal]() { return boost::optional<std::string>(defaultVal); };
  }
  else {
    defValueFun = noDefaultValue;
  }
  return Value (
    std::move( defValueFun ),
    [ &target_r, sep ] ( const CommandOption &opt, const boost::optional<std::string> &in ) {
      if ( !in ) ZYPP_THROW(MissingArgumentException(opt.name)); //value required

      auto it = std::inserter ( target_r, target_r.end() );
      if ( !sep.empty() ) {
        std::vector<std::string> vals;
        str::split( *in, std::inserter(vals, vals.end()), "," );
        for ( const std::string & strVal : vals )
          it = argValueConvert<T>( opt, strVal );
      } else {
        it = argValueConvert<T>( opt, in );
      }
    },
    std::move( hint )
  );
}

template < typename T >
Value GenericValueType  ( T &target_r, std::string hint = std::string() ) {
  return Value (
    noDefaultValue,
    [ &target_r ] ( const CommandOption &opt, const boost::optional<std::string> &in ) {
      if ( !in || in->empty() ) ZYPP_THROW(MissingArgumentException(opt.name)); //value required
      target_r = argValueConvert<T>( opt, in );
    },
    std::move( hint )
  );
}

template <typename T, typename E = T>
Value BitFieldType ( T& target, E flag , StoreFlag store = StoreTrue ) {
  return Value (
        [ &target, flag] () -> boost::optional<std::string> {
          return target.testFlag ( flag ) ? std::string("true") : std::string("false");
        },
        [ &target, flag, store ] ( const CommandOption &, const boost::optional<std::string> & ) {
          if ( store == StoreTrue )
            target.setFlag ( flag );
          else
            target.unsetFlag ( flag );
        }
  );
}

template <typename T>
Value WriteFixedValueType ( T& target, const T &value ) {
  return Value (
        noDefaultValue,
        [ &target, value ] ( const CommandOption &, const boost::optional<std::string> & ) {
          target = value;
        }
  );
}

/**
 * Returns a \sa ZyppFlags::Value instance handling flags taking a string parameter representing a \sa zypp::filesystem::PathName
 */
Value PathNameType( filesystem::Pathname &target, const boost::optional<std::string> &defValue = boost::optional<std::string>(), std::string hint = std::string() );

/**
 * Creates a null type, calling the setter or default value getter for this type will do nothing.
 * Use this to have a ignored deprecated flag
 */
Value NoValue ();

/**
 * Creqtes a type forwarding the \sa Value setter call to the function specified in \a callback
 */
Value CallbackVal ( ZyppFlags::SetterFun &&callback, std::string &&hint = std::string() );

/**
 * Creates a value that emits a warning when set, if \a val_r is valid the calls are forwarded to it after
 * emitting the warning.
 */
Value WarnOptionVal (Out &out_r , const std::string &warning_r, Out::Verbosity verbosity_r = Out::NORMAL, const boost::optional<Value> &val_r = boost::optional<Value>());

/**
 * Handles command argument that pass bug reference numbers of specific types ( issues, cve, bugzilla, bz )
 */
Value IssueSetType (std::set<Issue> &target_r, const std::string &issueType_r, std::string hint_r = "");


}}

#endif
