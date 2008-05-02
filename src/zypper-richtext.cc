#include <sstream>
#include <map>
#include <vector>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"

using namespace std;

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

std::map<string,tags> _RTtagmap;

bool pre;
bool ordered;
unsigned count_list_items;

void fillTagmap()
{
  _RTtagmap["p"] = PARAGRAPH;
  _RTtagmap["a"] = ANCHOR;
  _RTtagmap["b"] = BOLD;
  _RTtagmap["u"] = UNDERLINED;
  _RTtagmap["i"] = ITALIC;
  _RTtagmap["br"] = BREAK_LINE;
  _RTtagmap["em"] = EM;
  _RTtagmap["h1"] = HEADER1;
  _RTtagmap["h2"] = HEADER2;
  _RTtagmap["h3"] = HEADER3;
  _RTtagmap["hr"] = HR;
  _RTtagmap["li"] = LI;
  _RTtagmap["ol"] = OL;
  _RTtagmap["ul"] = UL;
  _RTtagmap["qt"] = QT;
  _RTtagmap["tt"] = TT;
  _RTtagmap["big"] = BIG;
  _RTtagmap["pre"] = PRE;
  _RTtagmap["bold"] = BOLD;
  _RTtagmap["code"] = CODE;
  _RTtagmap["font"] = UNKNOWN; //not parsed in parser
  _RTtagmap["large"] = UNKNOWN; //same as ncurses
  _RTtagmap["small"] = UNKNOWN; // same as necurses
  _RTtagmap["center"] = CENTER;
  _RTtagmap["strong"] = BOLD; // same as necurses
  _RTtagmap["blockquote"] = BLOCKQUOTE; // same as necurses
  
}

string closeTag(vector<tags>& tagStack)
{
  if(tagStack.empty())
  {
    WAR << "closing tag before any opening" << endl;;
    return "";
  }
  tags t = tagStack.back();
  tagStack.pop_back();
  switch(t)
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

string openTag(vector<tags>& tagStack, string& tag)
{
  tag = zypp::str::trim(tag);
  std::map<string,tags>::const_iterator it = _RTtagmap.find(tag);
  tags t;
  if (it == _RTtagmap.end())
  {
    if (tag.size()>3 && tag[0]=='!' && tag[1]=='-' && tag[2]=='-')
      return ""; //comment
    WAR << "unknown rich text tag "<<tag << endl;
    t = UNKNOWN;
  }
  else
  {
    t = it->second;
  }
  tagStack.push_back(t);
  switch(t)
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
      if (ordered)
      {
        ostringstream res;
        res << ++count_list_items << ") ";
        res.flush();
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

std::map<string,string> ampersmap;

void fillAmpersmap()
{
  ampersmap["gt"]=">";
  ampersmap["lt"]="<";
  ampersmap["amp"]="&";
  ampersmap["quot"]="\"";
  ampersmap["nbsp"]=" "; //TODO REAL NBSP
  ampersmap["product"]="product"; //TODO replace with real name
}

string getStringFromAmpr(const string& str)
{
  if (ampersmap.empty())
    fillAmpersmap();

  string::size_type end = str.find(';');
  DBG << "val ampr is: " << str << endl;
  if (str[0] == '#') //first is value
  {
    int res = 0;
    istringstream sstr(str.substr(1,end));
    sstr >> res;
    DBG << res << endl;
    if (res!=0)
    {
      return string(1,(char)res); //return char
    }
    else
    {
      WAR << "unknown number " << str << endl;
      return "";
    }
  }
  
  DBG << end <<" "<<str.substr(0,end) << endl;
  return ampersmap[str.substr(0,end)];

}

std::string processRichText(const std::string& text)
{
  if (_RTtagmap.empty())
    fillTagmap();
  //state machine vars
  pre = false;

  vector<tags> tagStack;

  string res("");
  res.reserve(text.size());
  string::size_type pos = 0;
  do {
    switch( text[pos] )
    {
      case ' ':
      case '\n':
      case '\t':
      case '\v':
      case '\r':
        if (pre)
          res.push_back(text[pos]);
        else
        {
          if (text[pos]==' ')
            res.push_back(' ');
        }
        break;
      case '<':
        if (pos+1==text.npos)
        {
          WAR << "ended with nonclosed tag."<< endl;
          return res; //chyba, tohle by se nemelo stavat
        }
        if (text[pos+1]=='/') //close tag
        {
          pos = text.find('>',pos);
          res.append(closeTag(tagStack));
        }
        else
        {
          string::size_type tagEndPos = text.find('>',pos);
          if(tagEndPos==text.npos)
          {
            WAR << "ended with non-closed tag " << endl;
            return res; 
          }
          string tagname = text.substr(pos+1,tagEndPos-pos-1);
          pos = tagEndPos;
          res.append(openTag(tagStack,tagname));
        }
        break;
      case '&':
      {
        string::size_type semipos = text.find(';',pos);
        string tmp = getStringFromAmpr(text.substr(pos+1,pos-semipos-1));
        DBG << "tmp is: " << tmp << endl;
        res.append(tmp);
        pos = semipos;
        break;
      }
      default:
        res.push_back(text[pos]);
    }

    ++pos;
  } while (pos!=text.size());
  return res;
}

