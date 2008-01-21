/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/FileReaderBaseImpl.h
 *
*/
#ifndef ZYPP_PARSER_SUSETAGS_FILEREADERBASEIMPL_H
#define ZYPP_PARSER_SUSETAGS_FILEREADERBASEIMPL_H

#include <iosfwd>

#include "zypp/base/Logger.h"
#include "zypp/base/Function.h"

#include "zypp/parser/susetags/FileReaderBase.h"
#include "zypp/parser/ParseException.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/PathInfo.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      inline std::string makeSharedIdent( ResKind kind_r,
					  const std::string & name_r,
					  Edition edition_r,
					  Arch arch_r )
      {
	std::string ret( kind_r.asString() );
	ret += ":";
	ret += name_r;
	ret += "-";
	ret += edition_r.asString();
	ret += ".";
	ret += arch_r.asString();
	return ret;
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : FileReaderBase::BaseImpl
      //
      /** Common base for susetags::FileReader implementations. */
      class FileReaderBase::BaseImpl : private base::NonCopyable
      {
	public:
	  BaseImpl( const FileReaderBase & parent_r )
	  : _parent( parent_r )
	  {}
	  virtual ~BaseImpl()
	  {}

	public:

	  template<class _Res>
	  void depAddLine( const std::string & line_r,
			   data::DependencyList & deps_r )
	  {
	    depAddLine( line_r, ResTraits<_Res>::kind, deps_r );
	  }

	  void depAddLine( const std::string & line_r,
			   ResKind refers_r,
			   data::DependencyList & deps_r )
	  {
	    deps_r.insert( Capability( line_r, refers_r ) );
	  }


	  template<class _Res>
	  void depParse( const MultiTagPtr & tag_r,
			 data::DependencyList & deps_r )
	  {
	    depParse( tag_r, ResTraits<_Res>::kind, deps_r );
	  }

	  void depParse( const MultiTagPtr & tag_r,
			 ResKind refers_r,
			 data::DependencyList & deps_r )
	  {
	    std::for_each( tag_r->value.begin(),
			   tag_r->value.end(),
			   bind( &BaseImpl::depAddLine, this, _1, refers_r, ref(deps_r) ) );
	  }

	public:
	  ParseException error( const SingleTagPtr & tag_r,
				const std::string & msg_r = std::string() ) const
	  { return ParseException( _parent.errPrefix( tag_r, msg_r ) ); }

	  ParseException error( const MultiTagPtr & tag_r,
				const std::string & msg_r = std::string() ) const
	  { return ParseException( _parent.errPrefix( tag_r, msg_r ) ); }

	private:
	  const FileReaderBase & _parent;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_SUSETAGS_FILEREADERBASEIMPL_H
