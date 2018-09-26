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

namespace zypp {
namespace ZyppFlags {

namespace {

ResKind parseKindArgument( const CommandOption &opt, const boost::optional<std::string> &in)
{
  if (!in) ZYPP_THROW(MissingArgumentException(opt.name)); //value required
  ResKind knd = string_to_kind(*in);
  if ( knd == ResKind::nokind )
    ZYPP_THROW(InvalidValueException( opt.name, *in, _("Unknown package type")));

  return knd;
}

}

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
          if (!in)
            ZYPP_THROW(MissingArgumentException(opt.name));

          try {
            *target = std::stoi( *in );
          } catch ( const std::invalid_argument &e ) {
            ZYPP_THROW(InvalidValueException(opt.name, *in, e.what()));
          } catch ( const std::out_of_range &e) {
            ZYPP_THROW(InvalidValueException(opt.name, *in, _("Out of range")));
          } catch ( ... ) {
            ZYPP_THROW(ZyppFlagsException(str::Format(_("Unknown error while assigning the value %1% to flag %2%.")) % *in % opt.name));
          }
        },
        "NUMBER"
  );
}

Value BoolType(bool *target, StoreFlag store, const boost::optional<bool> &defVal) {
  return Value (
    [defVal]() -> boost::optional<std::string>{
      if (!defVal)
        return boost::optional<std::string>();
      return std::string( (*defVal) ? "true" : "false" );
    },
   [target, store]( const CommandOption &, const boost::optional<std::string> &){
      *target = (store == StoreTrue);
    }
  );
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

Value CounterType(int *target, const boost::optional<int> &defValue, const boost::optional<int> &maxValue)
{
  return Value (
        [defValue]() -> boost::optional<std::string>{
          if(defValue) {
            return std::to_string(*defValue);
          } else
            return boost::optional<std::string>();
        },

        [target, maxValue]( const CommandOption &opt, const boost::optional<std::string> & ) {
          *target += 1;
          if ( maxValue && *target > *maxValue)
            ZYPP_THROW(ZyppFlagsException(str::Format(_("The flag '%1%' can only be used a maximum of %2% times.")) % opt.name % *maxValue));
        }
  );
}


Value KindSetType(std::set<ResKind> *target) {
  return Value (
        noDefaultValue,
        [target] ( const CommandOption &opt, const boost::optional<std::string> &in ) {
            target->insert( parseKindArgument( opt, in ) );
            return;
          },
          "TYPE"
  );
}

Value StringVectorType(std::vector<std::string> *target, std::string hint) {
  return Value (
        noDefaultValue,
        [target] ( const CommandOption &opt, const boost::optional<std::string> &in ) {
          if ( !in || in->empty() ) ZYPP_THROW(MissingArgumentException(opt.name)); //value required
          target->push_back(*in);
          return;
        },
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

}
}
