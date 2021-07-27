#ifndef ZYPP_RPM_ERRORCODES_H
#define ZYPP_RPM_ERRORCODES_H

#include <string_view>

// This is supposed to contain header only code, add nothing that would require symbols to be linked
namespace zypprpm {
  enum ErrCodes {
    NoError = 0,
    FailedToOpenDb,
    WrongHeaderSize,
    WrongMessageFormat,
    RpmInitFailed,
    FailedToReadPackage,
    FailedToAddStepToTransaction,
    RpmFinishedWithTransactionError, // we got explicit error problems from rpm
    RpmFinishedWithError,            // the transaction started but could not be finished)
    RpmOrderFailed,                  // running rpmtsorder failed
    OtherError = 255
  };
}

// we will send a end of message tag through the script FD whenever we receive a "STOP" message
// this message will consist of this tag and a \n
constexpr char endOfScriptTagData[] = {
  (char)0xde,
  (char)0xad,
  (char)0xbe,
  (char)0xef,
  '\n'
};
constexpr std::string_view endOfScriptTag ( endOfScriptTagData, 5 );

#endif // ZYPP_RPM_ERRORCODES_H
