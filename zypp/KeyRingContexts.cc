/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/KeyRingContexts.cc
 */
#include <optional>

#include <zypp/KeyRingContexts.h>
#include <zypp/base/LogTools.h>

///////////////////////////////////////////////////////////////////
namespace zypp::keyring
{
  ///////////////////////////////////////////////////////////////////
  /// \class VerifyFileContext::Impl
  /// Directly accessed by verifyFileSignatureWorkflow
  /// to set the result data.
  ///////////////////////////////////////////////////////////////////
  class VerifyFileContext::Impl
  {
  public:
    Impl()
    {}
    Impl( Pathname file_r )
    : _file( std::move(file_r) )
    {}
    Impl( Pathname file_r, Pathname signature_r )
    : _file( std::move(file_r) )
    , _signature( std::move(signature_r) )
    {}

    void resetResults()
    { _fileAccepted = _fileValidated = _signatureIdTrusted = false; _signatureId.clear(); }

    // In:
    Pathname _file;
    Pathname _signature;
    std::optional<std::string> _shortFile;
    KeyContext _keyContext;
    BuddyKeys _buddyKeys;

    // Results:
    bool _fileAccepted = false;
    bool _fileValidated = false;
    std::string _signatureId;
    bool _signatureIdTrusted = false;

  private:
    friend Impl * rwcowClone<Impl>( const Impl * rhs );
    /** clone for RWCOW_pointer */
    Impl * clone() const
    { return new Impl( *this ); }
  };

  VerifyFileContext::VerifyFileContext()
  : _pimpl( new Impl() )
  {}
  VerifyFileContext::VerifyFileContext( Pathname file_r )
  : _pimpl( new Impl( std::move(file_r) ) )
  {}
  VerifyFileContext::VerifyFileContext( Pathname file_r, Pathname signature_r )
  : _pimpl( new Impl( std::move(file_r), std::move(signature_r) ) )
  {}
  VerifyFileContext::~VerifyFileContext()
  {}

  // In:
  const Pathname & VerifyFileContext::file() const
  { return _pimpl->_file; }

  void VerifyFileContext::file( Pathname file_r )
  { _pimpl->_file = std::move(file_r); }

  const Pathname & VerifyFileContext::signature() const
  { return _pimpl->_signature; }

  void VerifyFileContext::signature( Pathname signature_r )
  { _pimpl->_signature = std::move(signature_r); }

  std::string VerifyFileContext::shortFile() const
  { return _pimpl->_shortFile.has_value() ? _pimpl->_shortFile.value() : _pimpl->_file.basename(); }

  void VerifyFileContext::shortFile( std::string shortFile_r )
  { _pimpl->_shortFile = std::move(shortFile_r); }

  const KeyContext & VerifyFileContext::keyContext() const
  { return _pimpl->_keyContext; }

  void VerifyFileContext::keyContext( KeyContext keyContext_r )
  { _pimpl->_keyContext = std::move(keyContext_r); }

  const VerifyFileContext::BuddyKeys & VerifyFileContext::buddyKeys() const
  { return _pimpl->_buddyKeys; }

  void VerifyFileContext::addBuddyKey( std::string sid_r )
  { _pimpl->_buddyKeys.insert( sid_r ); }

  // Results:
  void VerifyFileContext::resetResults()
  { _pimpl->resetResults(); }

  bool VerifyFileContext::fileAccepted() const
  { return _pimpl->_fileAccepted; }

  void VerifyFileContext::fileAccepted( bool yesno_r )
  { _pimpl->_fileAccepted = yesno_r; }

  bool VerifyFileContext::fileValidated() const
  { return _pimpl->_fileValidated; }

  void VerifyFileContext::fileValidated( bool yesno_r )
  { _pimpl->_fileValidated = yesno_r; }

  const std::string & VerifyFileContext::signatureId() const
  { return _pimpl->_signatureId; }

  void VerifyFileContext::signatureId( std::string signatureId_r )
  { _pimpl->_signatureId = std::move(signatureId_r); }

  bool VerifyFileContext::signatureIdTrusted() const
  { return _pimpl->_signatureIdTrusted; }

  void VerifyFileContext::signatureIdTrusted( bool yesno_r )
  { _pimpl->_signatureIdTrusted = yesno_r; }

  std::ostream & operator<<( std::ostream & str, const VerifyFileContext & obj )
  {
    return str << obj.file()
    << "[" << obj.signature().basename()
    << " accepted:" << asString( obj.fileAccepted() )
    << ", validated:" << ( obj.fileValidated() ? ( obj.signatureIdTrusted() ? "trusted" : "true" ) : "false" )
    << "(" << obj.signatureId() << ")"
    << "]";;
  }

} // namespace zypp::keyring
///////////////////////////////////////////////////////////////////
