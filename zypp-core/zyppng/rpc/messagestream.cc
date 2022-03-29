/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/

#include "messagestream.h"

#include <zypp-core/AutoDispose.h>
#include <zypp-core/zyppng/base/AutoDisconnect>

namespace zyppng {

  InvalidMessageReceivedException::InvalidMessageReceivedException( const std::string &msg )
    : zypp::Exception( zypp::str::Str() << "Invalid Message received: (" << msg <<")" )
  { }


  zyppng::RpcMessageStream::RpcMessageStream( IODevice::Ptr iostr ) : _ioDev( std::move(iostr) )
  {
    connect( *_nextMessageTimer, &Timer::sigExpired, *this, &RpcMessageStream::timeout );
    _nextMessageTimer->setSingleShot(false);

    connect( *_ioDev, &IODevice::sigReadyRead, *this, &RpcMessageStream::readAllMessages );
    if ( _ioDev->isOpen () && _ioDev->canRead () )
      readAllMessages ();
  }

  bool RpcMessageStream::readNextMessage( )
  {
    if ( _pendingMessageSize == 0 ) {
      if ( _ioDev->bytesAvailable() >= sizeof( rpc::HeaderSizeType ) ) {
        _ioDev->read( reinterpret_cast<char *>( &_pendingMessageSize ),  sizeof( rpc::HeaderSizeType ) );
      }
    }

    if ( _ioDev->bytesAvailable() < _pendingMessageSize ) {
      return false;
    }

    auto bytes = _ioDev->read( _pendingMessageSize );
    _pendingMessageSize = 0;

    zypp::proto::Envelope m;
    if (! m.ParseFromArray( bytes.data(), bytes.size() ) ) {
      ERR << "Received malformed message from peer" << std::endl;
      _sigInvalidMessageReceived.emit();
      return false;
    }

    _messages.push_back( std::move(m) );
    _sigNextMessage.emit ();

    if ( _messages.size() ) {
      // nag the user code until all messages have been used up
      _nextMessageTimer->start(0);
    }

    return true;
  }

  void RpcMessageStream::timeout(const Timer &)
  {
    if ( _messages.size() )
      _sigNextMessage.emit();

    if ( !_messages.size() )
      _nextMessageTimer->stop();
  }

  std::optional<RpcMessage> zyppng::RpcMessageStream::nextMessage( const std::string &msgName )
  {
    if ( !_messages.size () ) {

      // try to read the next messages from the fd
      {
        _sigNextMessage.block ();
        zypp::OnScopeExit unblock([&](){
          _sigNextMessage.unblock();
        });
        readAllMessages();
      }

      if ( !_messages.size () )
        return {};
    }

    std::optional<RpcMessage>  res;

    if( msgName.empty() ) {
      res = std::move( _messages.front () );
      _messages.pop_front();

    } else {
      const auto i = std::find_if( _messages.begin(), _messages.end(), [&]( const zypp::proto::Envelope &env ) {
        return env.messagetypename() == msgName;
      });

      if ( i != _messages.end() ) {
        res = std::move(*i);
        _messages.erase(i);
      }
    }

    if ( _messages.size() )
      _nextMessageTimer->stop();

    return res;
  }

  std::optional<RpcMessage> RpcMessageStream::nextMessageWait( const std::string &msgName )
  {
    // make sure the signal is not emitted until we have the next message
    _sigNextMessage.block ();
    zypp::OnScopeExit unblock([&](){
      _sigNextMessage.unblock();
    });

    bool receivedInvalidMsg = false;
    AutoDisconnect defered( connectFunc( *this, &RpcMessageStream::sigInvalidMessageReceived, [&](){
      receivedInvalidMsg = true;
    }));

    const bool hasMsgName = msgName.size();
    while ( !receivedInvalidMsg && _ioDev->isOpen() && _ioDev->canRead() ) {
      if ( _messages.size() ) {
        if ( hasMsgName ) {
          std::optional<RpcMessage> msg = nextMessage(msgName);
          if ( msg ) return msg;
        }
        else {
          break;
        }
      }

      if ( !_ioDev->waitForReadyRead ( -1 ) ) {
        // this can only mean that a error happened, like device was closed
        return {};
      }
    }
    return nextMessage (msgName);
  }

  bool zyppng::RpcMessageStream::sendMessage( const RpcMessage &env )
  {
    if ( !_ioDev->canWrite () )
      return false;

    const auto &str = env.SerializeAsString();
    rpc::HeaderSizeType msgSize = str.length();
    _ioDev->write( (char *)(&msgSize), sizeof( rpc::HeaderSizeType ) );
    _ioDev->write( str.data(), str.size() );
    return true;
  }

  SignalProxy<void ()> zyppng::RpcMessageStream::sigMessageReceived()
  {
    return _sigNextMessage;
  }

  SignalProxy<void ()> RpcMessageStream::sigInvalidMessageReceived()
  {
    return _sigInvalidMessageReceived;
  }

  void RpcMessageStream::readAllMessages()
  {
    bool cont = true;
    while ( cont && _ioDev->bytesAvailable() ) {
      cont = readNextMessage ();
    }
  }
}
