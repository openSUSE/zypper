/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                    (C) SuSE Linux AG |
\----------------------------------------------------------------------/

  File:       ISOLanguage.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Textdomain "iso-languages"

/-*/

#define TEXTDOMAIN "iso-languages"
#define _(MSG)  dgettext(TEXTDOMAIN, (MSG))
#define __(MSG) MSG

#include <iostream>
#include <map>
#include <libintl.h>

#include <y2util/Y2SLog.h>
#include <y2util/stringutil.h>

#include <y2util/ISOLanguage.h>

using namespace std;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ISOLanguage::_D
/**
 *
 **/
struct ISOLanguage::_D : public Rep {

  typedef map<string,string>      CodeMap;
  typedef CodeMap::const_iterator Index;

  // CodeMap[code] = untranslated language name
  // Translation is done in name().
  static CodeMap _iso639_1_CodeMap;
  static CodeMap _iso639_2_CodeMap;
  static CodeMap _others_CodeMap;

  static void setDefaultCodeMaps( CodeMap & iso639_1,
                                  CodeMap & iso639_2,
                                  CodeMap & others );

  static const string _noCode;
  static const string _noName;

  Index _index;

  void _assertInitCodemaps() {
    if ( _others_CodeMap.empty() ) {
      setDefaultCodeMaps( _iso639_1_CodeMap,
                          _iso639_2_CodeMap,
                          _others_CodeMap );
    }
  }

  Index _lookupCode( const std::string & code_r ) {
    _assertInitCodemaps();
    if ( code_r.size() == 2 )
      {
        CodeMap::iterator it = _iso639_1_CodeMap.find( code_r );
        if ( it != _iso639_1_CodeMap.end() )
          {
            return it;
          }
        // else check _others_CodeMap
      }
    if ( code_r.size() == 3 )
      {
        CodeMap::iterator it = _iso639_2_CodeMap.find( code_r );
        if ( it != _iso639_2_CodeMap.end() )
          {
            return it;
          }
        // else check _others_CodeMap
      }
    return _others_CodeMap.find( code_r );
  }

  Index _assert( const std::string & code_r ) {
    Index it = _lookupCode( code_r );
    if ( it != _others_CodeMap.end() )
      {
        return it;
      }

    // Still here? Remember a new code
    CodeMap::value_type nval( code_r, string() );

    if ( code_r.size() != 2 && code_r.size() != 3 ) {
      WAR << "Malformed ISOLanguage code '" << code_r << "' (expect 2 or 3-letter)" << endl;
    }

    string lcode( stringutil::toLower( code_r ) );
    if ( lcode != code_r ) {
      WAR << "Malformed ISOLanguage code '" << code_r << "' (not lower case)" << endl;
      // but maybe we're lucky with the lower case code
      // and find a language name.
      it = _lookupCode( lcode );
      if ( it != _others_CodeMap.end() ) {
	nval.second = it->second;
      }
    }

    MIL << "Remember language code '" << code_r << "': '" << nval.second << "'" << endl;
    return _others_CodeMap.insert( nval ).first;
  }

  _D( const std::string & code_r )
    : _index( _assert(code_r) )
  {}

  string code() const {
    return _index->first;
  }

  string name() const {
    if ( _index->second.empty() )
      {
        string ret( _("Unknown language: ") );
        ret += "'";
        ret += _index->first;
        ret += "'";
        return ret;
      }
    return _( _index->second.c_str() );
  }
};

ISOLanguage::_D::CodeMap ISOLanguage::_D::_iso639_1_CodeMap;
ISOLanguage::_D::CodeMap ISOLanguage::_D::_iso639_2_CodeMap;
ISOLanguage::_D::CodeMap ISOLanguage::_D::_others_CodeMap;

const string ISOLanguage::_D::_noCode( "" );
const string ISOLanguage::_D::_noName( "Default" );

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOLanguage::ISOLanguage
//	METHOD TYPE : Constructor
//
ISOLanguage::ISOLanguage()
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOLanguage::ISOLanguage
//	METHOD TYPE : Constructor
//
ISOLanguage::ISOLanguage( const std::string & code_r )
{
  if ( code_r.size() ) {
    _d = makeVarPtr( new _D( code_r ) );
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOLanguage::~ISOLanguage
//	METHOD TYPE : Destructor
//
ISOLanguage::~ISOLanguage()
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOLanguage::isSet
//	METHOD TYPE : bool
//
bool ISOLanguage::isSet() const
{
  return _d;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOLanguage::code
//	METHOD TYPE : std::string
//
std::string ISOLanguage::code() const
{
  return _d ? _d->code() : _D::_noCode;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOLanguage::name
//	METHOD TYPE : std::string
//
std::string ISOLanguage::name() const
{
  return _d ? _d->name() : _D::_noName;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
*/
ostream & operator<<( ostream & str, const ISOLanguage & obj )
{
  return str << obj.code();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOLanguage::_D::setDefaultCodeMaps
//	METHOD TYPE : void
//
//      http://www.loc.gov/standards/iso639-2/ISO-639-2_values_8bits.txt
//
void ISOLanguage::_D::setDefaultCodeMaps( ISOLanguage::_D::CodeMap & iso639_1,
                                          ISOLanguage::_D::CodeMap & iso639_2,
                                          ISOLanguage::_D::CodeMap & others )
{
  bindtextdomain( TEXTDOMAIN, LOCALEDIR );
  bind_textdomain_codeset( TEXTDOMAIN, "UTF-8" );

  others["default"] = __( "Default" );

  // language code: aar aa
  iso639_2["aar"]                   = iso639_1["aa"] = __( "Afar" );
  // language code: abk ab
  iso639_2["abk"]                   = iso639_1["ab"] = __( "Abkhazian" );
  // language code: ace
  iso639_2["ace"]                                    = __( "Achinese" );
  // language code: ach
  iso639_2["ach"]                                    = __( "Acoli" );
  // language code: ada
  iso639_2["ada"]                                    = __( "Adangme" );
  // language code: ady
  iso639_2["ady"]                                    = __( "Adyghe" );
  // language code: afa
  iso639_2["afa"]                                    = __( "Afro-Asiatic (Other)" );
  // language code: afh
  iso639_2["afh"]                                    = __( "Afrihili" );
  // language code: afr af
  iso639_2["afr"]                   = iso639_1["af"] = __( "Afrikaans" );
  // language code: ain
  iso639_2["ain"]                                    = __( "Ainu" );
  // language code: aka ak
  iso639_2["aka"]                   = iso639_1["ak"] = __( "Akan" );
  // language code: akk
  iso639_2["akk"]                                    = __( "Akkadian" );
  // language code: alb sqi sq
  iso639_2["alb"] = iso639_2["sqi"] = iso639_1["sq"] = __( "Albanian" );
  // language code: ale
  iso639_2["ale"]                                    = __( "Aleut" );
  // language code: alg
  iso639_2["alg"]                                    = __( "Algonquian languages" );
  // language code: alt
  iso639_2["alt"]                                    = __( "Southern Altai" );
  // language code: amh am
  iso639_2["amh"]                   = iso639_1["am"] = __( "Amharic" );
  // language code: ang
  iso639_2["ang"]                                    = __( "English, Old (ca.450-1100)" );
  // language code: apa
  iso639_2["apa"]                                    = __( "Apache languages" );
  // language code: ara ar
  iso639_2["ara"]                   = iso639_1["ar"] = __( "Arabic" );
  // language code: arc
  iso639_2["arc"]                                    = __( "Aramaic" );
  // language code: arg an
  iso639_2["arg"]                   = iso639_1["an"] = __( "Aragonese" );
  // language code: arm hye hy
  iso639_2["arm"] = iso639_2["hye"] = iso639_1["hy"] = __( "Armenian" );
  // language code: arn
  iso639_2["arn"]                                    = __( "Araucanian" );
  // language code: arp
  iso639_2["arp"]                                    = __( "Arapaho" );
  // language code: art
  iso639_2["art"]                                    = __( "Artificial (Other)" );
  // language code: arw
  iso639_2["arw"]                                    = __( "Arawak" );
  // language code: asm as
  iso639_2["asm"]                   = iso639_1["as"] = __( "Assamese" );
  // language code: ast
  iso639_2["ast"]                                    = __( "Asturian" );
  // language code: ath
  iso639_2["ath"]                                    = __( "Athapascan languages" );
  // language code: aus
  iso639_2["aus"]                                    = __( "Australian languages" );
  // language code: ava av
  iso639_2["ava"]                   = iso639_1["av"] = __( "Avaric" );
  // language code: ave ae
  iso639_2["ave"]                   = iso639_1["ae"] = __( "Avestan" );
  // language code: awa
  iso639_2["awa"]                                    = __( "Awadhi" );
  // language code: aym ay
  iso639_2["aym"]                   = iso639_1["ay"] = __( "Aymara" );
  // language code: aze az
  iso639_2["aze"]                   = iso639_1["az"] = __( "Azerbaijani" );
  // language code: bad
  iso639_2["bad"]                                    = __( "Banda" );
  // language code: bai
  iso639_2["bai"]                                    = __( "Bamileke languages" );
  // language code: bak ba
  iso639_2["bak"]                   = iso639_1["ba"] = __( "Bashkir" );
  // language code: bal
  iso639_2["bal"]                                    = __( "Baluchi" );
  // language code: bam bm
  iso639_2["bam"]                   = iso639_1["bm"] = __( "Bambara" );
  // language code: ban
  iso639_2["ban"]                                    = __( "Balinese" );
  // language code: baq eus eu
  iso639_2["baq"] = iso639_2["eus"] = iso639_1["eu"] = __( "Basque" );
  // language code: bas
  iso639_2["bas"]                                    = __( "Basa" );
  // language code: bat
  iso639_2["bat"]                                    = __( "Baltic (Other)" );
  // language code: bej
  iso639_2["bej"]                                    = __( "Beja" );
  // language code: bel be
  iso639_2["bel"]                   = iso639_1["be"] = __( "Belarusian" );
  // language code: bem
  iso639_2["bem"]                                    = __( "Bemba" );
  // language code: ben bn
  iso639_2["ben"]                   = iso639_1["bn"] = __( "Bengali" );
  // language code: ber
  iso639_2["ber"]                                    = __( "Berber (Other)" );
  // language code: bho
  iso639_2["bho"]                                    = __( "Bhojpuri" );
  // language code: bih bh
  iso639_2["bih"]                   = iso639_1["bh"] = __( "Bihari" );
  // language code: bik
  iso639_2["bik"]                                    = __( "Bikol" );
  // language code: bin
  iso639_2["bin"]                                    = __( "Bini" );
  // language code: bis bi
  iso639_2["bis"]                   = iso639_1["bi"] = __( "Bislama" );
  // language code: bla
  iso639_2["bla"]                                    = __( "Siksika" );
  // language code: bnt
  iso639_2["bnt"]                                    = __( "Bantu (Other)" );
  // language code: bos bs
  iso639_2["bos"]                   = iso639_1["bs"] = __( "Bosnian" );
  // language code: bra
  iso639_2["bra"]                                    = __( "Braj" );
  // language code: bre br
  iso639_2["bre"]                   = iso639_1["br"] = __( "Breton" );
  // language code: btk
  iso639_2["btk"]                                    = __( "Batak (Indonesia)" );
  // language code: bua
  iso639_2["bua"]                                    = __( "Buriat" );
  // language code: bug
  iso639_2["bug"]                                    = __( "Buginese" );
  // language code: bul bg
  iso639_2["bul"]                   = iso639_1["bg"] = __( "Bulgarian" );
  // language code: bur mya my
  iso639_2["bur"] = iso639_2["mya"] = iso639_1["my"] = __( "Burmese" );
  // language code: byn
  iso639_2["byn"]                                    = __( "Blin" );
  // language code: cad
  iso639_2["cad"]                                    = __( "Caddo" );
  // language code: cai
  iso639_2["cai"]                                    = __( "Central American Indian (Other)" );
  // language code: car
  iso639_2["car"]                                    = __( "Carib" );
  // language code: cat ca
  iso639_2["cat"]                   = iso639_1["ca"] = __( "Catalan" );
  // language code: cau
  iso639_2["cau"]                                    = __( "Caucasian (Other)" );
  // language code: ceb
  iso639_2["ceb"]                                    = __( "Cebuano" );
  // language code: cel
  iso639_2["cel"]                                    = __( "Celtic (Other)" );
  // language code: cha ch
  iso639_2["cha"]                   = iso639_1["ch"] = __( "Chamorro" );
  // language code: chb
  iso639_2["chb"]                                    = __( "Chibcha" );
  // language code: che ce
  iso639_2["che"]                   = iso639_1["ce"] = __( "Chechen" );
  // language code: chg
  iso639_2["chg"]                                    = __( "Chagatai" );
  // language code: chi zho zh
  iso639_2["chi"] = iso639_2["zho"] = iso639_1["zh"] = __( "Chinese" );
  // language code: chk
  iso639_2["chk"]                                    = __( "Chuukese" );
  // language code: chm
  iso639_2["chm"]                                    = __( "Mari" );
  // language code: chn
  iso639_2["chn"]                                    = __( "Chinook jargon" );
  // language code: cho
  iso639_2["cho"]                                    = __( "Choctaw" );
  // language code: chp
  iso639_2["chp"]                                    = __( "Chipewyan" );
  // language code: chr
  iso639_2["chr"]                                    = __( "Cherokee" );
  // language code: chu cu
  iso639_2["chu"]                   = iso639_1["cu"] = __( "Church Slavic" );
  // language code: chv cv
  iso639_2["chv"]                   = iso639_1["cv"] = __( "Chuvash" );
  // language code: chy
  iso639_2["chy"]                                    = __( "Cheyenne" );
  // language code: cmc
  iso639_2["cmc"]                                    = __( "Chamic languages" );
  // language code: cop
  iso639_2["cop"]                                    = __( "Coptic" );
  // language code: cor kw
  iso639_2["cor"]                   = iso639_1["kw"] = __( "Cornish" );
  // language code: cos co
  iso639_2["cos"]                   = iso639_1["co"] = __( "Corsican" );
  // language code: cpe
  iso639_2["cpe"]                                    = __( "Creoles and pidgins, English based (Other)" );
  // language code: cpf
  iso639_2["cpf"]                                    = __( "Creoles and pidgins, French-based (Other)" );
  // language code: cpp
  iso639_2["cpp"]                                    = __( "Creoles and pidgins, Portuguese-based (Other)" );
  // language code: cre cr
  iso639_2["cre"]                   = iso639_1["cr"] = __( "Cree" );
  // language code: crh
  iso639_2["crh"]                                    = __( "Crimean Tatar" );
  // language code: crp
  iso639_2["crp"]                                    = __( "Creoles and pidgins (Other)" );
  // language code: csb
  iso639_2["csb"]                                    = __( "Kashubian" );
  // language code: cus
  iso639_2["cus"]                                    = __( "Cushitic (Other)" );
  // language code: cze ces cs
  iso639_2["cze"] = iso639_2["ces"] = iso639_1["cs"] = __( "Czech" );
  // language code: dak
  iso639_2["dak"]                                    = __( "Dakota" );
  // language code: dan da
  iso639_2["dan"]                   = iso639_1["da"] = __( "Danish" );
  // language code: dar
  iso639_2["dar"]                                    = __( "Dargwa" );
  // language code: day
  iso639_2["day"]                                    = __( "Dayak" );
  // language code: del
  iso639_2["del"]                                    = __( "Delaware" );
  // language code: den
  iso639_2["den"]                                    = __( "Slave (Athapascan)" );
  // language code: dgr
  iso639_2["dgr"]                                    = __( "Dogrib" );
  // language code: din
  iso639_2["din"]                                    = __( "Dinka" );
  // language code: div dv
  iso639_2["div"]                   = iso639_1["dv"] = __( "Divehi" );
  // language code: doi
  iso639_2["doi"]                                    = __( "Dogri" );
  // language code: dra
  iso639_2["dra"]                                    = __( "Dravidian (Other)" );
  // language code: dsb
  iso639_2["dsb"]                                    = __( "Lower Sorbian" );
  // language code: dua
  iso639_2["dua"]                                    = __( "Duala" );
  // language code: dum
  iso639_2["dum"]                                    = __( "Dutch, Middle (ca.1050-1350)" );
  // language code: dut nld nl
  iso639_2["dut"] = iso639_2["nld"] = iso639_1["nl"] = __( "Dutch" );
  // language code: dyu
  iso639_2["dyu"]                                    = __( "Dyula" );
  // language code: dzo dz
  iso639_2["dzo"]                   = iso639_1["dz"] = __( "Dzongkha" );
  // language code: efi
  iso639_2["efi"]                                    = __( "Efik" );
  // language code: egy
  iso639_2["egy"]                                    = __( "Egyptian (Ancient)" );
  // language code: eka
  iso639_2["eka"]                                    = __( "Ekajuk" );
  // language code: elx
  iso639_2["elx"]                                    = __( "Elamite" );
  // language code: eng en
  iso639_2["eng"]                   = iso639_1["en"] = __( "English" );
  // language code: enm
  iso639_2["enm"]                                    = __( "English, Middle (1100-1500)" );
  // language code: epo eo
  iso639_2["epo"]                   = iso639_1["eo"] = __( "Esperanto" );
  // language code: est et
  iso639_2["est"]                   = iso639_1["et"] = __( "Estonian" );
  // language code: ewe ee
  iso639_2["ewe"]                   = iso639_1["ee"] = __( "Ewe" );
  // language code: ewo
  iso639_2["ewo"]                                    = __( "Ewondo" );
  // language code: fan
  iso639_2["fan"]                                    = __( "Fang" );
  // language code: fao fo
  iso639_2["fao"]                   = iso639_1["fo"] = __( "Faroese" );
  // language code: fat
  iso639_2["fat"]                                    = __( "Fanti" );
  // language code: fij fj
  iso639_2["fij"]                   = iso639_1["fj"] = __( "Fijian" );
  // language code: fil
  iso639_2["fil"]                                    = __( "Filipino" );
  // language code: fin fi
  iso639_2["fin"]                   = iso639_1["fi"] = __( "Finnish" );
  // language code: fiu
  iso639_2["fiu"]                                    = __( "Finno-Ugrian (Other)" );
  // language code: fon
  iso639_2["fon"]                                    = __( "Fon" );
  // language code: fre fra fr
  iso639_2["fre"] = iso639_2["fra"] = iso639_1["fr"] = __( "French" );
  // language code: frm
  iso639_2["frm"]                                    = __( "French, Middle (ca.1400-1600)" );
  // language code: fro
  iso639_2["fro"]                                    = __( "French, Old (842-ca.1400)" );
  // language code: fry fy
  iso639_2["fry"]                   = iso639_1["fy"] = __( "Frisian" );
  // language code: ful ff
  iso639_2["ful"]                   = iso639_1["ff"] = __( "Fulah" );
  // language code: fur
  iso639_2["fur"]                                    = __( "Friulian" );
  // language code: gaa
  iso639_2["gaa"]                                    = __( "Ga" );
  // language code: gay
  iso639_2["gay"]                                    = __( "Gayo" );
  // language code: gba
  iso639_2["gba"]                                    = __( "Gbaya" );
  // language code: gem
  iso639_2["gem"]                                    = __( "Germanic (Other)" );
  // language code: geo kat ka
  iso639_2["geo"] = iso639_2["kat"] = iso639_1["ka"] = __( "Georgian" );
  // language code: ger deu de
  iso639_2["ger"] = iso639_2["deu"] = iso639_1["de"] = __( "German" );
  // language code: gez
  iso639_2["gez"]                                    = __( "Geez" );
  // language code: gil
  iso639_2["gil"]                                    = __( "Gilbertese" );
  // language code: gla gd
  iso639_2["gla"]                   = iso639_1["gd"] = __( "Gaelic" );
  // language code: gle ga
  iso639_2["gle"]                   = iso639_1["ga"] = __( "Irish" );
  // language code: glg gl
  iso639_2["glg"]                   = iso639_1["gl"] = __( "Galician" );
  // language code: glv gv
  iso639_2["glv"]                   = iso639_1["gv"] = __( "Manx" );
  // language code: gmh
  iso639_2["gmh"]                                    = __( "German, Middle High (ca.1050-1500)" );
  // language code: goh
  iso639_2["goh"]                                    = __( "German, Old High (ca.750-1050)" );
  // language code: gon
  iso639_2["gon"]                                    = __( "Gondi" );
  // language code: gor
  iso639_2["gor"]                                    = __( "Gorontalo" );
  // language code: got
  iso639_2["got"]                                    = __( "Gothic" );
  // language code: grb
  iso639_2["grb"]                                    = __( "Grebo" );
  // language code: grc
  iso639_2["grc"]                                    = __( "Greek, Ancient (to 1453)" );
  // language code: gre ell el
  iso639_2["gre"] = iso639_2["ell"] = iso639_1["el"] = __( "Greek, Modern (1453-)" );
  // language code: grn gn
  iso639_2["grn"]                   = iso639_1["gn"] = __( "Guarani" );
  // language code: guj gu
  iso639_2["guj"]                   = iso639_1["gu"] = __( "Gujarati" );
  // language code: gwi
  iso639_2["gwi"]                                    = __( "Gwich'in" );
  // language code: hai
  iso639_2["hai"]                                    = __( "Haida" );
  // language code: hat ht
  iso639_2["hat"]                   = iso639_1["ht"] = __( "Haitian" );
  // language code: hau ha
  iso639_2["hau"]                   = iso639_1["ha"] = __( "Hausa" );
  // language code: haw
  iso639_2["haw"]                                    = __( "Hawaiian" );
  // language code: heb he
  iso639_2["heb"]                   = iso639_1["he"] = __( "Hebrew" );
  // language code: her hz
  iso639_2["her"]                   = iso639_1["hz"] = __( "Herero" );
  // language code: hil
  iso639_2["hil"]                                    = __( "Hiligaynon" );
  // language code: him
  iso639_2["him"]                                    = __( "Himachali" );
  // language code: hin hi
  iso639_2["hin"]                   = iso639_1["hi"] = __( "Hindi" );
  // language code: hit
  iso639_2["hit"]                                    = __( "Hittite" );
  // language code: hmn
  iso639_2["hmn"]                                    = __( "Hmong" );
  // language code: hmo ho
  iso639_2["hmo"]                   = iso639_1["ho"] = __( "Hiri Motu" );
  // language code: hsb
  iso639_2["hsb"]                                    = __( "Upper Sorbian" );
  // language code: hun hu
  iso639_2["hun"]                   = iso639_1["hu"] = __( "Hungarian" );
  // language code: hup
  iso639_2["hup"]                                    = __( "Hupa" );
  // language code: iba
  iso639_2["iba"]                                    = __( "Iban" );
  // language code: ibo ig
  iso639_2["ibo"]                   = iso639_1["ig"] = __( "Igbo" );
  // language code: ice isl is
  iso639_2["ice"] = iso639_2["isl"] = iso639_1["is"] = __( "Icelandic" );
  // language code: ido io
  iso639_2["ido"]                   = iso639_1["io"] = __( "Ido" );
  // language code: iii ii
  iso639_2["iii"]                   = iso639_1["ii"] = __( "Sichuan Yi" );
  // language code: ijo
  iso639_2["ijo"]                                    = __( "Ijo" );
  // language code: iku iu
  iso639_2["iku"]                   = iso639_1["iu"] = __( "Inuktitut" );
  // language code: ile ie
  iso639_2["ile"]                   = iso639_1["ie"] = __( "Interlingue" );
  // language code: ilo
  iso639_2["ilo"]                                    = __( "Iloko" );
  // language code: ina ia
  iso639_2["ina"]                   = iso639_1["ia"] = __( "Interlingua (International Auxiliary Language Association)" );
  // language code: inc
  iso639_2["inc"]                                    = __( "Indic (Other)" );
  // language code: ind id
  iso639_2["ind"]                   = iso639_1["id"] = __( "Indonesian" );
  // language code: ine
  iso639_2["ine"]                                    = __( "Indo-European (Other)" );
  // language code: inh
  iso639_2["inh"]                                    = __( "Ingush" );
  // language code: ipk ik
  iso639_2["ipk"]                   = iso639_1["ik"] = __( "Inupiaq" );
  // language code: ira
  iso639_2["ira"]                                    = __( "Iranian (Other)" );
  // language code: iro
  iso639_2["iro"]                                    = __( "Iroquoian languages" );
  // language code: ita it
  iso639_2["ita"]                   = iso639_1["it"] = __( "Italian" );
  // language code: jav jv
  iso639_2["jav"]                   = iso639_1["jv"] = __( "Javanese" );
  // language code: jbo
  iso639_2["jbo"]                                    = __( "Lojban" );
  // language code: jpn ja
  iso639_2["jpn"]                   = iso639_1["ja"] = __( "Japanese" );
  // language code: jpr
  iso639_2["jpr"]                                    = __( "Judeo-Persian" );
  // language code: jrb
  iso639_2["jrb"]                                    = __( "Judeo-Arabic" );
  // language code: kaa
  iso639_2["kaa"]                                    = __( "Kara-Kalpak" );
  // language code: kab
  iso639_2["kab"]                                    = __( "Kabyle" );
  // language code: kac
  iso639_2["kac"]                                    = __( "Kachin" );
  // language code: kal kl
  iso639_2["kal"]                   = iso639_1["kl"] = __( "Kalaallisut" );
  // language code: kam
  iso639_2["kam"]                                    = __( "Kamba" );
  // language code: kan kn
  iso639_2["kan"]                   = iso639_1["kn"] = __( "Kannada" );
  // language code: kar
  iso639_2["kar"]                                    = __( "Karen" );
  // language code: kas ks
  iso639_2["kas"]                   = iso639_1["ks"] = __( "Kashmiri" );
  // language code: kau kr
  iso639_2["kau"]                   = iso639_1["kr"] = __( "Kanuri" );
  // language code: kaw
  iso639_2["kaw"]                                    = __( "Kawi" );
  // language code: kaz kk
  iso639_2["kaz"]                   = iso639_1["kk"] = __( "Kazakh" );
  // language code: kbd
  iso639_2["kbd"]                                    = __( "Kabardian" );
  // language code: kha
  iso639_2["kha"]                                    = __( "Khasi" );
  // language code: khi
  iso639_2["khi"]                                    = __( "Khoisan (Other)" );
  // language code: khm km
  iso639_2["khm"]                   = iso639_1["km"] = __( "Khmer" );
  // language code: kho
  iso639_2["kho"]                                    = __( "Khotanese" );
  // language code: kik ki
  iso639_2["kik"]                   = iso639_1["ki"] = __( "Kikuyu" );
  // language code: kin rw
  iso639_2["kin"]                   = iso639_1["rw"] = __( "Kinyarwanda" );
  // language code: kir ky
  iso639_2["kir"]                   = iso639_1["ky"] = __( "Kirghiz" );
  // language code: kmb
  iso639_2["kmb"]                                    = __( "Kimbundu" );
  // language code: kok
  iso639_2["kok"]                                    = __( "Konkani" );
  // language code: kom kv
  iso639_2["kom"]                   = iso639_1["kv"] = __( "Komi" );
  // language code: kon kg
  iso639_2["kon"]                   = iso639_1["kg"] = __( "Kongo" );
  // language code: kor ko
  iso639_2["kor"]                   = iso639_1["ko"] = __( "Korean" );
  // language code: kos
  iso639_2["kos"]                                    = __( "Kosraean" );
  // language code: kpe
  iso639_2["kpe"]                                    = __( "Kpelle" );
  // language code: krc
  iso639_2["krc"]                                    = __( "Karachay-Balkar" );
  // language code: kro
  iso639_2["kro"]                                    = __( "Kru" );
  // language code: kru
  iso639_2["kru"]                                    = __( "Kurukh" );
  // language code: kua kj
  iso639_2["kua"]                   = iso639_1["kj"] = __( "Kuanyama" );
  // language code: kum
  iso639_2["kum"]                                    = __( "Kumyk" );
  // language code: kur ku
  iso639_2["kur"]                   = iso639_1["ku"] = __( "Kurdish" );
  // language code: kut
  iso639_2["kut"]                                    = __( "Kutenai" );
  // language code: lad
  iso639_2["lad"]                                    = __( "Ladino" );
  // language code: lah
  iso639_2["lah"]                                    = __( "Lahnda" );
  // language code: lam
  iso639_2["lam"]                                    = __( "Lamba" );
  // language code: lao lo
  iso639_2["lao"]                   = iso639_1["lo"] = __( "Lao" );
  // language code: lat la
  iso639_2["lat"]                   = iso639_1["la"] = __( "Latin" );
  // language code: lav lv
  iso639_2["lav"]                   = iso639_1["lv"] = __( "Latvian" );
  // language code: lez
  iso639_2["lez"]                                    = __( "Lezghian" );
  // language code: lim li
  iso639_2["lim"]                   = iso639_1["li"] = __( "Limburgan" );
  // language code: lin ln
  iso639_2["lin"]                   = iso639_1["ln"] = __( "Lingala" );
  // language code: lit lt
  iso639_2["lit"]                   = iso639_1["lt"] = __( "Lithuanian" );
  // language code: lol
  iso639_2["lol"]                                    = __( "Mongo" );
  // language code: loz
  iso639_2["loz"]                                    = __( "Lozi" );
  // language code: ltz lb
  iso639_2["ltz"]                   = iso639_1["lb"] = __( "Luxembourgish" );
  // language code: lua
  iso639_2["lua"]                                    = __( "Luba-Lulua" );
  // language code: lub lu
  iso639_2["lub"]                   = iso639_1["lu"] = __( "Luba-Katanga" );
  // language code: lug lg
  iso639_2["lug"]                   = iso639_1["lg"] = __( "Ganda" );
  // language code: lui
  iso639_2["lui"]                                    = __( "Luiseno" );
  // language code: lun
  iso639_2["lun"]                                    = __( "Lunda" );
  // language code: luo
  iso639_2["luo"]                                    = __( "Luo (Kenya and Tanzania)" );
  // language code: lus
  iso639_2["lus"]                                    = __( "lushai" );
  // language code: mac mkd mk
  iso639_2["mac"] = iso639_2["mkd"] = iso639_1["mk"] = __( "Macedonian" );
  // language code: mad
  iso639_2["mad"]                                    = __( "Madurese" );
  // language code: mag
  iso639_2["mag"]                                    = __( "Magahi" );
  // language code: mah mh
  iso639_2["mah"]                   = iso639_1["mh"] = __( "Marshallese" );
  // language code: mai
  iso639_2["mai"]                                    = __( "Maithili" );
  // language code: mak
  iso639_2["mak"]                                    = __( "Makasar" );
  // language code: mal ml
  iso639_2["mal"]                   = iso639_1["ml"] = __( "Malayalam" );
  // language code: man
  iso639_2["man"]                                    = __( "Mandingo" );
  // language code: mao mri mi
  iso639_2["mao"] = iso639_2["mri"] = iso639_1["mi"] = __( "Maori" );
  // language code: map
  iso639_2["map"]                                    = __( "Austronesian (Other)" );
  // language code: mar mr
  iso639_2["mar"]                   = iso639_1["mr"] = __( "Marathi" );
  // language code: mas
  iso639_2["mas"]                                    = __( "Masai" );
  // language code: may msa ms
  iso639_2["may"] = iso639_2["msa"] = iso639_1["ms"] = __( "Malay" );
  // language code: mdf
  iso639_2["mdf"]                                    = __( "Moksha" );
  // language code: mdr
  iso639_2["mdr"]                                    = __( "Mandar" );
  // language code: men
  iso639_2["men"]                                    = __( "Mende" );
  // language code: mga
  iso639_2["mga"]                                    = __( "Irish, Middle (900-1200)" );
  // language code: mic
  iso639_2["mic"]                                    = __( "Mi'kmaq" );
  // language code: min
  iso639_2["min"]                                    = __( "Minangkabau" );
  // language code: mis
  iso639_2["mis"]                                    = __( "Miscellaneous languages" );
  // language code: mkh
  iso639_2["mkh"]                                    = __( "Mon-Khmer (Other)" );
  // language code: mlg mg
  iso639_2["mlg"]                   = iso639_1["mg"] = __( "Malagasy" );
  // language code: mlt mt
  iso639_2["mlt"]                   = iso639_1["mt"] = __( "Maltese" );
  // language code: mnc
  iso639_2["mnc"]                                    = __( "Manchu" );
  // language code: mni
  iso639_2["mni"]                                    = __( "Manipuri" );
  // language code: mno
  iso639_2["mno"]                                    = __( "Manobo languages" );
  // language code: moh
  iso639_2["moh"]                                    = __( "Mohawk" );
  // language code: mol mo
  iso639_2["mol"]                   = iso639_1["mo"] = __( "Moldavian" );
  // language code: mon mn
  iso639_2["mon"]                   = iso639_1["mn"] = __( "Mongolian" );
  // language code: mos
  iso639_2["mos"]                                    = __( "Mossi" );
  // language code: mul
  iso639_2["mul"]                                    = __( "Multiple languages" );
  // language code: mun
  iso639_2["mun"]                                    = __( "Munda languages" );
  // language code: mus
  iso639_2["mus"]                                    = __( "Creek" );
  // language code: mwl
  iso639_2["mwl"]                                    = __( "Mirandese" );
  // language code: mwr
  iso639_2["mwr"]                                    = __( "Marwari" );
  // language code: myn
  iso639_2["myn"]                                    = __( "Mayan languages" );
  // language code: myv
  iso639_2["myv"]                                    = __( "Erzya" );
  // language code: nah
  iso639_2["nah"]                                    = __( "Nahuatl" );
  // language code: nai
  iso639_2["nai"]                                    = __( "North American Indian" );
  // language code: nap
  iso639_2["nap"]                                    = __( "Neapolitan" );
  // language code: nau na
  iso639_2["nau"]                   = iso639_1["na"] = __( "Nauru" );
  // language code: nav nv
  iso639_2["nav"]                   = iso639_1["nv"] = __( "Navajo" );
  // language code: nbl nr
  iso639_2["nbl"]                   = iso639_1["nr"] = __( "Ndebele, South" );
  // language code: nde nd
  iso639_2["nde"]                   = iso639_1["nd"] = __( "Ndebele, North" );
  // language code: ndo ng
  iso639_2["ndo"]                   = iso639_1["ng"] = __( "Ndonga" );
  // language code: nds
  iso639_2["nds"]                                    = __( "Low German" );
  // language code: nep ne
  iso639_2["nep"]                   = iso639_1["ne"] = __( "Nepali" );
  // language code: new
  iso639_2["new"]                                    = __( "Nepal Bhasa" );
  // language code: nia
  iso639_2["nia"]                                    = __( "Nias" );
  // language code: nic
  iso639_2["nic"]                                    = __( "Niger-Kordofanian (Other)" );
  // language code: niu
  iso639_2["niu"]                                    = __( "Niuean" );
  // language code: nno nn
  iso639_2["nno"]                   = iso639_1["nn"] = __( "Norwegian Nynorsk" );
  // language code: nob nb
  iso639_2["nob"]                   = iso639_1["nb"] = __( "Norwegian Bokmal" );
  // language code: nog
  iso639_2["nog"]                                    = __( "Nogai" );
  // language code: non
  iso639_2["non"]                                    = __( "Norse, Old" );
  // language code: nor no
  iso639_2["nor"]                   = iso639_1["no"] = __( "Norwegian" );
  // language code: nso
  iso639_2["nso"]                                    = __( "Northern Sotho" );
  // language code: nub
  iso639_2["nub"]                                    = __( "Nubian languages" );
  // language code: nwc
  iso639_2["nwc"]                                    = __( "Classical Newari" );
  // language code: nya ny
  iso639_2["nya"]                   = iso639_1["ny"] = __( "Chichewa" );
  // language code: nym
  iso639_2["nym"]                                    = __( "Nyamwezi" );
  // language code: nyn
  iso639_2["nyn"]                                    = __( "Nyankole" );
  // language code: nyo
  iso639_2["nyo"]                                    = __( "Nyoro" );
  // language code: nzi
  iso639_2["nzi"]                                    = __( "Nzima" );
  // language code: oci oc
  iso639_2["oci"]                   = iso639_1["oc"] = __( "Occitan (post 1500)" );
  // language code: oji oj
  iso639_2["oji"]                   = iso639_1["oj"] = __( "Ojibwa" );
  // language code: ori or
  iso639_2["ori"]                   = iso639_1["or"] = __( "Oriya" );
  // language code: orm om
  iso639_2["orm"]                   = iso639_1["om"] = __( "Oromo" );
  // language code: osa
  iso639_2["osa"]                                    = __( "Osage" );
  // language code: oss os
  iso639_2["oss"]                   = iso639_1["os"] = __( "Ossetian" );
  // language code: ota
  iso639_2["ota"]                                    = __( "Turkish, Ottoman (1500-1928)" );
  // language code: oto
  iso639_2["oto"]                                    = __( "Otomian languages" );
  // language code: paa
  iso639_2["paa"]                                    = __( "Papuan (Other)" );
  // language code: pag
  iso639_2["pag"]                                    = __( "Pangasinan" );
  // language code: pal
  iso639_2["pal"]                                    = __( "Pahlavi" );
  // language code: pam
  iso639_2["pam"]                                    = __( "Pampanga" );
  // language code: pan pa
  iso639_2["pan"]                   = iso639_1["pa"] = __( "Panjabi" );
  // language code: pap
  iso639_2["pap"]                                    = __( "Papiamento" );
  // language code: pau
  iso639_2["pau"]                                    = __( "Palauan" );
  // language code: peo
  iso639_2["peo"]                                    = __( "Persian, Old (ca.600-400 B.C.)" );
  // language code: per fas fa
  iso639_2["per"] = iso639_2["fas"] = iso639_1["fa"] = __( "Persian" );
  // language code: phi
  iso639_2["phi"]                                    = __( "Philippine (Other)" );
  // language code: phn
  iso639_2["phn"]                                    = __( "Phoenician" );
  // language code: pli pi
  iso639_2["pli"]                   = iso639_1["pi"] = __( "Pali" );
  // language code: pol pl
  iso639_2["pol"]                   = iso639_1["pl"] = __( "Polish" );
  // language code: pon
  iso639_2["pon"]                                    = __( "Pohnpeian" );
  // language code: por pt
  iso639_2["por"]                   = iso639_1["pt"] = __( "Portuguese" );
  // language code: pra
  iso639_2["pra"]                                    = __( "Prakrit languages" );
  // language code: pro
  iso639_2["pro"]                                    = __( "Provencal, Old (to 1500)" );
  // language code: pus ps
  iso639_2["pus"]                   = iso639_1["ps"] = __( "Pushto" );
  // language code: que qu
  iso639_2["que"]                   = iso639_1["qu"] = __( "Quechua" );
  // language code: raj
  iso639_2["raj"]                                    = __( "Rajasthani" );
  // language code: rap
  iso639_2["rap"]                                    = __( "Rapanui" );
  // language code: rar
  iso639_2["rar"]                                    = __( "Rarotongan" );
  // language code: roa
  iso639_2["roa"]                                    = __( "Romance (Other)" );
  // language code: roh rm
  iso639_2["roh"]                   = iso639_1["rm"] = __( "Raeto-Romance" );
  // language code: rom
  iso639_2["rom"]                                    = __( "Romany" );
  // language code: rum ron ro
  iso639_2["rum"] = iso639_2["ron"] = iso639_1["ro"] = __( "Romanian" );
  // language code: run rn
  iso639_2["run"]                   = iso639_1["rn"] = __( "Rundi" );
  // language code: rus ru
  iso639_2["rus"]                   = iso639_1["ru"] = __( "Russian" );
  // language code: sad
  iso639_2["sad"]                                    = __( "Sandawe" );
  // language code: sag sg
  iso639_2["sag"]                   = iso639_1["sg"] = __( "Sango" );
  // language code: sah
  iso639_2["sah"]                                    = __( "Yakut" );
  // language code: sai
  iso639_2["sai"]                                    = __( "South American Indian (Other)" );
  // language code: sal
  iso639_2["sal"]                                    = __( "Salishan languages" );
  // language code: sam
  iso639_2["sam"]                                    = __( "Samaritan Aramaic" );
  // language code: san sa
  iso639_2["san"]                   = iso639_1["sa"] = __( "Sanskrit" );
  // language code: sas
  iso639_2["sas"]                                    = __( "Sasak" );
  // language code: sat
  iso639_2["sat"]                                    = __( "Santali" );
  // language code: scc srp sr
  iso639_2["scc"] = iso639_2["srp"] = iso639_1["sr"] = __( "Serbian" );
  // language code: scn
  iso639_2["scn"]                                    = __( "Sicilian" );
  // language code: sco
  iso639_2["sco"]                                    = __( "Scots" );
  // language code: scr hrv hr
  iso639_2["scr"] = iso639_2["hrv"] = iso639_1["hr"] = __( "Croatian" );
  // language code: sel
  iso639_2["sel"]                                    = __( "Selkup" );
  // language code: sem
  iso639_2["sem"]                                    = __( "Semitic (Other)" );
  // language code: sga
  iso639_2["sga"]                                    = __( "Irish, Old (to 900)" );
  // language code: sgn
  iso639_2["sgn"]                                    = __( "Sign Languages" );
  // language code: shn
  iso639_2["shn"]                                    = __( "Shan" );
  // language code: sid
  iso639_2["sid"]                                    = __( "Sidamo" );
  // language code: sin si
  iso639_2["sin"]                   = iso639_1["si"] = __( "Sinhala" );
  // language code: sio
  iso639_2["sio"]                                    = __( "Siouan languages" );
  // language code: sit
  iso639_2["sit"]                                    = __( "Sino-Tibetan (Other)" );
  // language code: sla
  iso639_2["sla"]                                    = __( "Slavic (Other)" );
  // language code: slo slk sk
  iso639_2["slo"] = iso639_2["slk"] = iso639_1["sk"] = __( "Slovak" );
  // language code: slv sl
  iso639_2["slv"]                   = iso639_1["sl"] = __( "Slovenian" );
  // language code: sma
  iso639_2["sma"]                                    = __( "Southern Sami" );
  // language code: sme se
  iso639_2["sme"]                   = iso639_1["se"] = __( "Northern Sami" );
  // language code: smi
  iso639_2["smi"]                                    = __( "Sami languages (Other)" );
  // language code: smj
  iso639_2["smj"]                                    = __( "Lule Sami" );
  // language code: smn
  iso639_2["smn"]                                    = __( "Inari Sami" );
  // language code: smo sm
  iso639_2["smo"]                   = iso639_1["sm"] = __( "Samoan" );
  // language code: sms
  iso639_2["sms"]                                    = __( "Skolt Sami" );
  // language code: sna sn
  iso639_2["sna"]                   = iso639_1["sn"] = __( "Shona" );
  // language code: snd sd
  iso639_2["snd"]                   = iso639_1["sd"] = __( "Sindhi" );
  // language code: snk
  iso639_2["snk"]                                    = __( "Soninke" );
  // language code: sog
  iso639_2["sog"]                                    = __( "Sogdian" );
  // language code: som so
  iso639_2["som"]                   = iso639_1["so"] = __( "Somali" );
  // language code: son
  iso639_2["son"]                                    = __( "Songhai" );
  // language code: sot st
  iso639_2["sot"]                   = iso639_1["st"] = __( "Sotho, Southern" );
  // language code: spa es
  iso639_2["spa"]                   = iso639_1["es"] = __( "Spanish" );
  // language code: srd sc
  iso639_2["srd"]                   = iso639_1["sc"] = __( "Sardinian" );
  // language code: srr
  iso639_2["srr"]                                    = __( "Serer" );
  // language code: ssa
  iso639_2["ssa"]                                    = __( "Nilo-Saharan (Other)" );
  // language code: ssw ss
  iso639_2["ssw"]                   = iso639_1["ss"] = __( "Swati" );
  // language code: suk
  iso639_2["suk"]                                    = __( "Sukuma" );
  // language code: sun su
  iso639_2["sun"]                   = iso639_1["su"] = __( "Sundanese" );
  // language code: sus
  iso639_2["sus"]                                    = __( "Susu" );
  // language code: sux
  iso639_2["sux"]                                    = __( "Sumerian" );
  // language code: swa sw
  iso639_2["swa"]                   = iso639_1["sw"] = __( "Swahili" );
  // language code: swe sv
  iso639_2["swe"]                   = iso639_1["sv"] = __( "Swedish" );
  // language code: syr
  iso639_2["syr"]                                    = __( "Syriac" );
  // language code: tah ty
  iso639_2["tah"]                   = iso639_1["ty"] = __( "Tahitian" );
  // language code: tai
  iso639_2["tai"]                                    = __( "Tai (Other)" );
  // language code: tam ta
  iso639_2["tam"]                   = iso639_1["ta"] = __( "Tamil" );
  // language code: tat tt
  iso639_2["tat"]                   = iso639_1["tt"] = __( "Tatar" );
  // language code: tel te
  iso639_2["tel"]                   = iso639_1["te"] = __( "Telugu" );
  // language code: tem
  iso639_2["tem"]                                    = __( "Timne" );
  // language code: ter
  iso639_2["ter"]                                    = __( "Tereno" );
  // language code: tet
  iso639_2["tet"]                                    = __( "Tetum" );
  // language code: tgk tg
  iso639_2["tgk"]                   = iso639_1["tg"] = __( "Tajik" );
  // language code: tgl tl
  iso639_2["tgl"]                   = iso639_1["tl"] = __( "Tagalog" );
  // language code: tha th
  iso639_2["tha"]                   = iso639_1["th"] = __( "Thai" );
  // language code: tib bod bo
  iso639_2["tib"] = iso639_2["bod"] = iso639_1["bo"] = __( "Tibetan" );
  // language code: tig
  iso639_2["tig"]                                    = __( "Tigre" );
  // language code: tir ti
  iso639_2["tir"]                   = iso639_1["ti"] = __( "Tigrinya" );
  // language code: tiv
  iso639_2["tiv"]                                    = __( "Tiv" );
  // language code: tkl
  iso639_2["tkl"]                                    = __( "Tokelau" );
  // language code: tlh
  iso639_2["tlh"]                                    = __( "Klingon" );
  // language code: tli
  iso639_2["tli"]                                    = __( "Tlingit" );
  // language code: tmh
  iso639_2["tmh"]                                    = __( "Tamashek" );
  // language code: tog
  iso639_2["tog"]                                    = __( "Tonga (Nyasa)" );
  // language code: ton to
  iso639_2["ton"]                   = iso639_1["to"] = __( "Tonga (Tonga Islands)" );
  // language code: tpi
  iso639_2["tpi"]                                    = __( "Tok Pisin" );
  // language code: tsi
  iso639_2["tsi"]                                    = __( "Tsimshian" );
  // language code: tsn tn
  iso639_2["tsn"]                   = iso639_1["tn"] = __( "Tswana" );
  // language code: tso ts
  iso639_2["tso"]                   = iso639_1["ts"] = __( "Tsonga" );
  // language code: tuk tk
  iso639_2["tuk"]                   = iso639_1["tk"] = __( "Turkmen" );
  // language code: tum
  iso639_2["tum"]                                    = __( "Tumbuka" );
  // language code: tup
  iso639_2["tup"]                                    = __( "Tupi languages" );
  // language code: tur tr
  iso639_2["tur"]                   = iso639_1["tr"] = __( "Turkish" );
  // language code: tut
  iso639_2["tut"]                                    = __( "Altaic (Other)" );
  // language code: tvl
  iso639_2["tvl"]                                    = __( "Tuvalu" );
  // language code: twi tw
  iso639_2["twi"]                   = iso639_1["tw"] = __( "Twi" );
  // language code: tyv
  iso639_2["tyv"]                                    = __( "Tuvinian" );
  // language code: udm
  iso639_2["udm"]                                    = __( "Udmurt" );
  // language code: uga
  iso639_2["uga"]                                    = __( "Ugaritic" );
  // language code: uig ug
  iso639_2["uig"]                   = iso639_1["ug"] = __( "Uighur" );
  // language code: ukr uk
  iso639_2["ukr"]                   = iso639_1["uk"] = __( "Ukrainian" );
  // language code: umb
  iso639_2["umb"]                                    = __( "Umbundu" );
  // language code: und
  iso639_2["und"]                                    = __( "Undetermined" );
  // language code: urd ur
  iso639_2["urd"]                   = iso639_1["ur"] = __( "Urdu" );
  // language code: uzb uz
  iso639_2["uzb"]                   = iso639_1["uz"] = __( "Uzbek" );
  // language code: vai
  iso639_2["vai"]                                    = __( "Vai" );
  // language code: ven ve
  iso639_2["ven"]                   = iso639_1["ve"] = __( "Venda" );
  // language code: vie vi
  iso639_2["vie"]                   = iso639_1["vi"] = __( "Vietnamese" );
  // language code: vol vo
  iso639_2["vol"]                   = iso639_1["vo"] = __( "Volapuk" );
  // language code: vot
  iso639_2["vot"]                                    = __( "Votic" );
  // language code: wak
  iso639_2["wak"]                                    = __( "Wakashan languages" );
  // language code: wal
  iso639_2["wal"]                                    = __( "Walamo" );
  // language code: war
  iso639_2["war"]                                    = __( "Waray" );
  // language code: was
  iso639_2["was"]                                    = __( "Washo" );
  // language code: wel cym cy
  iso639_2["wel"] = iso639_2["cym"] = iso639_1["cy"] = __( "Welsh" );
  // language code: wen
  iso639_2["wen"]                                    = __( "Sorbian languages" );
  // language code: wln wa
  iso639_2["wln"]                   = iso639_1["wa"] = __( "Walloon" );
  // language code: wol wo
  iso639_2["wol"]                   = iso639_1["wo"] = __( "Wolof" );
  // language code: xal
  iso639_2["xal"]                                    = __( "Kalmyk" );
  // language code: xho xh
  iso639_2["xho"]                   = iso639_1["xh"] = __( "Xhosa" );
  // language code: yao
  iso639_2["yao"]                                    = __( "Yao" );
  // language code: yap
  iso639_2["yap"]                                    = __( "Yapese" );
  // language code: yid yi
  iso639_2["yid"]                   = iso639_1["yi"] = __( "Yiddish" );
  // language code: yor yo
  iso639_2["yor"]                   = iso639_1["yo"] = __( "Yoruba" );
  // language code: ypk
  iso639_2["ypk"]                                    = __( "Yupik languages" );
  // language code: zap
  iso639_2["zap"]                                    = __( "Zapotec" );
  // language code: zen
  iso639_2["zen"]                                    = __( "Zenaga" );
  // language code: zha za
  iso639_2["zha"]                   = iso639_1["za"] = __( "Zhuang" );
  // language code: znd
  iso639_2["znd"]                                    = __( "Zande" );
  // language code: zul zu
  iso639_2["zul"]                   = iso639_1["zu"] = __( "Zulu" );
  // language code: zun
  iso639_2["zun"]                                    = __( "Zuni" );
}
