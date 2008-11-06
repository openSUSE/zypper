/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/xml/ParseDefConsume.cc
 *
*/
#include "zypp/parser/xml/ParseDefConsume.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefConsume
    //
    ///////////////////////////////////////////////////////////////////

    ParseDefConsume::~ParseDefConsume()
    {}

    void ParseDefConsume::start( const Node & _node )
    {}

    void ParseDefConsume::text( const Node & _node )
    {}

    void ParseDefConsume::cdata( const Node & _node )
    {}

    void ParseDefConsume::done( const Node & _node )
    {}

    void ParseDefConsume::startSubnode( const Node & _node )
    {}

    void ParseDefConsume::doneSubnode( const Node & _node )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefConsumeRedirect
    //
    ///////////////////////////////////////////////////////////////////

    ParseDefConsumeRedirect::ParseDefConsumeRedirect()
    {}

    ParseDefConsumeRedirect::ParseDefConsumeRedirect( const shared_ptr<ParseDefConsume> & target_r )
    : _target( target_r )
    {}

    ParseDefConsumeRedirect::ParseDefConsumeRedirect( ParseDefConsume * allocatedTarget_r )
    : _target( allocatedTarget_r )
    {}

    ParseDefConsumeRedirect::ParseDefConsumeRedirect( ParseDefConsume & target_r )
    : _target( &target_r, NullDeleter() )
    {}

    ParseDefConsumeRedirect::~ParseDefConsumeRedirect()
    {}

    void ParseDefConsumeRedirect::setRedirect( const shared_ptr<ParseDefConsume> & target_r )
    { _target = target_r; }

    void ParseDefConsumeRedirect::setRedirect( ParseDefConsume * allocatedTarget_r )
    { _target.reset( allocatedTarget_r ); }

    void ParseDefConsumeRedirect::setRedirect( ParseDefConsume & target_r )
    { _target.reset( &target_r, NullDeleter() ); }

    void ParseDefConsumeRedirect::cancelRedirect()
    { _target.reset(); }

    shared_ptr<ParseDefConsume> ParseDefConsumeRedirect::getRedirect() const
    { return _target; }

    void ParseDefConsumeRedirect::start( const Node & _node )
    {
      if ( _target )
        _target->start( _node );
    }

    void ParseDefConsumeRedirect::text( const Node & _node )
    {
      if ( _target )
        _target->text( _node );
    }

    void ParseDefConsumeRedirect::cdata( const Node & _node )
    {
      if ( _target )
        _target->cdata( _node );
    }

    void ParseDefConsumeRedirect::done( const Node & _node )
    {
      if ( _target )
        _target->done( _node );
    }

    void ParseDefConsumeRedirect::startSubnode( const Node & _node )
    {
      if ( _target )
        _target->startSubnode( _node );
    }

    void ParseDefConsumeRedirect::doneSubnode ( const Node & _node )
    {
      if ( _target )
        _target->doneSubnode( _node );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefConsumeCallback
    //
    ///////////////////////////////////////////////////////////////////

    ParseDefConsumeCallback::ParseDefConsumeCallback()
    {}

    ParseDefConsumeCallback::~ParseDefConsumeCallback()
    {}

    void ParseDefConsumeCallback::start( const Node & node_r )
    {
      if ( _start )
        _start( node_r );
    }

    void ParseDefConsumeCallback::text( const Node & node_r )
    {
      if ( _text )
        _text( node_r );
    }

    void ParseDefConsumeCallback::cdata( const Node & node_r )
    {
      if ( _cdata )
        _cdata( node_r );
    }

    void ParseDefConsumeCallback::done( const Node & node_r )
    {
      if ( _done )
        _done( node_r );
    }

    void ParseDefConsumeCallback::startSubnode( const Node & node_r )
    {
      if ( _startSubnode )
        _startSubnode( node_r );
    }

    void ParseDefConsumeCallback::doneSubnode( const Node & node_r )
    {
      if ( _doneSubnode )
        _doneSubnode( node_r );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
