/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "flagtypes.h"
#include "main.h"
#include "utils/messages.h"
#include "utils/misc.h"

#include "Zypper.h"

namespace zypp {
namespace ZyppFlags {

boost::optional<std::string> noDefaultValue()
{
  return boost::optional<std::string>();
}

Value StringType(std::string *target, const boost::optional<const char *> &defValue, std::string hint) {
  return Value (
    [defValue]() ->  boost::optional<std::string>{
      if (!defValue || *defValue == nullptr)
        return boost::optional<std::string>();
      return std::string(*defValue);
    },
    [target]( const CommandOption &opt, const boost::optional<std::string> &in ){
      if (!in)
        ZYPP_THROW(MissingArgumentException(opt.name));
      *target = *in;
      return;
    },
    std::move(hint)
  );
}

Value IntType(int *target, const boost::optional<int> &defValue) {
  return Value (
        [defValue]() -> boost::optional<std::string>{
          if(defValue) {
            return std::to_string(*defValue);
          } else
            return boost::optional<std::string>();
        },

        [target]( const CommandOption &opt, const boost::optional<std::string> &in ) {
          *target = argValueConvert<int>(opt, in);
        },
        ARG_INTEGER
  );
}

Value BoolType(bool *target, StoreFlag store, const boost::optional<bool> &defVal) {
  return BoolCompatibleType (*target, store, defVal );
}

Value TriBoolType(TriBool &target, StoreFlag store, const boost::optional<TriBool> &defValue)
{
  return Value (
    [defValue]() -> boost::optional<std::string>{
      if (!defValue)
        return boost::optional<std::string>();
      return asString ( *defValue );
    },
   [&target, store]( const CommandOption &, const boost::optional<std::string> &){
      target = TriBool(store == StoreTrue);
    }
  );
}

Value CounterType(int *target, const boost::optional<int> &defValue, const boost::optional<int> &maxValue, bool failOnOverflow)
{
  return Value (
        [defValue]() -> boost::optional<std::string>{
          if(defValue) {
            return std::to_string(*defValue);
          } else
            return boost::optional<std::string>();
        },

        [target, maxValue, failOnOverflow]( const CommandOption &opt, const boost::optional<std::string> & ) {
          if ( maxValue && (*target) + 1 > *maxValue) {
            if ( failOnOverflow )
              ZYPP_THROW(ZyppFlagsException(str::Format(_("The flag '%1%' can only be used a maximum of %2% times.")) % opt.name % *maxValue));
            else
              return;
          }
          *target += 1;
        }
  );
}


Value KindSetType(std::set<ResKind> *target) {
  return GenericContainerType (
    *target,
    ARG_TYPE
  );
}


Value StringVectorType(std::vector<std::string> *target, std::string hint) {
  return GenericContainerType (
        *target,
        std::move(hint)
  );
}


Value NoValue()
{
  return Value (
        noDefaultValue,
        []( const CommandOption &, const boost::optional<std::string> & ) {
          return;
        }
  );
}

Value WarnOptionVal(Out &out_r, const std::string &warning_r, Out::Verbosity verbosity_r, const boost::optional<Value> &val_r )
{
  return Value (
        [ val_r ] () -> boost::optional<std::string> {
          if ( val_r )
            return val_r->defaultValue();
          return boost::optional<std::string>();
        },
        [ &out_r, warning_r, val_r, verbosity_r ] ( const CommandOption &opt, const boost::optional<std::string> &in ) {
          out_r.warning( warning_r, verbosity_r );
          if ( val_r ) {
            Value val = *val_r; // C++ forces us to copy by value again
            val.set ( opt, in );
          }
        }
  );
}

Value PathNameType( filesystem::Pathname &target, const boost::optional<std::string> &defValue, std::string hint ) {
  return Value (
    [defValue]() ->  boost::optional<std::string>{
      if (!defValue )
        return boost::optional<std::string>();
      return *defValue;
    },
    [ &target ]( const CommandOption &opt, const boost::optional<std::string> &in ){
      if (!in)
        ZYPP_THROW(MissingArgumentException(opt.name));
      target = filesystem::Pathname(*in);
      return;
    },
    std::move(hint)
  );
}

Value IssueSetType(std::set<Issue> &target_r, const std::string &issueType_r, std::string hint_r )
{
  return Value (
    [](){
      //in case the default value is given a Issue with anyID is pushed into the result set
      return std::string ();
    },
    [ &target_r, issueType_r ]( const CommandOption &opt, const boost::optional<std::string> &in ){

      std::vector<std::string> issueIds;
      Issue anyVal = Issue( issueType_r, std::string() );

      // if issue type is already in the target
      // -> and the value it empty we replace it
      // -> and the value is not empty we keep the existing value
      auto pos = target_r.find( anyVal );
      if ( pos != target_r.end() ) {
        if ( ( *pos ).id().empty() ) {

          Zypper::instance().out().warning(str::form(
            _("Ignoring %s without argument because similar option with an argument has been specified."),
            ("--" + opt.name).c_str() ));

          target_r.erase( pos );
        }
      }

      /// \bug this will not insert anything if the key already exists, should we emit a message?
      if ( !in || str::split( *in, std::back_inserter(issueIds), "," ) == 0 ) {
        target_r.insert( anyVal );
      } else {
        for ( auto & val : issueIds )
        { target_r.insert( Issue( issueType_r, std::move(val) ) ); }
      }

    },
    std::move(hint_r)
  );
}

template <>
zypp::Date argValueConvert ( const CommandOption &opt, const boost::optional<std::string> &in ) {
  try {
    return Date ( *in, "%F" );
  } catch ( const DateFormatException &e ) {
    ZYPP_THROW(InvalidValueException ( opt.name, *in, e.msg() ) );
  }
}

template <>
int argValueConvert ( const CommandOption &opt, const boost::optional<std::string> &in )
{
  if (!in)
    ZYPP_THROW(MissingArgumentException(opt.name));

  try {
    return std::stoi( *in );
  } catch ( const std::invalid_argument &e ) {
    ZYPP_THROW(InvalidValueException(opt.name, *in, e.what()));
  } catch ( const std::out_of_range &e) {
    ZYPP_THROW(InvalidValueException(opt.name, *in, _("Out of range")));
  } catch ( ... ) {
    ZYPP_THROW(ZyppFlagsException(str::Format(_("Unknown error while assigning the value %1% to flag %2%.")) % *in % opt.name));
  }
}

template<>
ResKind argValueConvert( const CommandOption &opt, const boost::optional<std::string> &in )
{
  if (!in) ZYPP_THROW(MissingArgumentException(opt.name)); //value required
  ResKind knd = string_to_kind(*in);
  if ( knd == ResKind::nokind )
    ZYPP_THROW(InvalidValueException( opt.name, *in, _("Unknown package type")));

  return knd;
}

Value CallbackVal( SetterFun &&callback , std::string &&hint )
{
  return Value {
    noDefaultValue,
    std::move( callback ),
    std::move( hint )
  };
}

}
}
