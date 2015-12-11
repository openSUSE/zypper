#include <sstream>
#include <map>
#include <vector>
#include <string>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>

using std::endl;
using namespace zypp;

enum tags {
  PARAGRAPH,
  PRE,
  BLOCKQUOTE,
  BOLD,
  UNDERLINED,
  ANCHOR,
  HEADER1,
  HEADER2,
  HEADER3,
  BREAK_LINE,
  EM,
  ITALIC,
  HR,
  LI,
  OL,
  UL,
  TT,
  QT,
  BIG,
  CODE,
  CENTER,
  //special for unknown tags
  UNKNOWN
};

std::map<std::string,tags> _rtTagmap;

bool pre;
bool ordered;
unsigned count_list_items;

void fillTagmap()
{
  _rtTagmap["p"] = PARAGRAPH;
  _rtTagmap["a"] = ANCHOR;
  _rtTagmap["b"] = BOLD;
  _rtTagmap["u"] = UNDERLINED;
  _rtTagmap["i"] = ITALIC;
  _rtTagmap["br"] = BREAK_LINE;
  _rtTagmap["em"] = EM;
  _rtTagmap["h1"] = HEADER1;
  _rtTagmap["h2"] = HEADER2;
  _rtTagmap["h3"] = HEADER3;
  _rtTagmap["hr"] = HR;
  _rtTagmap["li"] = LI;
  _rtTagmap["ol"] = OL;
  _rtTagmap["ul"] = UL;
  _rtTagmap["qt"] = QT;
  _rtTagmap["tt"] = TT;
  _rtTagmap["big"] = BIG;
  _rtTagmap["pre"] = PRE;
  _rtTagmap["bold"] = BOLD;
  _rtTagmap["code"] = CODE;
  _rtTagmap["font"] = UNKNOWN; //not parsed in parser
  _rtTagmap["large"] = UNKNOWN; //same as ncurses
  _rtTagmap["small"] = UNKNOWN; // same as necurses
  _rtTagmap["center"] = CENTER;
  _rtTagmap["strong"] = BOLD; // same as necurses
  _rtTagmap["blockquote"] = BLOCKQUOTE; // same as necurses

}

std::string closeTag( std::vector<tags>& tagStack )
{
  if( tagStack.empty() )
  {
    WAR << "closing tag before any opening" << endl;;
    return "";
  }
  tags t = tagStack.back();
  tagStack.pop_back();
  switch ( t )
  {
    case PARAGRAPH:
      return "\n\n";
    case LI:
      return "\n";
    case PRE:
      pre = false; //fall thrue
    default:
      return "";
  }
}

std::string openTag( std::vector<tags>& tagStack, std::string & tag )
{
  tag = str::trim(tag);
  std::map<std::string,tags>::const_iterator it = _rtTagmap.find( tag );
  tags t;
  if ( it == _rtTagmap.end() )
  {
    if ( tag.size() > 3 && tag[0] == '!' && tag[1] == '-' && tag[2] == '-' )
      return ""; //comment
    WAR << "unknown rich text tag " << tag << endl;
    t = UNKNOWN;
  }
  else
  {
    t = it->second;
  }
  tagStack.push_back( t );
  switch ( t )
  {
    case HR:
      tagStack.pop_back(); //hr haven't closing tag
      return "--------------------";

    case PARAGRAPH:
      return "";
    case BREAK_LINE:
      tagStack.pop_back(); //br haven't closing tag
      return "\n";
    case OL:
      ordered = true;
      count_list_items = 0;
      return "\n";
    case UL:
      ordered = false;
      return "\n";
    case LI:
      if ( ordered )
      {
        std::ostringstream res;
        res << ++count_list_items << ") ";
        return res.str();
      }
      else
      {
        return "- ";
      }
    case PRE:
      pre = true; //fall thrue
    default:
      return "";
  }
}

std::map<std::string,std::string> ampersmap;

void fillAmpersmap()
{
  ampersmap["gt"]	=">";
  ampersmap["lt"]	="<";
  ampersmap["amp"]	="&";
  ampersmap["quot"]	="\"";
  ampersmap["nbsp"]	=" "; //TODO REAL NBSP
  ampersmap["product"]	="product"; //TODO replace with real name
}

std::string getStringFromAmpr( const std::string & str )
{
  if ( ampersmap.empty() )
    fillAmpersmap();

  std::string::size_type end = str.find( ';' );
  DBG << "val ampr is: " << str << endl;
  if ( str[0] == '#' ) //first is value
  {
    int res = 0;
    std::istringstream sstr( str.substr( 1, end ) );
    sstr >> res;
    DBG << res << endl;
    if ( res != 0 )
    {
      return std::string( 1,(char)res ); //return char
    }
    else
    {
      WAR << "unknown number " << str << endl;
      return "";
    }
  }

  DBG << end << " " << str.substr( 0, end ) << endl;
  return ampersmap[str.substr( 0, end )];

}

std::string processRichText( const std::string& text )
{
  if ( _rtTagmap.empty() )
    fillTagmap();
  //state machine vars
  pre = false;

  std::vector<tags> tagStack;

  std::string res;
  res.reserve( text.size() );
  std::string::size_type pos = 0;
  do {
    switch( text[pos] )
    {
      case ' ':
      case '\n':
      case '\t':
      case '\v':
      case '\r':
        if ( pre )
          res.push_back( text[pos] );
        else
        {
          if ( text[pos] == ' ' )
            res.push_back( ' ' );
        }
        break;
      case '<':
        if ( pos+1 == text.npos )
        {
          WAR << "ended with nonclosed tag."<< endl;
          return res; //chyba, tohle by se nemelo stavat
        }
        if ( text[pos+1] == '/' ) //close tag
        {
          pos = text.find( '>', pos );
          res.append( closeTag( tagStack ) );
        }
        else
        {
          std::string::size_type tagEndPos = text.find( '>', pos );
          if ( tagEndPos == text.npos )
          {
            WAR << "ended with non-closed tag " << endl;
            return res;
          }
          std::string tagname( text.substr( pos+1, tagEndPos-pos-1 ) );
          pos = tagEndPos;
          res.append( openTag( tagStack, tagname ) );
        }
        break;
      case '&':
      {
        std::string::size_type semipos = text.find( ';', pos );
        std::string tmp = getStringFromAmpr( text.substr( pos+1, pos-semipos-1 ) );
        DBG << "tmp is: " << tmp << endl;
        res.append( tmp );
        pos = semipos;
        break;
      }
      default:
        res.push_back( text[pos] );
    }

    ++pos;
  } while ( pos != text.size() );
  return res;
}

