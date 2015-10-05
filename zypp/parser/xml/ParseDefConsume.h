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
#include "zypp/base/Hash.h"
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
    /** \ref parseDefAssign exposed details */
    namespace parse_def_assign
    { /////////////////////////////////////////////////////////////////
      template <class Tp> struct Assigner;

      typedef shared_ptr<Assigner<void> > AssignerRef;

      /** Common interface to all Assigner types. */
      template <>
      struct Assigner<void>
      {
        virtual ~Assigner()
        {}
        virtual void assign( const char * text_r )
        {}
      };

      /** Assigner assigns text to types constructible from \c char*.
       * \see \ref assigner convenience constructor.
      */
      template <class Tp>
      struct Assigner : public Assigner<void>
      {
        Assigner( Tp & value_r )
          : _value( &value_r )
        {}

        virtual void assign( const char * text_r )
        { *_value = Tp( text_r ); }

        private:
          Tp * _value;
      };

      /** \name Assigner specialisation for numeric and boolean values.
       *  \relates Assigner
       */
      //@{
      template <>
          inline void Assigner<short>::assign( const char * text_r )              { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<int>::assign( const char * text_r )                { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<long>::assign( const char * text_r )               { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<long long>::assign( const char * text_r )          { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<unsigned short>::assign( const char * text_r )     { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<unsigned>::assign( const char * text_r )           { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<unsigned long>::assign( const char * text_r )      { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<unsigned long long>::assign( const char * text_r ) { str::strtonum( text_r, *_value ); }
      template <>
          inline void Assigner<bool>::assign( const char * text_r )               { str::strToBoolNodefault( text_r, *_value ); }
      //@}

      /** \name \relates Assigner Convenience constructor */
      //@{
      template <class Tp>
          inline AssignerRef assigner( Tp & value_r )
      { return AssignerRef( new Assigner<Tp>( value_r ) ); }

      template <class Tp, Tp TInitial>
          inline AssignerRef assigner( DefaultIntegral<Tp,TInitial> & value_r )
      { return AssignerRef( new Assigner<Tp>( value_r.get() ) ); }
      //@}


      /** \ref ParseDef consumer assigning \ref Node text and attribues values to variables.
       *
       * This can be used with all types supported by \ref Assigner.
       * Basically all types constructible from \c char*, or where a
       * specialisation exists (e.g. numeric and bool).
       *
       * You may also set a <tt>void( const Node & )</tt> notification
       * callback which is invoked after the node was processed.
       *
       * \note Use and see \ref xml::parseDefAssign convenience constructor.
       *
       * \code
       * // parsedef for '<setup attr="13">value</setup>'
       * ParseDef( "attr", MANDTAORY, xml::parseDefAssign( data.value )
       *                                                 ( "attr", data.attr ) )
       * \endcode
       */
      struct Consumer : public ParseDefConsume
      {
        /** Extend \ref Consumer. */
        void add( const AssignerRef & assigner_r )
        { _text.push_back( assigner_r ); }

        /** Extend \ref Consumer. */
        void add( const std::string & attr_r, const AssignerRef & assigner_r )
        { _attr[attr_r].push_back( assigner_r ); }

        /** Set pre notification callback. */
        void prenotify( function<void ( const Node & )> pre_r )
        { _pre = pre_r; }

        /** Set post notification callback. */
        void postnotify( function<void ( const Node & )> post_r )
        { _post = post_r; }

        virtual void start( const xml::Node & node_r )
        {
          if ( _pre )
            _pre( node_r );

          if ( ! _attr.empty() )
            for_( it, _attr.begin(), _attr.end() )
              assign( it->second, node_r.getAttribute( it->first.c_str() ).c_str() );
        }

        virtual void text( const xml::Node & node_r )
        {
          if ( ! _text.empty() )
            assign( _text, node_r.value().c_str() );
        }

        virtual void done( const xml::Node & node_r )
        {
          if ( _post )
            _post( node_r );
        }

        private:
          void assign( const std::vector<AssignerRef> & vec_r, const char * value_r )
          {
            if ( value_r )
              for_( it, vec_r.begin(), vec_r.end() )
                (*it)->assign( value_r );
          }

        private:
          std::unordered_map<std::string, std::vector<AssignerRef> > _attr;
          std::vector<AssignerRef>                                   _text;
          function<void ( const Node & )>                            _pre;
          function<void ( const Node & )>                            _post;
      };

      /** Helper class to build a \ref Consumer.
       * \relates Consumer
       *
       * The class constructs the consumer, allows to extend it via
       * \ref operator(), and provides a conversion to
       * \c shared_ptr<ParseDefConsume>, so it can be passed as a
       * node consumer to \ref ParseDef.
       *
       * You may also set a <tt>void( const Node & )</tt> notification
       * callback which is invoked before/after the node was processed.
       *
       * \note Use and see \ref xml::parseDefAssign convenience constructor.
      */
      struct Builder
      {
        /** Contruct \ref Consumer. */
        Builder()
          : _ptr( new Consumer )
        {}

        /** Contruct \ref Consumer. */
        template <class Tp>
            Builder( Tp & value_r )
          : _ptr( new Consumer )
        { operator()( value_r ); }

        /** Contruct \ref Consumer. */
        template <class Tp>
            Builder( const std::string & attr_r, Tp & value_r )
          : _ptr( new Consumer )
        {  operator()( attr_r, value_r ); }

        /** Extend \ref Consumer. */
        template <class Tp>
            Builder & operator()( Tp & value_r )
        { _ptr->add( assigner( value_r ) ); return *this; }

        /** Extend \ref Consumer. */
        template <class Tp>
            Builder & operator()( const std::string & attr_r, Tp & value_r )
        { _ptr->add( attr_r, assigner( value_r ) ); return *this; }

        /** Set pre notification callback. */
        Builder & operator<<( function<void ( const Node & )> done_r )
        { _ptr->prenotify( done_r ); return *this; }

        /** Set post notification callback. */
        Builder & operator>>( function<void ( const Node & )> done_r )
        { _ptr->postnotify( done_r ); return *this; }

        /** Type conversion so this can be passed as node consumer to \ref ParseDef. */
        operator shared_ptr<ParseDefConsume> () const
        { return _ptr; }

        private:
          shared_ptr<Consumer> _ptr;
      };
      /////////////////////////////////////////////////////////////////
    } // namespace parse_def_assign
    ///////////////////////////////////////////////////////////////////

    /** \name \ref ParseDef consumer assigning \ref Node text and attribues values to variables.
     * \relates parse_def_assign::Consumer
     * \relates parse_def_assign::Builder
     *
     * This function allows convenient contruction of a \ref parse_def_assign::Consumer
     * to be passed as \ref Node conssumer to \ref ParseDef. Simply list each attributes
     * name together with the variable it's value should be assigned to. If the attribute
     * name is omitted, the nodes text value gets assigned.
     *
     * Target variables can be of any type tsupported by \ref Assigner.
     * Basically all types constructible from \c char*, or where a
     * specialisation exists (e.g. numeric and bool).
     *
     * \code
     * void setupDone( const xml::Node & _node )
     * { ... }
     *
     * // parsedef for '<setup attr="13">value</setup>'
     * ParseDef( "attr", MANDTAORY,
     *           xml::parseDefAssign( data.value )
     *                              ( "attr", data.attr )
     *                              >> &setupDone       );
     * \endcode
     *
     * \see \ref xml::rnParse for more example.
     */
    //@{
    inline parse_def_assign::Builder parseDefAssign()
    { return parse_def_assign::Builder(); }

    template <class Tp>
        inline parse_def_assign::Builder parseDefAssign( Tp & value_r )
    { return parse_def_assign::Builder( value_r ); }

    template <class Tp>
        inline parse_def_assign::Builder parseDefAssign( const std::string & attr_r, Tp & value_r )
    { return parse_def_assign::Builder( attr_r, value_r ); }
    //@}

    /////////////////////////////////////////////////////////////////
  } // namespace xml
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_XML_PARSEDEFCONSUME_H
