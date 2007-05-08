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
#include "zypp/parser/tagfile/ParseException.h"
#include "zypp/data/ResolvableData.h"

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

	  struct CapImplCache
	  {
	    template<class _Res>
	    capability::CapabilityImpl::Ptr get( const std::string & line_r )
	    {
	      return get( line_r, ResTraits<_Res>::kind );
	    }

	    capability::CapabilityImpl::Ptr get( const std::string & line_r,
		                                 ResolvableTraits::KindType refers_r )
	    {
	      capability::CapabilityImpl::Ptr & ret( _cache[refers_r][line_r] );
	      if ( ! ret )
	      {
		ret = capability::parse( refers_r, line_r );
	      }
	      return ret;
	    }

	    private:
	      std::map<ResolvableTraits::KindType, std::map<std::string, capability::CapabilityImpl::Ptr> > _cache;
	  };

	public:

	  template<class _Res>
	  void depAddLine( const std::string & line_r,
			   data::DependencyList & deps_r )
	  {
	    depAddLine( line_r, ResTraits<_Res>::kind, deps_r );
	  }

	  void depAddLine( const std::string & line_r,
			   ResolvableTraits::KindType refers_r,
			   data::DependencyList & deps_r )
	  {
	    deps_r.insert( _depcache.get( line_r, refers_r ) );
	  }


	  template<class _Res>
	  void depParse( const MultiTagPtr & tag_r,
			 data::DependencyList & deps_r )
	  {
	    depParse( tag_r, ResTraits<_Res>::kind, deps_r );
	  }

	  void depParse( const MultiTagPtr & tag_r,
			 ResolvableTraits::KindType refers_r,
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
	  CapImplCache           _depcache;
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
