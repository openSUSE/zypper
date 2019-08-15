/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_UTILS_H
#define ZYPPER_UTILS_H

#include <string>
#include <set>
#include <list>

#include <zypp/Url.h>
#include <zypp/Pathname.h>

#include <zypp/ResKind.h>
#include <zypp/RepoInfo.h>
#include <zypp/ZYppCommitPolicy.h>

class Zypper;
class Table;

namespace zypp
{
  class PoolItem;
  class Resolvable;
  class Product;
  class Pattern;
}
using namespace zypp;

#ifdef JEZYPP_PRODTRANS
#if 0
#include <iostream>
#include <zypp/base/LogTools.h>
using std::cout;
using std::endl;
#endif
#include <map>
#include <zypp/PoolItem.h>
#include <zypp/parser/ProductFileReader.h>
// JEZYPP replacement for Product::Ptr/constPtr as RES libsolv
// does not provide product: pseudo packages.
class FakeProduct
{
  struct Impl
  {
    Impl()
    {}

    Impl( const PoolItem & pi_r )
    : _pi { pi_r }
    {
      std::map<std::string,Edition> vmap;
      // parse product- provides from the release package:
      for ( auto cap : pi_r.provides() )
      {
	if ( const char * cstr = cap.c_str() )
	{
	  if ( cstr[0] == 'p' && cstr[1] == 'r' && cstr[2] == 'o' && cstr[3] == 'd' && cstr[4] == 'u' && cstr[5] == 'c' && cstr[6] == 't' )
	  {
	    if ( cstr[7] == '(' )
	    {
	      const CapDetail & detail { cap.detail() };
	      if ( detail.isVersioned() && detail.op() == Rel::EQ )
	      {
		cstr = detail.name().c_str();
		if ( cstr[8] == ')' &&  cstr[9] == '\0' )
		{
		  // product() = NAME
		  assignNameIf( detail.ed().idStr() );
		}
		else if ( cstr[8] != '\0' )
		{
		  std::string name { cstr+8 };
		  if ( name.back() == ')' )
		  {
		    // product(openSUSE) = 15.1-1
		    name.erase( name.size()-1 );
		    assignEditionIf( vmap[name], detail.ed() );
		  }
		}
	      }
	    }
	    else if ( cstr[7] == '-' )
	    {
	      using zypp::url::decode;
	      const CapDetail & detail { cap.detail() };
	      if ( detail.isVersioned() && detail.op() == Rel::EQ )
	      {
		assignTagIf( product_label(), detail.name(), detail.ed().idStr() )
		|| assignTagIf( product_endoflife(), detail.name(), detail.ed().idStr() )
		|| assignTagIf( product_register_target(), detail.name(), detail.ed().idStr() )
		|| assignTagIf( product_register_release(), detail.name(), detail.ed().idStr() )
		|| assignTagIf( product_register_flavor(), detail.name(), detail.ed().idStr() )
		|| assignTagIf( product_type(), detail.name(), detail.ed().idStr() )
		|| assignTagIf( product_cpeid(), detail.name(), detail.ed().idStr() )
// 		|| assignTagIf( product_flags(), detail.name(), detail.ed().idStr() )
// 		|| assignTagIf( product_updates_repoid(), detail.name(), detail.ed().idStr() )
		;
	      }
	    }
	  }
	}
      }

      if ( _name.empty() )
      {
	// legacy `product()` without `= name` --> the lex-least one
	if ( !vmap.empty() )
	{
	  _name = IdString(vmap.begin()->first);
	  _edition = vmap.begin()->second;
	}
	else
	{
	  // Oops - no product
	  *this = Impl();
	  return;
	}

      }
      else
	_edition = vmap[_name.asString()];

      // so far so good, BUT...
      // ... product_endoflife() is a YYYY-MM_DD to be converted to Date
      // ... product-register-release() is usually no product- provides, but read from the .prod file
      //     as libsolv prefers the .pod file data for installed products.
      if ( _pi.isSystem() )
      {
	// Rough but will work as long as product FOO uses Foo.prod
	_referenceFilename = _name.asString()+".prod";

	PathInfo referenceFilePath { proddir() / _referenceFilename };
	if ( ! referenceFilePath.isFile() )
	  _referenceFilename.clear();	// TODO: had to check the filelist
	else
	{
	  // are we base? (overwrite package-provides)
	  _isBaseProduct = ( filesystem::readlink( proddir()/"baseproduct" ).basename() == _referenceFilename );
	  _tagmap[product_type()] = ( _isBaseProduct ? "base" : "" );

	  // get 'installed only' attributes scanned from the .prod file
	  try
          {
	    parser::ProductFileData pd { parser::ProductFileReader::scanFile( referenceFilePath.path() ) };
	    assignTag( product_register_target(), pd.registerTarget() );
	    assignTag( product_register_release(), pd.registerRelease() );
	    assignTag( product_register_flavor(), pd.registerFlavor() );
	    assignTag( productline(), pd.productline() );
	  }
          catch ( ... )
          {;}
	}
      }
    }

    void assignNameIf( const IdString & n_r )
    {
      if ( !n_r.empty() && ( _name.empty() || n_r < _name ) )
	_name = n_r;
    }

    void assignEditionIf( Edition & lval_r, const Edition & e_r )
    {
      if ( !e_r.empty() && ( lval_r.empty() || e_r > lval_r ) )
	lval_r = e_r;
    }

    bool assignTagIf( const IdString & ltag_r, const IdString & key_r, const IdString & val_r )
    {
      bool ret = false;
      if ( ltag_r == key_r && !val_r.empty() )
      {
	std::string & lval ( _tagmap[ltag_r] );
	std::string val { url::decode( val_r.asString() ) };
	if ( lval.empty() || val < lval )
	{
	  lval = val;
	  ret = true;
	}
      }
      return ret;
    }

    void assignTag( const IdString & ltag_r, const std::string & val_r )
    { _tagmap[ltag_r] = val_r; }


    std::string tagstr( const IdString & tag_r ) const
    {
      std::string ret;
      auto it = _tagmap.find( tag_r );
      if ( it != _tagmap.end() )
	ret = (*it).second;
      return ret;
    }


    PoolItem _pi;
    IdString _name;	// product() = openSUSE
    Edition _edition;	// product(openSUSE) = 15.1-1

    Pathname proddir() const;		// (/root)/etc/products.d"
    std::string _referenceFilename;	// installed only; name of the included .prod file
    bool _isBaseProduct = false;	// installed && baseproduct symlink points to this .prod

    std::map<IdString,std::string> _tagmap;	// deescaped string values of product provides
    static IdString product_label()		{ static IdString tag( "product-label()" ); return tag; }
    static IdString product_endoflife()		{ static IdString tag( "product-endoflife()" ); return tag; }
    static IdString product_register_target()	{ static IdString tag( "product-register-target()" ); return tag; }
    static IdString product_register_release()	{ static IdString tag( "product-register-release()" ); return tag; }
    static IdString product_register_flavor()	{ static IdString tag( "product-register-flavor()" ); return tag; }
    static IdString product_type()		{ static IdString tag( "product-type()" ); return tag; }
    static IdString product_cpeid()		{ static IdString tag( "product-cpeid()" ); return tag; }
    // other tags read from installed .prod files
    static IdString productline()		{ return IdString( 0 ); }

//     not yet needed but known known product- provides...
//     static IdString product_flags()		{ static IdString tag( "product-flags()" ); return tag; }
//     static IdString product_updates_repoid()	{ static IdString tag( "product-updates-repoid()" ); return tag; }
  };

public:
  FakeProduct()
  : FakeProduct( PoolItem() )
  {}

  FakeProduct( const sat::Solvable & solv_r )
  : FakeProduct( PoolItem( solv_r ) )
  {}

  FakeProduct( const PoolItem & pi_r )
  : _pimpl { new Impl( pi_r ) }
  {}

  // to be syntax compatible with Product::Ptr
  FakeProduct * operator->()		{ return this; }
  const FakeProduct * operator->() const{ return this; }
  FakeProduct & operator*()		{ return *this; }
  const FakeProduct & operator*() const	{ return *this; }

  // False if created from a not-release-package
  bool isProduct() const		{ return bool(_pimpl->_pi.satSolvable()); }

  // product attributes we need
  ResKind kind() const			{ return ResKind::product; }
  std::string name() const		{ return _pimpl->_name.asString(); }
  Edition edition() const		{ return _pimpl->_edition; }
  Arch arch() const			{ return _pimpl->_pi.arch(); }
  IdString vendor() const		{ return _pimpl->_pi.vendor(); }
  std::string summary() const		{ return _pimpl->_pi.summary(); }
  std::string description() const	{ return _pimpl->_pi.description(); }
  Repository repository() const		{ return _pimpl->_pi.repository(); }
  RepoInfo repoInfo() const		{ return _pimpl->_pi.repoInfo(); }
  bool isSystem() const			{ return _pimpl->_pi.isSystem(); }

  std::string shortName() const 	{ std::string ret { _pimpl->tagstr( Impl::product_label() ) }; if ( ret.empty() ) ret = name(); return ret; }
  std::string type() const		{ return _pimpl->tagstr( Impl::product_type() ); }
  CpeId cpeId() const			{ return CpeId( _pimpl->tagstr( Impl::product_cpeid() ), CpeId::noThrow ); }

  Date endOfLife() const		{ return Date(); }		// product-endoflife()
  bool hasEndOfLife() const	        { return false; }
  bool hasEndOfLife( Date & value ) const { return false; }

  static bool isTargetDistribution( const PoolItem & fakeproduct_r )
  { return FakeProduct(fakeproduct_r).isTargetDistribution(); }

  bool isTargetDistribution() const	{ return _pimpl->_isBaseProduct; }
  std::string registerTarget() const	{ return _pimpl->tagstr( Impl::product_register_target() ); }
  std::string registerRelease() const	{ return _pimpl->tagstr( Impl::product_register_release() ); }
  std::string registerFlavor() const	{ return _pimpl->tagstr( Impl::product_register_flavor() ); }

  std::string productLine() const	{ return _pimpl->tagstr( Impl::productline() ); }
  std::string flavor() const;
  std::string referenceFilename() const { return _pimpl->_referenceFilename; }

  sat::Solvable referencePackage() const{ return _pimpl->_pi.satSolvable(); }

private:
  RW_pointer<Impl> _pimpl;
};

inline std::ostream & operator<<( std::ostream & str, const FakeProduct & obj )
{
  str << "FakeProduct["
      << obj.name() << "-" << obj.edition() << "." << obj.arch()
      << (obj.isTargetDistribution() ? " *" : "  ") << obj.referenceFilename()
      << "{" << obj.referencePackage() << "}]";
  return str;
}

// for zypper -x products
std::string asXML( const FakeProduct & p, bool is_installed );

// fake query result, map release-package to Fakeproduct
typedef std::map<sat::Solvable,FakeProduct> Prodtranstable;
#endif // JEZYPP_PRODTRANS


typedef std::set<ResKind> ResKindSet;

/** Build search status column tag */
inline const char * lockStatusTag( const char * tag_r, bool islocked_r, bool isautoinst_r = false )
{
  if ( islocked_r )
  {
    if ( *tag_r == 'i' )
      return "il";
    else if ( *tag_r == 'v' )
      return "vl";
    else if ( *tag_r == '\0' || *tag_r == ' ' )
      return " l";
    INT << "unknown status tag '" << tag_r << "'" << std::endl;
    return "?L";	// should not happen
  }

  if ( *tag_r == 'i' )
    return( isautoinst_r ? "i" : "i+" );

  return tag_r;
}

/** Whether running on SLE.
 * If so, report e.g. unsupported packages per default.
 */
bool runningOnEnterprise();

/** Converts user-supplied kind to ResKind object.
 * Returns an empty one if not recognized. */
ResKind string_to_kind( std::string skind );

ResKindSet kindset_from( const std::list<std::string> & kindstrings );

std::string kind_to_string_localized( const ResKind & kind, unsigned long count );


// ----------------------------------------------------------------------------
// PATCH related strings for various purposes
// ----------------------------------------------------------------------------
const char* textPatchStatus( const PoolItem & pi_r );		///< Patch status: plain text noWS (forXML)
std::string i18nPatchStatus( const PoolItem & pi_r );		///< Patch status: i18n + color
std::string patchHighlightCategory( const Patch & patch_r );	///< Patch Category + color
std::string patchHighlightSeverity( const Patch & patch_r );	///< Patch Severity + color
std::string patchInteractiveFlags( const Patch & patch_r );	///< Patch interactive properties (reboot|message|license|restart or ---) + color

/** Patches table default format */
struct FillPatchesTable
{
  FillPatchesTable( Table & table_r, TriBool inst_notinst_r = indeterminate );
  bool operator()( const PoolItem & pi_r ) const;
private:
  Table * _table;	///< table used for output
  TriBool _inst_notinst;///< LEGACY: internally filter [not]installed
};

/** Patches table when searching for issues */
struct FillPatchesTableForIssue
{
  FillPatchesTableForIssue( Table & table_r );
  bool operator()( const PoolItem & pi_r, std::string issue_r, std::string issueNo_r ) const;
private:
  Table * _table;	///< table used for output
};

// ----------------------------------------------------------------------------
/**
 * Creates a Url out of \a urls_s. If the url_s looks looks_like_url()
 * Url(url_s) is returned. Otherwise if \a url_s represends a valid path to
 * a file or directory, a dir:// Url is returned. Otherwise an empty Url is
 * returned.
 */
Url make_url( const std::string & url_s );

/**
 * Creates Url out of obs://project/platform URI with given base URL and default
 * platform (used in case the platform is not specified in the URI).
 */
Url make_obs_url( const std::string & obsuri, const Url & base_url, const std::string & default_platform );

/**
 * Returns <code>true</code> if the string \a s contains a substring starting
 * at the beginning and ending before the first colon matches any of registered
 * schemes (Url::isRegisteredScheme()).
 */
bool looks_like_url( const std::string& s );

/**
 * Returns <code>true</code> if \a s ends with ".rpm" or starts with "/", "./",
 * or "../".
 */
bool looks_like_rpm_file( const std::string & s );

/**
 * Download the RPM file specified by \a rpm_uri_str and copy it into
 * \a cache_dir.
 *
 * \return The local Pathname of the file in the cache on success, empty
 *      Pathname if a problem occurs.
 */
Pathname cache_rpm( const std::string & rpm_uri_str, const Pathname & cache_dir );

std::string & indent( std::string & text, int columns );

// comparator for RepoInfo set
struct RepoInfoAliasComparator
{
  bool operator()( const RepoInfo & a, const RepoInfo & b )
  { return a.alias() < b.alias(); }
};


// comparator for Service set
struct ServiceAliasComparator
{
  bool operator()( const repo::RepoInfoBase_Ptr & a, const repo::RepoInfoBase_Ptr & b )
  { return a->alias() < b->alias(); }
};


/**
 * checks name for .repo string
 */
inline bool isRepoFile( const std::string & name )
{ return name.find(".repo") != name.npos; }

std::string asXML( const Product & p, bool is_installed );

std::string asXML( const Pattern & p, bool is_installed );

/**
 * Check whether one of --download-* or --download options was given and return
 * the specified mode.
 */
DownloadMode get_download_option( Zypper & zypper, bool quiet = false );

/** Check whether packagekit is running using a DBus call */
bool packagekit_running();

/** Send suggestion to quit to PackageKit via DBus */
void packagekit_suggest_quit();

#endif /*ZYPPER_UTILS_H*/
