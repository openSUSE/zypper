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

#include <zypp/ResKind.h>
#include <zypp/TriBool.h>
#include <set>

class Out;

namespace zypp {

class Kind;

namespace ZyppFlags {

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
 */
Value CounterType    ( int *target, const boost::optional<int> &defValue = boost::optional<int>(), const boost::optional<int> &maxValue = boost::optional<int>()  );

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
 * Creates a boolean flag. Can either set or unset a boolean value controlled by \a store.
 * The value in \a defVal is only used for generating the help
 */
Value BoolType   ( bool *target, StoreFlag store = StoreTrue, const boost::optional<bool> &defValue = boost::optional<bool>()  );


Value TriBoolType   ( TriBool &target, StoreFlag store = StoreTrue, const boost::optional<TriBool> &defValue = boost::optional<TriBool>()  );


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

/**
 * Creates a null type, calling the setter or default value getter for this type will do nothing.
 * Use this to have a ignored deprecated flag
 */
Value NoValue ();

/**
 * Creates a value that emits a warning when set, if \a val_r is valid the calls are forwarded to it after
 * emitting the warning.
 */
Value WarnOptionVal (Out &out_r , const std::string &warning_r, Out::Verbosity verbosity_r = Out::NORMAL, const boost::optional<Value> &val_r = boost::optional<Value>());

/**
 * Helper function that just returns a empty default value
 */
boost::optional<std::string> noDefaultValue();

}}

#endif
