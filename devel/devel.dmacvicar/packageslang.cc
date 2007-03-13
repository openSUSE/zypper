class ParsePackagesLang
{
public:
  struct Entry : public NVRA
  {
    friend std::ostream & operator<<( std::ostream & str, const  Entry & obj );
    TagCacheRetrievalPtr retrieval;
    TagRetrievalPos posSUMMARY;
    TagRetrievalPos posDESCRIPTION;
    TagRetrievalPos posINSNOTIFY;
    TagRetrievalPos posDELNOTIFY;
    TagRetrievalPos posLICENSETOCONFIRM;

    Entry() {}
    Entry( const NVRA & pkgident_r, TagCacheRetrievalPtr retrieval_r )
      : NVRA( pkgident_r ), retrieval( retrieval_r )
    {}
  };

  private:

  // Tag ids for the TaggedParser
  enum Tags
  {
    PACKAGE,          // name version release arch
    SUMMARY,          // short summary (label)
    DESCRIPTION,      // long description
    INSNOTIFY,  // install notification
    DELNOTIFY,  // delete notification
    LICENSETOCONFIRM, // license to confirm upon install
    // LAST ENTRY:
    NUM_TAGS
  };

private:

  const Pathname _file;
  std::ifstream  _stream;
  std::string    _version;

  TaggedParser         _parser;
  TaggedFile::TagSet   _tagset;
  TagCacheRetrievalPtr _retrieval;

private:

  void getData( Entry & entry_r );

public:

  ParsePackagesLang( const Pathname & file_r );
  ~ParsePackagesLang();

  TagCacheRetrievalPtr getRetrieval() { return _retrieval; }

  TaggedFile::assignstatus getEntry( Entry & entry_r );

  Entry noEntry() const { return Entry( NVRA(), _retrieval ); }
};

ParsePackagesLang::ParsePackagesLang( const Pathname & file_r )
    : _file( file_r )
{
  // Check an open file
  PathInfo file( _file );
  if ( ! file.isFile() ) {
    WAR << "No File: " << file << endl;
    return;
  }

  _stream.open( file.path().asString().c_str() );
  if ( ! _stream.is_open() ) {
    WAR << "Can't open" << file << endl;
    return;
  }

  // Find initial version tag
  TaggedParser::TagType type = _parser.lookupTag( _stream );
  if ( type != TaggedParser::SINGLE
       || _parser.currentTag() != "Ver"
       || ! _parser.currentLocale().empty() ) {
    WAR << "Initial '=Ver:' tag missing in " << file << endl;
    return;
  }

  _version = _parser.data();
  if ( _version != "2.0" ) {
    WAR << "Version '" << _version << "' != 2.0 in " << file << endl;
    return;
  }

  // initialize tagset
  _tagset.setAllowMultipleSets( true ); // multiple tagsets per file
  _tagset.setAllowUnknownTags( true );  // skip unknown tags

  // Using loop and switch to get a compiler warning, if tags are
  // defined but uninitialized, or vice versa.
  for ( Tags tag = Tags(0); tag < NUM_TAGS; tag = Tags(tag+1) )
  {
    switch ( tag )
    {
#define DEFTAG(T,ARGS) case T: _tagset.addTag ARGS; break
      DEFTAG( PACKAGE,    ( "Pkg", tag, TaggedFile::SINGLE, TaggedFile::START ) );

      DEFTAG( SUMMARY,    ( "Sum", tag, TaggedFile::SINGLE ) );
      DEFTAG( DESCRIPTION,  ( "Des", tag, TaggedFile::MULTI ) );
      DEFTAG( INSNOTIFY,  ( "Ins", tag,   TaggedFile::MULTI ) );
      DEFTAG( DELNOTIFY,  ( "Del", tag,   TaggedFile::MULTI ) );
      DEFTAG( LICENSETOCONFIRM, ( "Eul", tag,   TaggedFile::MULTI ) );

#undef DEFTAG
      // No default: let compiler warn missing enumeration values
    case NUM_TAGS: break;
    }
  }

  // Everything ok so far
  _retrieval = new TagCacheRetrieval( file.path() );
  MIL << _file << " ready to parse." << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//  METHOD NAME : ParsePackagesLang::~ParsePackagesLang
//  METHOD TYPE : Destructor
//
ParsePackagesLang::~ParsePackagesLang()
{
}

///////////////////////////////////////////////////////////////////
//
//
//  METHOD NAME : ParsePackagesLang::getEntry
//  METHOD TYPE : TaggedFile::assignstatus
//
TaggedFile::assignstatus ParsePackagesLang::getEntry( Entry & entry_r ) {

  entry_r = Entry();
  TaggedFile::assignstatus ret = _tagset.assignSet( _parser, _stream );

  if ( ret == TaggedFile::ACCEPTED_FULL ) {

    getData( entry_r );

  } else if ( ret == TaggedFile::REJECTED_EOF ) {

    MIL << _file << " at EOF" << endl;

  } else {

    ERR << _file << "(" << _parser.lineNumber() << "): error "
      << ret << ", last tag read: " << _parser.currentTag();
    if ( ! _parser.currentLocale().empty() )
      ERR << "." << _parser.currentLocale();
    ERR << endl;

  }

  return ret;
}

///////////////////////////////////////////////////////////////////
//
//
//  METHOD NAME : ParsePackagesLang::getData
//  METHOD TYPE : void
//
void ParsePackagesLang::getData( Entry & entry_r )
{
  string ident( _tagset.getTagByIndex(PACKAGE)->Data() );
  if ( ident.empty() ) {
    ERR << _file << "(" << _parser.lineNumber() << ") No '=Pkg' value found" << endl;
    return;
  }

  std::vector<std::string> splitted;
  str::split (ident, std::back_inserter(splitted) );

  if ( splitted.size() != 4 )
  {
    ERR << _file << "(" << _parser.lineNumber() << ") Illegal '=Pkg' value '" << ident << "'" << endl;
    return;
  }

  entry_r = Entry( NVRA( splitted[0],
           Edition( splitted[1], splitted[2] ),
           Arch( splitted[3] ) ),
       _retrieval );

  entry_r.posSUMMARY          = _tagset.getTagByIndex( SUMMARY )->Pos();
  entry_r.posDESCRIPTION      = _tagset.getTagByIndex( DESCRIPTION )->Pos();
  entry_r.posINSNOTIFY        = _tagset.getTagByIndex( INSNOTIFY )->Pos();
  entry_r.posDELNOTIFY        = _tagset.getTagByIndex( DELNOTIFY )->Pos();
  entry_r.posLICENSETOCONFIRM = _tagset.getTagByIndex( LICENSETOCONFIRM )->Pos();
}

/******************************************************************
**
**
**  FUNCTION NAME : operator<<
**  FUNCTION TYPE : std::ostream &
*/
ostream & operator<<( ostream & str, const ParsePackagesLang::Entry & obj )
{
//   str << obj.name() << endl;
//   str << "    SUM " << obj.posSUMMARY << endl;
//   str << "    DES " << obj.posDESCRIPTION << endl;
//   str << "    INY " << obj.posINSNOTIFY << endl;
//   str << "    DNY " << obj.posDELNOTIFY << endl;
//   str << "    EUL " << obj.posLICENSETOCONFIRM;

  return str;
}

/******************************************************************
**
**
**  FUNCTION NAME : operator<<
**  FUNCTION TYPE : ostream &
*/
ostream & operator<<( ostream & str, const ParsePackagesLang & obj )
{
  //str << obj._file << " (" << obj._retrieval << ")";
  return str;
}
