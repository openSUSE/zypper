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

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Function.h"
#include "zypp/base/String.h"
#include "zypp/base/DefaultIntegral.h"

#include "zypp/parser/xml/Node.h"

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
    /** Base class for ParseDef consumer.
     */
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
    /** ParseDef consumer redirecting all events to another consumer.
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
    /** ParseDef consumer that invokes callbacks.
    */
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

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ParseDefAssignText<_Type>
    //
    /** Assign a \ref Node text to types constructible from \c char*.
     * \code
     * struct ProductNode : public xml::ParseDef
     * {
     *   ProductNode( ProductFileData::Impl & pdata_r )
     *     : ParseDef( "product", MANDTAORY )
     *   {
     *     (*this)
     *         ("vendor",        OPTIONAL,  parseDefAssignText( _vendor ) )
     *         ("name",          MANDTAORY, parseDefAssignText( _name ) )
     *         ...
     *   }
     *
     *   std::string _vendor;
     *   std::string _name;
     * };
     * \endcode
     */
    template <class _Type>
    struct ParseDefAssignText : public xml::ParseDefConsume
    {
      ParseDefAssignText( _Type & value_r )
        : _value( &value_r )
      {}

      virtual void text( const xml::Node & node_r )
      {
        *_value = _Type( node_r.value().c_str() );
      }

      private:
      _Type * _value;
    };

    /** \name ParseDefAssignText specialisation for numeric and boolean values.
     *  \relates ParseDefAssignText
     */
    //@{
    template <>
    inline void ParseDefAssignText<short>::text( const xml::Node & node_r ) { str::strtonum( node_r.value().c_str(), *_value ); }
    template <>
    inline void ParseDefAssignText<int>::text( const xml::Node & node_r ) { str::strtonum( node_r.value().c_str(), *_value ); }
    template <>
    inline void ParseDefAssignText<long>::text( const xml::Node & node_r ) { str::strtonum( node_r.value().c_str(), *_value ); }
    template <>
    inline void ParseDefAssignText<long long>::text( const xml::Node & node_r ) { str::strtonum( node_r.value().c_str(), *_value ); }
    template <>
    inline void ParseDefAssignText<unsigned short>::text( const xml::Node & node_r ) { str::strtonum( node_r.value().c_str(), *_value ); }
    template <>
    inline void ParseDefAssignText<unsigned>::text( const xml::Node & node_r ) { str::strtonum( node_r.value().c_str(), *_value ); }
    template <>
    inline void ParseDefAssignText<unsigned long>::text( const xml::Node & node_r )  { str::strtonum( node_r.value().c_str(), *_value ); }
    template <>
    inline void ParseDefAssignText<unsigned long long>::text( const xml::Node & node_r ) { str::strtonum( node_r.value().c_str(), *_value ); }
    template <>
    inline void ParseDefAssignText<bool>::text( const xml::Node & node_r ) { str::strToBoolNodefault( node_r.value().c_str(), *_value ); }
    //@}

    /** \name ParseDefAssignText Convenience constructor.
     * \relates ParseDefAssignText
     *
     * This returns a \c shared_ptr<xml::ParseDefConsume> ready to be passed
     * to a \ref ParseDef node.
     */
    //@{
    template <class _Type>
    shared_ptr<xml::ParseDefConsume> parseDefAssignText( _Type & value_r )
    { return shared_ptr<xml::ParseDefConsume>( new ParseDefAssignText<_Type>( value_r ) ); }

    template<class _Tp, _Tp _Initial>
    shared_ptr<xml::ParseDefConsume> parseDefAssignText( DefaultIntegral<_Tp,_Initial> & value_r )
    { return shared_ptr<xml::ParseDefConsume>( new ParseDefAssignText<_Tp>( value_r.get() ) ); }
    //@}

    ///////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_XML_PARSEDEFCONSUME_H
