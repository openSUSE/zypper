/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/xml/ParseDefConsume.h
 *
*/
#ifndef ZYPP_PARSER_XML_PARSEDEFCONSUME_H
#define ZYPP_PARSER_XML_PARSEDEFCONSUME_H

#include <zypp/base/PtrTypes.h>
#include <zypp/base/Function.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace xml
  { /////////////////////////////////////////////////////////////////

    class Node;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefConsume
    //
    /** */
    struct ParseDefConsume
    {
      virtual ~ParseDefConsume();

      virtual void start( const Node & _node );
      virtual void text ( const Node & _node );
      virtual void cdata( const Node & _node );
      virtual void done ( const Node & _node );

      virtual void startSubnode( const Node & _node );
      virtual void doneSubnode ( const Node & _node );
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefConsumeRedirect
    //
    /**
     * \note Allocated <tt>ParseDefConsume *</tt> passed are
     *       immediately wraped into a shared_ptr.
    */
    class ParseDefConsumeRedirect : public ParseDefConsume
    {
    public:
      ParseDefConsumeRedirect();
      ParseDefConsumeRedirect( const shared_ptr<ParseDefConsume> & target_r );
      ParseDefConsumeRedirect( ParseDefConsume * allocatedTarget_r );
      ParseDefConsumeRedirect( ParseDefConsume & target_r );

      virtual ~ParseDefConsumeRedirect();

    public:
      void setRedirect( const shared_ptr<ParseDefConsume> & target_r );
      void setRedirect( ParseDefConsume * allocatedTarget_r );
      void setRedirect( ParseDefConsume & target_r );
      void cancelRedirect();

      shared_ptr<ParseDefConsume> getRedirect() const;

    public:
      virtual void start( const Node & _node );
      virtual void text ( const Node & _node );
      virtual void cdata( const Node & _node );
      virtual void done ( const Node & _node );
      virtual void startSubnode( const Node & _node );
      virtual void doneSubnode ( const Node & _node );

    private:
      shared_ptr<ParseDefConsume> _target;
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefConsumeCallback
    //
    /** */
    class ParseDefConsumeCallback : public ParseDefConsume
    {
    public:
      typedef function<void(const Node &)> Callback;

      ParseDefConsumeCallback();

      virtual ~ParseDefConsumeCallback();

    public:
      virtual void start( const Node & node_r );
      virtual void text( const Node & node_r );
      virtual void cdata( const Node & node_r );
      virtual void done( const Node & node_r );
      virtual void startSubnode( const Node & node_r );
      virtual void doneSubnode( const Node & node_r );

    public:
      Callback _start;
      Callback _text;
      Callback _cdata;
      Callback _done;
      Callback _startSubnode;
      Callback _doneSubnode;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_XML_PARSEDEFCONSUME_H
