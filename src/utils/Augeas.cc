/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
extern "C"
{
  #include <augeas.h>
}

#include <iostream>
#include <stdlib.h>

#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

#include "Zypper.h"
#include "utils/Augeas.h"

///////////////////////////////////////////////////////////////////
namespace env
{
  inline const char * notEmpty( const char * var_r )
  {
    const char * ret = ::getenv( var_r );
    if ( ret && ! *ret )
      ret = nullptr;

    return ret;
  }

  inline const char * HOME()
  { return notEmpty( "HOME" ); }

  inline const char * PWD()
  { return notEmpty( "PWD" ); }

} // namespace env
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace
{
  struct AugException : public Exception
  {
    AugException()
    : Exception( "AugeasException" )
    {}
    AugException( const std::string & msg_r )
    : Exception( "AugeasException: "+msg_r )
    {}
  };

  struct AugBadPath : public AugException
  {
    AugBadPath( const std::string & augpath_r )
    : AugException( "Bad path "+augpath_r )
    {}
  };

  struct AugBadInsert : public AugException
  {
    AugBadInsert( const std::string & augpath_r, const std::string & label_r )
    : AugException( "Insert '"+label_r+"' failed at "+augpath_r )
    {}
  };

  struct AugBadMatchIndex : public AugException
  {
    AugBadMatchIndex( unsigned idx_r )
    : AugException( "Bad index "+std::to_string(idx_r) )
    {}
  };

  struct AugParseError : public AugException
  {
    AugParseError( std::string  msg_r )
    : AugException( std::move(msg_r) )
    {}
  };

  struct AugSaveError : public AugException
  {
    AugSaveError( std::string  msg_r )
    : AugException( std::move(msg_r) )
    {}
  };

  struct AugPath;

  ///////////////////////////////////////////////////////////////////
  /// \brief Shared reference to ::augeas (calling ::aug_close if done).
  /// Also \ref AugPath factory from std::string.
  struct AugRef
  {
    AugRef()
    {}

    explicit AugRef( ::augeas * take_r )
    : _ref { take_r, &deleter }
    {
      if ( ! take_r )
	ZYPP_THROW( AugException(_("Cannot initialize configuration file parser.") ) );
    }

  public:
    AugPath augPath( std::string path_r = std::string() ) const;	///< AugPath factory

  public:
    ::augeas * aug()
    { return _ref.get(); }

    const ::augeas * aug() const
    { return _ref.get(); }

  private:
    static void deleter( ::augeas * aug_r )
    { if ( aug_r ) ::aug_close( aug_r ); }

  private:
    boost::shared_ptr<::augeas> _ref;
  };

  ///////////////////////////////////////////////////////////////////
  /// \brief Path into an augeas tree.
  /// Able to get/set \ref label and \ref value if the path is unique.
  /// Use \ref AugMatches to get the unique matches denoted by an
  /// ambiguous path.
  ///
  struct AugPath : public AugRef
  {
    friend std::ostream & operator<<( std::ostream & str_r, const AugPath & obj_r );

    AugPath( const AugRef & aug_r, std::string path_r = std::string() )
    : AugRef { aug_r }
    , _path { std::move(path_r) }
    {}


    std::string & str()
    { return _path; }

    const std::string & str() const
    { return _path; }

    const char * c_str() const
    { return _path.c_str(); }

    explicit operator bool() const
    { return !_path.empty(); }

    AugPath & operator=( const std::string & str_r )
    { str() = str_r; return *this; }

    AugPath & operator+=( const std::string & str_r )
    { str() += str_r; return *this; }

    AugPath operator+( const std::string & str_r ) const
    { return AugPath(*this) += str_r; }

    /** Returns a unique AugPath to the first match or an empty AugPath if no matches.
     * \see \ref AugMatches
     */
    AugPath firstMatch() const;

    /** Returns a unique AugPath to the last match or an empty AugPath if no matches.
     * \see \ref AugMatches
     */
    AugPath lastMatch() const;

  public:
    bool empty() const
    { return _path.empty(); }

    bool exists() const
    { return( matches() > 0 ); }

    bool unique() const
    { return( matches() == 1 ); }

    unsigned matches() const
    {
      int res = ::aug_match( aug(), c_str(), nullptr );
      if ( res == -1 )
	ZYPP_THROW( AugBadPath( str() ) );
      return res;
    }

  public:
    /** \name Get label and value from unique nodes.
     * \throws AugBadPath if path does not match exactly 1 node
     */
    //@{
    std::string label() const
    {
      const char * v { nullptr };
      if ( ::aug_label( aug(), c_str(), &v ) == 1 )
	return v;
      ZYPP_THROW( AugBadPath( str() ) );
    }

    /** An empty string is also returned for a nullptr value (see \ref getRaw) */
    std::string get() const
    {
      const char * v { getRaw() };
      return( v ? v : "" );
    }

    /** The returned value might be a nullptr (which is ok) */
    const char * getRaw() const
    {
      const char * v { nullptr };
      if ( ::aug_get( aug(), c_str(), &v ) == 1 )
	return v;
      ZYPP_THROW( AugBadPath( str() ) );
    }

    /** Unlike \ref get this method does not throw and returns an empty string if something went wrong. */
    std::string value() const
    {
      const char * v { unique() ? getRaw() : nullptr };
      return( v ? v : "" );
    }

    //@}

  public:
    /** \name Rename the labels of all matching nodes (returns the number).
     * \throws AugException if rename fails.
     */
    //@{
    unsigned rename( const char * label_r )
    {
      int ret = ::aug_rename( aug(), c_str(), label_r );
      if ( ret == -1 )
	ZYPP_THROW( AugException( str::Format("Rename '%1%' failed at %2%") % label_r % str() )  );
      return ret;
    }

    unsigned rename( const std::string & label_r )
    { return rename( label_r.c_str() ); }
    //@}

  public:
    /** \name Set value for unique or not existing nodes.
     * \throws AugBadPath if path matches more than 1 node.
     */
    //@{
    void set( const char * value_r )
    {
      if ( ::aug_set( aug(), c_str(), value_r ) != 0 )
	ZYPP_THROW( AugBadPath( str() ) );
    }

    void set( const std::string & value_r )
    { set( value_r.c_str() ); }
    //@}

  public:
    /** \name Insert a new sibling node before or after an existing node.
     * \throws AugBadInsert if path does not match exactly 1 node or label is invalid.
     */
    //@{
    void insert( const std::string & label_r, bool before_r )
    {
      if ( ::aug_insert( aug(), c_str(), label_r.c_str(), int(before_r) ) != 0 )
	ZYPP_THROW( AugBadInsert( str(), label_r ) );
    }

    void insertBefore( const std::string & label_r )
    { insert( label_r, /*before*/true ); }

    void insertAfter( const std::string & label_r )
    { insert( label_r, /*before*/false );}
    //@}

  public:
    /** \name Remove all nodes matching path and their descendants.
     */
    //@{
    void rm()
    { ::aug_rm( aug(), c_str() ); }

    void rmBelow()
    { (*this+"/*").rm(); }
    //@}

  private:
    std::string _path;
  };

  /** \relates AugPath Stream output */
  inline std::ostream & operator<<( std::ostream & str_r, const AugPath & obj_r )
  {
    if ( ! obj_r.empty() )
    {
      str_r << obj_r.str();

      if ( unsigned m = obj_r.matches() )
      {
	if ( m == 1 )
	{
	  const char * valif = obj_r.getRaw();
	  if ( valif )
	    str_r << " -{" << valif << "}-";
	  else
	    str_r << " --";
	}
	else
	  str_r << " -AMBIGUOUS-";
      }
      else
	str_r << " -NOTINTREE-";
    }
    else
      str_r << "EMPTYAUGPATH";

    return str_r;
  }

  ///////////////////////////////////////////////////////////////////
  /// \brief \ref AugPath matches in the the augeas tree (AugPath container)
  struct AugMatches : public AugRef
  {
    AugMatches( const AugPath & path_r )
    : AugRef { path_r }
    , _d { new D( path_r ) }
    {}

  public:
    typedef AugPath value_type;

  private:
    struct MkValue : public AugRef
    {
      typedef value_type result_type;

      MkValue( const AugRef & aug_r )
      : AugRef { aug_r }
      {}

      value_type operator()( char * el_r ) const
      { return { *this, el_r }; }
    };

  public:
    typedef transform_iterator<MkValue,char**> const_iterator;

    explicit operator bool() const
    { return !empty(); }

    bool empty() const
    { return !_d->_cnt; }

    unsigned size() const
    { return _d->_cnt; }

    const_iterator begin() const
    { return make_transform_iterator( _d->_matches, MkValue( *this ) ); }

    const_iterator last() const
    { return make_transform_iterator( _d->_matches+(_d->_cnt ? _d->_cnt-1 : 0U ), MkValue( *this ) ); }

    const_iterator end() const
    { return make_transform_iterator( _d->_matches+_d->_cnt, MkValue( *this ) ); }

    value_type operator[]( unsigned idx_r ) const
    {
      if ( idx_r < _d->_cnt )
	return { *this, _d->_matches[idx_r] };
      ZYPP_THROW( AugBadMatchIndex( idx_r ) );
    }

  private:
    struct D
    {
      D( const AugPath & path_r )
      {
	int res = ::aug_match( path_r.aug(), path_r.c_str(), &_matches );
	if ( res == -1 )
	  ZYPP_THROW( AugBadPath( path_r.c_str() ) );
	_cnt = res;
      }

      ~D()
      {
	if ( _matches )
	{
	  while ( _cnt-- )
	    ::free( _matches[_cnt] );
	  ::free( _matches );
	}
      }

      unsigned		_cnt	{ 0U };
      char **		_matches{ nullptr };
    };
    boost::shared_ptr<D> _d;
  };

  /** \relates AugMatches Stream output */
  inline std::ostream & operator<<( std::ostream & str_r, const AugMatches & obj_r )
  { return dumpRange( str_r << "AugMatches ", obj_r.begin(), obj_r.end() ); }

  ///////////////////////////////////////////////////////////////////

  inline AugPath AugRef::augPath( std::string path_r ) const
  { return { *this, std::move(path_r) }; }


  inline AugPath AugPath::firstMatch() const
  {
    AugPath ret { augPath() };
    AugMatches m { *this };
    if ( ! m.empty() ) ret = *m.begin();
    return ret;
  }

  inline AugPath AugPath::lastMatch() const
  {
    AugPath ret { augPath() };
    AugMatches m { *this };
    if ( ! m.empty() ) ret = *m.last();
    return ret;
  }

  ///////////////////////////////////////////////////////////////////
  /// \brief IOmanip to dump an augeas (sub)tree
  struct AugTree
  {
    AugTree( const AugPath & root_r )
    : _root { root_r }
    {}

    const AugPath & root() const
    { return _root; }

  public:
    static std::ostream & recDumpOn( std::ostream & str_r, const AugPath & path_r )
    {
      for ( auto p : AugMatches( path_r ) )
      {
	str_r << "  " << p << endl;
	p.str() += "/*";
	recDumpOn( str_r, p );
      }
      return str_r;
    }

  private:
    const AugPath & _root;
  };

  /** \relates AugTree Stream output */
  inline std::ostream & operator<<( std::ostream & str_r, const AugTree & obj_r )
  {
    const AugPath & root { obj_r.root() };

    if ( root.empty() )
      str_r << "Augeas {";
    else
      str_r << "AugTree(" << root.str() << ") {";

    if ( root.aug() )
      AugTree::recDumpOn( str_r << endl, root.empty() ? root.augPath("/") : root );

    return str_r << "}";
  }

  /** \relates AugRef Stream output */
  inline std::ostream & operator<<( std::ostream & str_r, const AugRef & obj_r )
  { return str_r << AugTree( obj_r ); }
} // namespace
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
/// \class Augeas::Impl
/// \brief Augeas::Impl implementation
///////////////////////////////////////////////////////////////////
struct Augeas::Impl
{
  explicit Impl( ::augeas * take_r )
  : _aug { take_r }
  {}

  AugPath augPath( const Pathname & path_r ) const
  { return { _aug, path_r.asString() }; }

  AugPath augPath( const std::string path_r ) const
  { return { _aug, std::move(path_r) }; }

  AugPath augPath( const char * path_r ) const
  { return { _aug, path_r?path_r:"" }; }


  std::ostream & augLogERR() const
  {
    for ( const AugPath & aP : AugMatches( augPath("/augeas//error") ) )
      ERR << AugTree(aP) << endl;
    return ERR;
  }


  static std::pair<std::string,std::string> splitOption( const std::string & option_r )
  {
    std::vector<std::string> words;
    str::split( option_r, back_inserter(words), "/" );
    if ( words.size() != 2 || words[0].empty() || words[1].empty() )
      ZYPP_THROW( AugException( str::Format(_("Malformed option name '%1%'." ) ) % option_r ) );
    return { std::move(words[0]), std::move(words[1]) };
  }


  void saveCfg()
  {
    MIL << "Going to save pending config file changes..." << endl;

    if ( ::aug_save( _aug.aug() ) != 0 )
    {
      augLogERR() << "failed to save the tree" << endl;
      ZYPP_THROW( AugException(_("Could not save the config file.") ) );
    }
  }

public:
  AugRef _aug;	///< The reference to ::augeas

  std::vector<Pathname> _cfgFiles;	///< list of config files to load (higher prio first)
};

///////////////////////////////////////////////////////////////////
/// class Augeas
///////////////////////////////////////////////////////////////////

Augeas::Augeas( Pathname customcfg_r, bool readmode_r  )
: _pimpl { new Impl( ::aug_init( NULL, "/usr/local/share/zypper:/usr/share/zypper", AUG_NO_STDINC | AUG_NO_LOAD ) ) }
{
  MIL << "Going to read zypper config using Augeas..." << endl;

  // determine the config files to load
  if ( customcfg_r.empty() )
  {
    // add $HOME/.zypper.conf
    if ( const char * HOME = env::HOME() )
      _pimpl->_cfgFiles.push_back( Pathname(HOME) / ".zypper.conf" );
    else
      WAR << "Cannot figure out user's home directory. Skipping user's config." << endl;

    // add /etc/zypp/zypper.conf
    _pimpl->_cfgFiles.push_back( "/etc/zypp/zypper.conf" );
  }
  else
  {
    // set user supplied custom config file
    if ( customcfg_r.relative() )
    {
      const char * PWD = env::PWD();
      customcfg_r = (PWD ? PWD : "/") / customcfg_r;
    }

    PathInfo pi( customcfg_r );
    if ( pi.isExist() && ! pi.isFile() )
      ZYPP_THROW( AugException(str::Format(_("Config file '%1%' exists but is not a file." ) ) % customcfg_r ) );

    _pimpl->_cfgFiles.push_back( customcfg_r );
  }

  // load the config files
  {
    AugPath inclP { _pimpl->augPath("/augeas/load/ZYpper/incl") };
    inclP.rm();	// in case something's predefined in the lense
    inclP += "[last()+1]";

    for ( const Pathname & cfg : _pimpl->_cfgFiles )
    { inclP.set( cfg.c_str() ); }
  }

  if ( ::aug_load( _pimpl->_aug.aug() ) != 0 )
    ZYPP_THROW( AugException(_("Could not load the config files.") ) );

  // collect eventual errors
  for ( const Pathname & cfg : _pimpl->_cfgFiles )
  {
    AugPath fP { _pimpl->augPath("/augeas/files") += cfg.asString() };

    if ( fP.exists() )
    {
      fP += "/error";
      if ( fP.exists() )
      {
	_pimpl->augLogERR();
	std::string err { str::Format( "%1%\n%2%: " ) % cfg % fP.value() };
	if ( ! (fP+"/line").value().empty() )
	{
	  err += ( str::Format( "at %1%:%2%: " ) % (fP+"/line").value() % (fP+"/char").value() );
	}
	err += ( str::Format( "%1%" ) % (fP+"/message").value() );
	ZYPP_THROW( AugParseError( err ) );
      }
      MIL << "READ config file: " << cfg << endl;
    }
    else
    {
      if ( readmode_r && ! customcfg_r.empty() )
	Zypper::instance().out().warning( str::Format(_("Config file '%1%' does not exist." ) ) % cfg );
      WAR << "MISS config file: " << cfg << endl;
    }
  }
}

Augeas::~Augeas()
{}

Pathname Augeas::getSaveFile() const
{ return( _pimpl->_cfgFiles.empty() ? Pathname() : _pimpl->_cfgFiles[0] ); }

std::string Augeas::getOption( const std::string & option_r ) const
{
  std::string ret;

  for ( const Pathname & cfg : _pimpl->_cfgFiles )
  {
    AugPath fP { _pimpl->augPath( "/files"/cfg/option_r ) };
    try
    {
      unsigned matches = fP.matches();
      if ( matches == 1 )
      {
	DBG << fP << endl;
	ret = fP.value();
	break;
      }
      else if ( matches > 1 )
      {
	// translator: %1% is the path to a config file, %2% is the name of an options inside the file
	Zypper::instance().out().error( str::Format(_("%1%: Option '%2%' is defined multiple times. Using the last one.") ) % cfg % option_r );

	fP = *AugMatches(fP).last();
	WAR << fP << endl;
	ret = fP.value();
	break;
      }
    }
    catch ( const AugException & excpt )
    {
      ZYPP_CAUGHT( excpt );
    }
  }

  return ret;
}

void Augeas::setOption( const std::string & option_r, const std::string & value_r )
{
  const Pathname & cfg { getSaveFile() };
  if ( cfg.empty() )
    ZYPP_THROW( AugSaveError(_("No config file is in use.") ) );

  AugPath fP { _pimpl->augPath( "/files"/cfg/option_r ) };

  unsigned matches = fP.matches();
  if ( matches )
  {
    if ( matches > 1 )
    {
      AugPath last { fP.lastMatch() };
      fP.rm();
      fP = last;
    }
    fP.set( value_r );
  }
  else
  {
    // - perferably insert the new node after its commented node (if it exists)
    const std::pair<std::string,std::string> & sk { Impl::splitOption( option_r ) };
    AugPath secP { _pimpl->augPath( "/files"/cfg/sk.first ) };

    AugPath comP { secP };
    comP += "/#kv/";
    comP += sk.second;
    comP = comP.lastMatch();
    if ( comP )
    {
      comP += "/..";
      comP.insertAfter( sk.second );	// now fP matches
      fP.set( value_r );
    }
    else
    {
      // - Not intended, but be prepared for ambiguous parent (multiple sections with same name)
      fP = secP;
      fP += "[last()]/";
      fP += sk.second;
      fP.set( value_r );
    }
  }
  DBG << fP << endl;
}

void Augeas::save()
{ _pimpl->saveCfg(); }

std::ostream & operator<<( std::ostream & str_r, const Augeas & obj_r )
{ return str_r << obj_r._pimpl->_aug; }
