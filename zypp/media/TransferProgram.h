
#ifndef TRANSFER_PROGRAM_H_
#define TRANSFER_PROGRAM_H_

#include <functional>
#include <list>
#include <string>
#include <vector>
#include "zypp/base/Flags.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"
#include "zypp/Url.h"

namespace zypp
{
namespace media
{

/**
 * Easy access to the transfer command line program
 *
 */
class TransferProgram
{
public:

  typedef std::vector<std::string> OutputLines;

  /**
   * After calling the program, one or more error flags can
   * be set according to the result.
   */
  enum ErrorFlag
  {
    /** Operation ended successfully */
    NO_ERROR = 0x0,
  };
  ZYPP_DECLARE_FLAGS(ErrorFlags,ErrorFlag);

  /**
   * Constructs a gpg cmd line access.
   * \p home is the home directory where keyrings are created.
   * 
   * Gpg has one pubkeyring and one secret keyring in its home.
   *
   */
  TransferProgram();

  /**
   * Executes command with a given argument
   */
  void execute();

  /**
   * Contains the error flags from the last command execution
   */
  ErrorFlags errorFlags() const;

  /**
   * contains the last command execution exit code
   */
  int exitCode() const;

  /**
   * begin iterator over output lines available after
   * execute
   */
  OutputLines::const_iterator outputLinesBegin() const;

  /**
   * end iterator over output lines available after
   * execute
   */
  OutputLines::const_iterator outputLinesEnd() const;

  /**
   * returns the full output for convenience
   */
  std::string output() const;

  /**
   * add a header, on the form "Foo: Bar"
   */
  void addHeader( const std::string &header );

  /**
   * sets the user agent ie: "Mozilla v3"
   */
  void setUserAgentString( const std::string &agent );

  /**
   * sets the auth username
   */
  void setUsername( const std::string &username );

  /**
   * sets the auth password
   */
  void setPassword( const std::string &password );

  /**
   * whether the proxy is used or not
   */
  void setProxyEnabled( bool enabled );

  /**
   * sets the proxy user
   */
  void setProxyUsername( const std::string &proxyuser );

  /**
   * sets the proxy password
   */
  void setProxyPassword( const std::string &proxypass );

  /**
   * set the connection timeout
   */
  void setTimeout( int t );

  /**
   * set url to download
   */
  void setUrl( const zypp::Url &url);

  class Impl;
protected:
  RWCOW_pointer<Impl> _impl;
};
ZYPP_DECLARE_OPERATORS_FOR_FLAGS(TransferProgram::ErrorFlags);

} // ns media
} // ns zypp

#endif
