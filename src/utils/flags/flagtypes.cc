/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "flagtypes.h"
#include "main.h"

namespace zypp {
namespace ZyppFlags {

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

}
}
