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

namespace zypp {
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

template <class Container>
Value StringContainerType ( Container *target, std::string hint = ARG_STRING ) {
  return Value (
        []() -> boost::optional<std::string> { return boost::optional<std::string>(); },
        [target] ( CommandOption *opt, const boost::optional<std::string> &in ) {
          if (!in) ZYPP_THROW(MissingArgumentException(opt->name)); //value required
          target->push_back(*in);
          return;
        },
        std::move(hint)
  );
}


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

}}

#endif
