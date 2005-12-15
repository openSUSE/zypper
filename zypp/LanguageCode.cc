/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/LanguageCode.cc
 *
*/
#include <iostream>
#include <map>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include "zypp/LanguageCode.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    typedef std::map<std::string,std::string> CodeMap;
    typedef CodeMap::const_iterator Index;

    // CodeMap[code] = untranslated language name
    // Translation is done in name().
    CodeMap _iso639_1_CodeMap;
    CodeMap _iso639_2_CodeMap;
    CodeMap _others_CodeMap;

    void setDefaultCodeMaps( CodeMap & iso639_1,
                             CodeMap & iso639_2,
                             CodeMap & others );

    /** Assert code maps are initialized. */
    void assertInitCodemaps()
    {
      if ( _others_CodeMap.empty() )
        setDefaultCodeMaps( _iso639_1_CodeMap,
                            _iso639_2_CodeMap,
                            _others_CodeMap );
    }

    /** Return index of \a code_r, if it's in the code maps. */
    Index lookupCode( const std::string & code_r )
    {
      assertInitCodemaps();
      switch ( code_r.size() )
        {
        case 2:
          {
            Index it = _iso639_1_CodeMap.find( code_r );
            if ( it != _iso639_1_CodeMap.end() )
              return it;
          }
          break;

        case 3:
          {
            Index it = _iso639_2_CodeMap.find( code_r );
            if ( it != _iso639_2_CodeMap.end() )
              return it;
          }
          break;
        }
      // not found: check _others_CodeMap
      // !!! not found at all returns _others_CodeMap.end()
      return _others_CodeMap.find( code_r );
    }

    /** Assert \a code_r is in the code maps and return it's index.
     * That's what LanguageCode::Impl calls.
    */
    Index getIndex( const std::string & code_r )
    {
      Index it = lookupCode( code_r );
      if ( it != _others_CodeMap.end() )
        return it;

      // not found: Remember a new code
      CodeMap::value_type nval( code_r, std::string() );

      if ( code_r.size() > 3 || code_r.size() < 2 )
        WAR << "Malformed LanguageCode '" << code_r << "' (expect 2 or 3-letter)" << endl;

      std::string lcode( str::toLower( code_r ) );
      if ( lcode != code_r )
        {
          WAR << "Malformed LanguageCode '" << code_r << "' (not lower case)" << endl;
          // but maybe we're lucky with the lower case code
          // and find a language name.
          it = lookupCode( lcode );
          if ( it != _others_CodeMap.end() )
            nval.second = it->second;
        }

      MIL << "Remember LanguageCode '" << code_r << "': '" << nval.second << "'" << endl;
      return _others_CodeMap.insert( nval ).first;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : LanguageCode::Impl
  //
  /** LanguageCode implementation.
   * \note CodeMaps contain the untranslated language names.
   * Translation is done in \ref name.
  */
  struct LanguageCode::Impl
  {
    Impl()
    : _index( getIndex( std::string() ) )
    {}

    Impl( const std::string & code_r )
    : _index( getIndex( code_r ) )
    {}

    std::string code() const
    { return _index->first; }

    std::string name() const {
      if ( _index->second.empty() )
        {
          std::string ret( _("Unknown language: ") );
          ret += "'";
          ret += _index->first;
          ret += "'";
          return ret;
        }
      return _( _index->second.c_str() );
    }

  private:
    /** index into code map. */
    Index _index;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    { if ( ! _nullimpl ) _nullimpl.reset( new Impl ); return _nullimpl; }

  private:
    /** Default Impl. */
    static shared_ptr<Impl> _nullimpl;
  };
  ///////////////////////////////////////////////////////////////////

  shared_ptr<LanguageCode::Impl> LanguageCode::Impl::_nullimpl;

  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : LanguageCode
  //
  ///////////////////////////////////////////////////////////////////

  const LanguageCode LanguageCode::noCode;
  const LanguageCode LanguageCode::useDefault( "default" );

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : LanguageCode::LanguageCode
  //	METHOD TYPE : Ctor
  //
  LanguageCode::LanguageCode()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : LanguageCode::LanguageCode
  //	METHOD TYPE : Ctor
  //
  LanguageCode::LanguageCode( const std::string & code_r )
  : _pimpl( new Impl( code_r ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : LanguageCode::~LanguageCode
  //	METHOD TYPE : Dtor
  //
  LanguageCode::~LanguageCode()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : LanguageCode::code
  //	METHOD TYPE : std::string
  //
  std::string LanguageCode::code() const
  { return _pimpl->code(); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : LanguageCode::name
  //	METHOD TYPE : std::string
  //
  std::string LanguageCode::name() const
  { return _pimpl->name(); }

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    /** Initialize the code maps.
     * http://www.loc.gov/standards/iso639-2/ISO-639-2_values_8bits.txt
    */
    void setDefaultCodeMaps( CodeMap & iso639_1,
                             CodeMap & iso639_2,
                             CodeMap & others )
    {
      // Defined LanguageCode constants
      others[""]        = N_( "noCode" );
      others["default"] = N_( "Default" );

      // language code: aar aa
      iso639_2["aar"]                   = iso639_1["aa"] = N_( "Afar" );
      // language code: abk ab
      iso639_2["abk"]                   = iso639_1["ab"] = N_( "Abkhazian" );
      // language code: ace
      iso639_2["ace"]                                    = N_( "Achinese" );
      // language code: ach
      iso639_2["ach"]                                    = N_( "Acoli" );
      // language code: ada
      iso639_2["ada"]                                    = N_( "Adangme" );
      // language code: ady
      iso639_2["ady"]                                    = N_( "Adyghe" );
      // language code: afa
      iso639_2["afa"]                                    = N_( "Afro-Asiatic (Other)" );
      // language code: afh
      iso639_2["afh"]                                    = N_( "Afrihili" );
      // language code: afr af
      iso639_2["afr"]                   = iso639_1["af"] = N_( "Afrikaans" );
      // language code: ain
      iso639_2["ain"]                                    = N_( "Ainu" );
      // language code: aka ak
      iso639_2["aka"]                   = iso639_1["ak"] = N_( "Akan" );
      // language code: akk
      iso639_2["akk"]                                    = N_( "Akkadian" );
      // language code: alb sqi sq
      iso639_2["alb"] = iso639_2["sqi"] = iso639_1["sq"] = N_( "Albanian" );
      // language code: ale
      iso639_2["ale"]                                    = N_( "Aleut" );
      // language code: alg
      iso639_2["alg"]                                    = N_( "Algonquian languages" );
      // language code: alt
      iso639_2["alt"]                                    = N_( "Southern Altai" );
      // language code: amh am
      iso639_2["amh"]                   = iso639_1["am"] = N_( "Amharic" );
      // language code: ang
      iso639_2["ang"]                                    = N_( "English, Old (ca.450-1100)" );
      // language code: apa
      iso639_2["apa"]                                    = N_( "Apache languages" );
      // language code: ara ar
      iso639_2["ara"]                   = iso639_1["ar"] = N_( "Arabic" );
      // language code: arc
      iso639_2["arc"]                                    = N_( "Aramaic" );
      // language code: arg an
      iso639_2["arg"]                   = iso639_1["an"] = N_( "Aragonese" );
      // language code: arm hye hy
      iso639_2["arm"] = iso639_2["hye"] = iso639_1["hy"] = N_( "Armenian" );
      // language code: arn
      iso639_2["arn"]                                    = N_( "Araucanian" );
      // language code: arp
      iso639_2["arp"]                                    = N_( "Arapaho" );
      // language code: art
      iso639_2["art"]                                    = N_( "Artificial (Other)" );
      // language code: arw
      iso639_2["arw"]                                    = N_( "Arawak" );
      // language code: asm as
      iso639_2["asm"]                   = iso639_1["as"] = N_( "Assamese" );
      // language code: ast
      iso639_2["ast"]                                    = N_( "Asturian" );
      // language code: ath
      iso639_2["ath"]                                    = N_( "Athapascan languages" );
      // language code: aus
      iso639_2["aus"]                                    = N_( "Australian languages" );
      // language code: ava av
      iso639_2["ava"]                   = iso639_1["av"] = N_( "Avaric" );
      // language code: ave ae
      iso639_2["ave"]                   = iso639_1["ae"] = N_( "Avestan" );
      // language code: awa
      iso639_2["awa"]                                    = N_( "Awadhi" );
      // language code: aym ay
      iso639_2["aym"]                   = iso639_1["ay"] = N_( "Aymara" );
      // language code: aze az
      iso639_2["aze"]                   = iso639_1["az"] = N_( "Azerbaijani" );
      // language code: bad
      iso639_2["bad"]                                    = N_( "Banda" );
      // language code: bai
      iso639_2["bai"]                                    = N_( "Bamileke languages" );
      // language code: bak ba
      iso639_2["bak"]                   = iso639_1["ba"] = N_( "Bashkir" );
      // language code: bal
      iso639_2["bal"]                                    = N_( "Baluchi" );
      // language code: bam bm
      iso639_2["bam"]                   = iso639_1["bm"] = N_( "Bambara" );
      // language code: ban
      iso639_2["ban"]                                    = N_( "Balinese" );
      // language code: baq eus eu
      iso639_2["baq"] = iso639_2["eus"] = iso639_1["eu"] = N_( "Basque" );
      // language code: bas
      iso639_2["bas"]                                    = N_( "Basa" );
      // language code: bat
      iso639_2["bat"]                                    = N_( "Baltic (Other)" );
      // language code: bej
      iso639_2["bej"]                                    = N_( "Beja" );
      // language code: bel be
      iso639_2["bel"]                   = iso639_1["be"] = N_( "Belarusian" );
      // language code: bem
      iso639_2["bem"]                                    = N_( "Bemba" );
      // language code: ben bn
      iso639_2["ben"]                   = iso639_1["bn"] = N_( "Bengali" );
      // language code: ber
      iso639_2["ber"]                                    = N_( "Berber (Other)" );
      // language code: bho
      iso639_2["bho"]                                    = N_( "Bhojpuri" );
      // language code: bih bh
      iso639_2["bih"]                   = iso639_1["bh"] = N_( "Bihari" );
      // language code: bik
      iso639_2["bik"]                                    = N_( "Bikol" );
      // language code: bin
      iso639_2["bin"]                                    = N_( "Bini" );
      // language code: bis bi
      iso639_2["bis"]                   = iso639_1["bi"] = N_( "Bislama" );
      // language code: bla
      iso639_2["bla"]                                    = N_( "Siksika" );
      // language code: bnt
      iso639_2["bnt"]                                    = N_( "Bantu (Other)" );
      // language code: bos bs
      iso639_2["bos"]                   = iso639_1["bs"] = N_( "Bosnian" );
      // language code: bra
      iso639_2["bra"]                                    = N_( "Braj" );
      // language code: bre br
      iso639_2["bre"]                   = iso639_1["br"] = N_( "Breton" );
      // language code: btk
      iso639_2["btk"]                                    = N_( "Batak (Indonesia)" );
      // language code: bua
      iso639_2["bua"]                                    = N_( "Buriat" );
      // language code: bug
      iso639_2["bug"]                                    = N_( "Buginese" );
      // language code: bul bg
      iso639_2["bul"]                   = iso639_1["bg"] = N_( "Bulgarian" );
      // language code: bur mya my
      iso639_2["bur"] = iso639_2["mya"] = iso639_1["my"] = N_( "Burmese" );
      // language code: byn
      iso639_2["byn"]                                    = N_( "Blin" );
      // language code: cad
      iso639_2["cad"]                                    = N_( "Caddo" );
      // language code: cai
      iso639_2["cai"]                                    = N_( "Central American Indian (Other)" );
      // language code: car
      iso639_2["car"]                                    = N_( "Carib" );
      // language code: cat ca
      iso639_2["cat"]                   = iso639_1["ca"] = N_( "Catalan" );
      // language code: cau
      iso639_2["cau"]                                    = N_( "Caucasian (Other)" );
      // language code: ceb
      iso639_2["ceb"]                                    = N_( "Cebuano" );
      // language code: cel
      iso639_2["cel"]                                    = N_( "Celtic (Other)" );
      // language code: cha ch
      iso639_2["cha"]                   = iso639_1["ch"] = N_( "Chamorro" );
      // language code: chb
      iso639_2["chb"]                                    = N_( "Chibcha" );
      // language code: che ce
      iso639_2["che"]                   = iso639_1["ce"] = N_( "Chechen" );
      // language code: chg
      iso639_2["chg"]                                    = N_( "Chagatai" );
      // language code: chi zho zh
      iso639_2["chi"] = iso639_2["zho"] = iso639_1["zh"] = N_( "Chinese" );
      // language code: chk
      iso639_2["chk"]                                    = N_( "Chuukese" );
      // language code: chm
      iso639_2["chm"]                                    = N_( "Mari" );
      // language code: chn
      iso639_2["chn"]                                    = N_( "Chinook jargon" );
      // language code: cho
      iso639_2["cho"]                                    = N_( "Choctaw" );
      // language code: chp
      iso639_2["chp"]                                    = N_( "Chipewyan" );
      // language code: chr
      iso639_2["chr"]                                    = N_( "Cherokee" );
      // language code: chu cu
      iso639_2["chu"]                   = iso639_1["cu"] = N_( "Church Slavic" );
      // language code: chv cv
      iso639_2["chv"]                   = iso639_1["cv"] = N_( "Chuvash" );
      // language code: chy
      iso639_2["chy"]                                    = N_( "Cheyenne" );
      // language code: cmc
      iso639_2["cmc"]                                    = N_( "Chamic languages" );
      // language code: cop
      iso639_2["cop"]                                    = N_( "Coptic" );
      // language code: cor kw
      iso639_2["cor"]                   = iso639_1["kw"] = N_( "Cornish" );
      // language code: cos co
      iso639_2["cos"]                   = iso639_1["co"] = N_( "Corsican" );
      // language code: cpe
      iso639_2["cpe"]                                    = N_( "Creoles and pidgins, English based (Other)" );
      // language code: cpf
      iso639_2["cpf"]                                    = N_( "Creoles and pidgins, French-based (Other)" );
      // language code: cpp
      iso639_2["cpp"]                                    = N_( "Creoles and pidgins, Portuguese-based (Other)" );
      // language code: cre cr
      iso639_2["cre"]                   = iso639_1["cr"] = N_( "Cree" );
      // language code: crh
      iso639_2["crh"]                                    = N_( "Crimean Tatar" );
      // language code: crp
      iso639_2["crp"]                                    = N_( "Creoles and pidgins (Other)" );
      // language code: csb
      iso639_2["csb"]                                    = N_( "Kashubian" );
      // language code: cus
      iso639_2["cus"]                                    = N_( "Cushitic (Other)" );
      // language code: cze ces cs
      iso639_2["cze"] = iso639_2["ces"] = iso639_1["cs"] = N_( "Czech" );
      // language code: dak
      iso639_2["dak"]                                    = N_( "Dakota" );
      // language code: dan da
      iso639_2["dan"]                   = iso639_1["da"] = N_( "Danish" );
      // language code: dar
      iso639_2["dar"]                                    = N_( "Dargwa" );
      // language code: day
      iso639_2["day"]                                    = N_( "Dayak" );
      // language code: del
      iso639_2["del"]                                    = N_( "Delaware" );
      // language code: den
      iso639_2["den"]                                    = N_( "Slave (Athapascan)" );
      // language code: dgr
      iso639_2["dgr"]                                    = N_( "Dogrib" );
      // language code: din
      iso639_2["din"]                                    = N_( "Dinka" );
      // language code: div dv
      iso639_2["div"]                   = iso639_1["dv"] = N_( "Divehi" );
      // language code: doi
      iso639_2["doi"]                                    = N_( "Dogri" );
      // language code: dra
      iso639_2["dra"]                                    = N_( "Dravidian (Other)" );
      // language code: dsb
      iso639_2["dsb"]                                    = N_( "Lower Sorbian" );
      // language code: dua
      iso639_2["dua"]                                    = N_( "Duala" );
      // language code: dum
      iso639_2["dum"]                                    = N_( "Dutch, Middle (ca.1050-1350)" );
      // language code: dut nld nl
      iso639_2["dut"] = iso639_2["nld"] = iso639_1["nl"] = N_( "Dutch" );
      // language code: dyu
      iso639_2["dyu"]                                    = N_( "Dyula" );
      // language code: dzo dz
      iso639_2["dzo"]                   = iso639_1["dz"] = N_( "Dzongkha" );
      // language code: efi
      iso639_2["efi"]                                    = N_( "Efik" );
      // language code: egy
      iso639_2["egy"]                                    = N_( "Egyptian (Ancient)" );
      // language code: eka
      iso639_2["eka"]                                    = N_( "Ekajuk" );
      // language code: elx
      iso639_2["elx"]                                    = N_( "Elamite" );
      // language code: eng en
      iso639_2["eng"]                   = iso639_1["en"] = N_( "English" );
      // language code: enm
      iso639_2["enm"]                                    = N_( "English, Middle (1100-1500)" );
      // language code: epo eo
      iso639_2["epo"]                   = iso639_1["eo"] = N_( "Esperanto" );
      // language code: est et
      iso639_2["est"]                   = iso639_1["et"] = N_( "Estonian" );
      // language code: ewe ee
      iso639_2["ewe"]                   = iso639_1["ee"] = N_( "Ewe" );
      // language code: ewo
      iso639_2["ewo"]                                    = N_( "Ewondo" );
      // language code: fan
      iso639_2["fan"]                                    = N_( "Fang" );
      // language code: fao fo
      iso639_2["fao"]                   = iso639_1["fo"] = N_( "Faroese" );
      // language code: fat
      iso639_2["fat"]                                    = N_( "Fanti" );
      // language code: fij fj
      iso639_2["fij"]                   = iso639_1["fj"] = N_( "Fijian" );
      // language code: fil
      iso639_2["fil"]                                    = N_( "Filipino" );
      // language code: fin fi
      iso639_2["fin"]                   = iso639_1["fi"] = N_( "Finnish" );
      // language code: fiu
      iso639_2["fiu"]                                    = N_( "Finno-Ugrian (Other)" );
      // language code: fon
      iso639_2["fon"]                                    = N_( "Fon" );
      // language code: fre fra fr
      iso639_2["fre"] = iso639_2["fra"] = iso639_1["fr"] = N_( "French" );
      // language code: frm
      iso639_2["frm"]                                    = N_( "French, Middle (ca.1400-1600)" );
      // language code: fro
      iso639_2["fro"]                                    = N_( "French, Old (842-ca.1400)" );
      // language code: fry fy
      iso639_2["fry"]                   = iso639_1["fy"] = N_( "Frisian" );
      // language code: ful ff
      iso639_2["ful"]                   = iso639_1["ff"] = N_( "Fulah" );
      // language code: fur
      iso639_2["fur"]                                    = N_( "Friulian" );
      // language code: gaa
      iso639_2["gaa"]                                    = N_( "Ga" );
      // language code: gay
      iso639_2["gay"]                                    = N_( "Gayo" );
      // language code: gba
      iso639_2["gba"]                                    = N_( "Gbaya" );
      // language code: gem
      iso639_2["gem"]                                    = N_( "Germanic (Other)" );
      // language code: geo kat ka
      iso639_2["geo"] = iso639_2["kat"] = iso639_1["ka"] = N_( "Georgian" );
      // language code: ger deu de
      iso639_2["ger"] = iso639_2["deu"] = iso639_1["de"] = N_( "German" );
      // language code: gez
      iso639_2["gez"]                                    = N_( "Geez" );
      // language code: gil
      iso639_2["gil"]                                    = N_( "Gilbertese" );
      // language code: gla gd
      iso639_2["gla"]                   = iso639_1["gd"] = N_( "Gaelic" );
      // language code: gle ga
      iso639_2["gle"]                   = iso639_1["ga"] = N_( "Irish" );
      // language code: glg gl
      iso639_2["glg"]                   = iso639_1["gl"] = N_( "Galician" );
      // language code: glv gv
      iso639_2["glv"]                   = iso639_1["gv"] = N_( "Manx" );
      // language code: gmh
      iso639_2["gmh"]                                    = N_( "German, Middle High (ca.1050-1500)" );
      // language code: goh
      iso639_2["goh"]                                    = N_( "German, Old High (ca.750-1050)" );
      // language code: gon
      iso639_2["gon"]                                    = N_( "Gondi" );
      // language code: gor
      iso639_2["gor"]                                    = N_( "Gorontalo" );
      // language code: got
      iso639_2["got"]                                    = N_( "Gothic" );
      // language code: grb
      iso639_2["grb"]                                    = N_( "Grebo" );
      // language code: grc
      iso639_2["grc"]                                    = N_( "Greek, Ancient (to 1453)" );
      // language code: gre ell el
      iso639_2["gre"] = iso639_2["ell"] = iso639_1["el"] = N_( "Greek, Modern (1453-)" );
      // language code: grn gn
      iso639_2["grn"]                   = iso639_1["gn"] = N_( "Guarani" );
      // language code: guj gu
      iso639_2["guj"]                   = iso639_1["gu"] = N_( "Gujarati" );
      // language code: gwi
      iso639_2["gwi"]                                    = N_( "Gwich'in" );
      // language code: hai
      iso639_2["hai"]                                    = N_( "Haida" );
      // language code: hat ht
      iso639_2["hat"]                   = iso639_1["ht"] = N_( "Haitian" );
      // language code: hau ha
      iso639_2["hau"]                   = iso639_1["ha"] = N_( "Hausa" );
      // language code: haw
      iso639_2["haw"]                                    = N_( "Hawaiian" );
      // language code: heb he
      iso639_2["heb"]                   = iso639_1["he"] = N_( "Hebrew" );
      // language code: her hz
      iso639_2["her"]                   = iso639_1["hz"] = N_( "Herero" );
      // language code: hil
      iso639_2["hil"]                                    = N_( "Hiligaynon" );
      // language code: him
      iso639_2["him"]                                    = N_( "Himachali" );
      // language code: hin hi
      iso639_2["hin"]                   = iso639_1["hi"] = N_( "Hindi" );
      // language code: hit
      iso639_2["hit"]                                    = N_( "Hittite" );
      // language code: hmn
      iso639_2["hmn"]                                    = N_( "Hmong" );
      // language code: hmo ho
      iso639_2["hmo"]                   = iso639_1["ho"] = N_( "Hiri Motu" );
      // language code: hsb
      iso639_2["hsb"]                                    = N_( "Upper Sorbian" );
      // language code: hun hu
      iso639_2["hun"]                   = iso639_1["hu"] = N_( "Hungarian" );
      // language code: hup
      iso639_2["hup"]                                    = N_( "Hupa" );
      // language code: iba
      iso639_2["iba"]                                    = N_( "Iban" );
      // language code: ibo ig
      iso639_2["ibo"]                   = iso639_1["ig"] = N_( "Igbo" );
      // language code: ice isl is
      iso639_2["ice"] = iso639_2["isl"] = iso639_1["is"] = N_( "Icelandic" );
      // language code: ido io
      iso639_2["ido"]                   = iso639_1["io"] = N_( "Ido" );
      // language code: iii ii
      iso639_2["iii"]                   = iso639_1["ii"] = N_( "Sichuan Yi" );
      // language code: ijo
      iso639_2["ijo"]                                    = N_( "Ijo" );
      // language code: iku iu
      iso639_2["iku"]                   = iso639_1["iu"] = N_( "Inuktitut" );
      // language code: ile ie
      iso639_2["ile"]                   = iso639_1["ie"] = N_( "Interlingue" );
      // language code: ilo
      iso639_2["ilo"]                                    = N_( "Iloko" );
      // language code: ina ia
      iso639_2["ina"]                   = iso639_1["ia"] = N_( "Interlingua (International Auxiliary Language Association)" );
      // language code: inc
      iso639_2["inc"]                                    = N_( "Indic (Other)" );
      // language code: ind id
      iso639_2["ind"]                   = iso639_1["id"] = N_( "Indonesian" );
      // language code: ine
      iso639_2["ine"]                                    = N_( "Indo-European (Other)" );
      // language code: inh
      iso639_2["inh"]                                    = N_( "Ingush" );
      // language code: ipk ik
      iso639_2["ipk"]                   = iso639_1["ik"] = N_( "Inupiaq" );
      // language code: ira
      iso639_2["ira"]                                    = N_( "Iranian (Other)" );
      // language code: iro
      iso639_2["iro"]                                    = N_( "Iroquoian languages" );
      // language code: ita it
      iso639_2["ita"]                   = iso639_1["it"] = N_( "Italian" );
      // language code: jav jv
      iso639_2["jav"]                   = iso639_1["jv"] = N_( "Javanese" );
      // language code: jbo
      iso639_2["jbo"]                                    = N_( "Lojban" );
      // language code: jpn ja
      iso639_2["jpn"]                   = iso639_1["ja"] = N_( "Japanese" );
      // language code: jpr
      iso639_2["jpr"]                                    = N_( "Judeo-Persian" );
      // language code: jrb
      iso639_2["jrb"]                                    = N_( "Judeo-Arabic" );
      // language code: kaa
      iso639_2["kaa"]                                    = N_( "Kara-Kalpak" );
      // language code: kab
      iso639_2["kab"]                                    = N_( "Kabyle" );
      // language code: kac
      iso639_2["kac"]                                    = N_( "Kachin" );
      // language code: kal kl
      iso639_2["kal"]                   = iso639_1["kl"] = N_( "Kalaallisut" );
      // language code: kam
      iso639_2["kam"]                                    = N_( "Kamba" );
      // language code: kan kn
      iso639_2["kan"]                   = iso639_1["kn"] = N_( "Kannada" );
      // language code: kar
      iso639_2["kar"]                                    = N_( "Karen" );
      // language code: kas ks
      iso639_2["kas"]                   = iso639_1["ks"] = N_( "Kashmiri" );
      // language code: kau kr
      iso639_2["kau"]                   = iso639_1["kr"] = N_( "Kanuri" );
      // language code: kaw
      iso639_2["kaw"]                                    = N_( "Kawi" );
      // language code: kaz kk
      iso639_2["kaz"]                   = iso639_1["kk"] = N_( "Kazakh" );
      // language code: kbd
      iso639_2["kbd"]                                    = N_( "Kabardian" );
      // language code: kha
      iso639_2["kha"]                                    = N_( "Khasi" );
      // language code: khi
      iso639_2["khi"]                                    = N_( "Khoisan (Other)" );
      // language code: khm km
      iso639_2["khm"]                   = iso639_1["km"] = N_( "Khmer" );
      // language code: kho
      iso639_2["kho"]                                    = N_( "Khotanese" );
      // language code: kik ki
      iso639_2["kik"]                   = iso639_1["ki"] = N_( "Kikuyu" );
      // language code: kin rw
      iso639_2["kin"]                   = iso639_1["rw"] = N_( "Kinyarwanda" );
      // language code: kir ky
      iso639_2["kir"]                   = iso639_1["ky"] = N_( "Kirghiz" );
      // language code: kmb
      iso639_2["kmb"]                                    = N_( "Kimbundu" );
      // language code: kok
      iso639_2["kok"]                                    = N_( "Konkani" );
      // language code: kom kv
      iso639_2["kom"]                   = iso639_1["kv"] = N_( "Komi" );
      // language code: kon kg
      iso639_2["kon"]                   = iso639_1["kg"] = N_( "Kongo" );
      // language code: kor ko
      iso639_2["kor"]                   = iso639_1["ko"] = N_( "Korean" );
      // language code: kos
      iso639_2["kos"]                                    = N_( "Kosraean" );
      // language code: kpe
      iso639_2["kpe"]                                    = N_( "Kpelle" );
      // language code: krc
      iso639_2["krc"]                                    = N_( "Karachay-Balkar" );
      // language code: kro
      iso639_2["kro"]                                    = N_( "Kru" );
      // language code: kru
      iso639_2["kru"]                                    = N_( "Kurukh" );
      // language code: kua kj
      iso639_2["kua"]                   = iso639_1["kj"] = N_( "Kuanyama" );
      // language code: kum
      iso639_2["kum"]                                    = N_( "Kumyk" );
      // language code: kur ku
      iso639_2["kur"]                   = iso639_1["ku"] = N_( "Kurdish" );
      // language code: kut
      iso639_2["kut"]                                    = N_( "Kutenai" );
      // language code: lad
      iso639_2["lad"]                                    = N_( "Ladino" );
      // language code: lah
      iso639_2["lah"]                                    = N_( "Lahnda" );
      // language code: lam
      iso639_2["lam"]                                    = N_( "Lamba" );
      // language code: lao lo
      iso639_2["lao"]                   = iso639_1["lo"] = N_( "Lao" );
      // language code: lat la
      iso639_2["lat"]                   = iso639_1["la"] = N_( "Latin" );
      // language code: lav lv
      iso639_2["lav"]                   = iso639_1["lv"] = N_( "Latvian" );
      // language code: lez
      iso639_2["lez"]                                    = N_( "Lezghian" );
      // language code: lim li
      iso639_2["lim"]                   = iso639_1["li"] = N_( "Limburgan" );
      // language code: lin ln
      iso639_2["lin"]                   = iso639_1["ln"] = N_( "Lingala" );
      // language code: lit lt
      iso639_2["lit"]                   = iso639_1["lt"] = N_( "Lithuanian" );
      // language code: lol
      iso639_2["lol"]                                    = N_( "Mongo" );
      // language code: loz
      iso639_2["loz"]                                    = N_( "Lozi" );
      // language code: ltz lb
      iso639_2["ltz"]                   = iso639_1["lb"] = N_( "Luxembourgish" );
      // language code: lua
      iso639_2["lua"]                                    = N_( "Luba-Lulua" );
      // language code: lub lu
      iso639_2["lub"]                   = iso639_1["lu"] = N_( "Luba-Katanga" );
      // language code: lug lg
      iso639_2["lug"]                   = iso639_1["lg"] = N_( "Ganda" );
      // language code: lui
      iso639_2["lui"]                                    = N_( "Luiseno" );
      // language code: lun
      iso639_2["lun"]                                    = N_( "Lunda" );
      // language code: luo
      iso639_2["luo"]                                    = N_( "Luo (Kenya and Tanzania)" );
      // language code: lus
      iso639_2["lus"]                                    = N_( "lushai" );
      // language code: mac mkd mk
      iso639_2["mac"] = iso639_2["mkd"] = iso639_1["mk"] = N_( "Macedonian" );
      // language code: mad
      iso639_2["mad"]                                    = N_( "Madurese" );
      // language code: mag
      iso639_2["mag"]                                    = N_( "Magahi" );
      // language code: mah mh
      iso639_2["mah"]                   = iso639_1["mh"] = N_( "Marshallese" );
      // language code: mai
      iso639_2["mai"]                                    = N_( "Maithili" );
      // language code: mak
      iso639_2["mak"]                                    = N_( "Makasar" );
      // language code: mal ml
      iso639_2["mal"]                   = iso639_1["ml"] = N_( "Malayalam" );
      // language code: man
      iso639_2["man"]                                    = N_( "Mandingo" );
      // language code: mao mri mi
      iso639_2["mao"] = iso639_2["mri"] = iso639_1["mi"] = N_( "Maori" );
      // language code: map
      iso639_2["map"]                                    = N_( "Austronesian (Other)" );
      // language code: mar mr
      iso639_2["mar"]                   = iso639_1["mr"] = N_( "Marathi" );
      // language code: mas
      iso639_2["mas"]                                    = N_( "Masai" );
      // language code: may msa ms
      iso639_2["may"] = iso639_2["msa"] = iso639_1["ms"] = N_( "Malay" );
      // language code: mdf
      iso639_2["mdf"]                                    = N_( "Moksha" );
      // language code: mdr
      iso639_2["mdr"]                                    = N_( "Mandar" );
      // language code: men
      iso639_2["men"]                                    = N_( "Mende" );
      // language code: mga
      iso639_2["mga"]                                    = N_( "Irish, Middle (900-1200)" );
      // language code: mic
      iso639_2["mic"]                                    = N_( "Mi'kmaq" );
      // language code: min
      iso639_2["min"]                                    = N_( "Minangkabau" );
      // language code: mis
      iso639_2["mis"]                                    = N_( "Miscellaneous languages" );
      // language code: mkh
      iso639_2["mkh"]                                    = N_( "Mon-Khmer (Other)" );
      // language code: mlg mg
      iso639_2["mlg"]                   = iso639_1["mg"] = N_( "Malagasy" );
      // language code: mlt mt
      iso639_2["mlt"]                   = iso639_1["mt"] = N_( "Maltese" );
      // language code: mnc
      iso639_2["mnc"]                                    = N_( "Manchu" );
      // language code: mni
      iso639_2["mni"]                                    = N_( "Manipuri" );
      // language code: mno
      iso639_2["mno"]                                    = N_( "Manobo languages" );
      // language code: moh
      iso639_2["moh"]                                    = N_( "Mohawk" );
      // language code: mol mo
      iso639_2["mol"]                   = iso639_1["mo"] = N_( "Moldavian" );
      // language code: mon mn
      iso639_2["mon"]                   = iso639_1["mn"] = N_( "Mongolian" );
      // language code: mos
      iso639_2["mos"]                                    = N_( "Mossi" );
      // language code: mul
      iso639_2["mul"]                                    = N_( "Multiple languages" );
      // language code: mun
      iso639_2["mun"]                                    = N_( "Munda languages" );
      // language code: mus
      iso639_2["mus"]                                    = N_( "Creek" );
      // language code: mwl
      iso639_2["mwl"]                                    = N_( "Mirandese" );
      // language code: mwr
      iso639_2["mwr"]                                    = N_( "Marwari" );
      // language code: myn
      iso639_2["myn"]                                    = N_( "Mayan languages" );
      // language code: myv
      iso639_2["myv"]                                    = N_( "Erzya" );
      // language code: nah
      iso639_2["nah"]                                    = N_( "Nahuatl" );
      // language code: nai
      iso639_2["nai"]                                    = N_( "North American Indian" );
      // language code: nap
      iso639_2["nap"]                                    = N_( "Neapolitan" );
      // language code: nau na
      iso639_2["nau"]                   = iso639_1["na"] = N_( "Nauru" );
      // language code: nav nv
      iso639_2["nav"]                   = iso639_1["nv"] = N_( "Navajo" );
      // language code: nbl nr
      iso639_2["nbl"]                   = iso639_1["nr"] = N_( "Ndebele, South" );
      // language code: nde nd
      iso639_2["nde"]                   = iso639_1["nd"] = N_( "Ndebele, North" );
      // language code: ndo ng
      iso639_2["ndo"]                   = iso639_1["ng"] = N_( "Ndonga" );
      // language code: nds
      iso639_2["nds"]                                    = N_( "Low German" );
      // language code: nep ne
      iso639_2["nep"]                   = iso639_1["ne"] = N_( "Nepali" );
      // language code: new
      iso639_2["new"]                                    = N_( "Nepal Bhasa" );
      // language code: nia
      iso639_2["nia"]                                    = N_( "Nias" );
      // language code: nic
      iso639_2["nic"]                                    = N_( "Niger-Kordofanian (Other)" );
      // language code: niu
      iso639_2["niu"]                                    = N_( "Niuean" );
      // language code: nno nn
      iso639_2["nno"]                   = iso639_1["nn"] = N_( "Norwegian Nynorsk" );
      // language code: nob nb
      iso639_2["nob"]                   = iso639_1["nb"] = N_( "Norwegian Bokmal" );
      // language code: nog
      iso639_2["nog"]                                    = N_( "Nogai" );
      // language code: non
      iso639_2["non"]                                    = N_( "Norse, Old" );
      // language code: nor no
      iso639_2["nor"]                   = iso639_1["no"] = N_( "Norwegian" );
      // language code: nso
      iso639_2["nso"]                                    = N_( "Northern Sotho" );
      // language code: nub
      iso639_2["nub"]                                    = N_( "Nubian languages" );
      // language code: nwc
      iso639_2["nwc"]                                    = N_( "Classical Newari" );
      // language code: nya ny
      iso639_2["nya"]                   = iso639_1["ny"] = N_( "Chichewa" );
      // language code: nym
      iso639_2["nym"]                                    = N_( "Nyamwezi" );
      // language code: nyn
      iso639_2["nyn"]                                    = N_( "Nyankole" );
      // language code: nyo
      iso639_2["nyo"]                                    = N_( "Nyoro" );
      // language code: nzi
      iso639_2["nzi"]                                    = N_( "Nzima" );
      // language code: oci oc
      iso639_2["oci"]                   = iso639_1["oc"] = N_( "Occitan (post 1500)" );
      // language code: oji oj
      iso639_2["oji"]                   = iso639_1["oj"] = N_( "Ojibwa" );
      // language code: ori or
      iso639_2["ori"]                   = iso639_1["or"] = N_( "Oriya" );
      // language code: orm om
      iso639_2["orm"]                   = iso639_1["om"] = N_( "Oromo" );
      // language code: osa
      iso639_2["osa"]                                    = N_( "Osage" );
      // language code: oss os
      iso639_2["oss"]                   = iso639_1["os"] = N_( "Ossetian" );
      // language code: ota
      iso639_2["ota"]                                    = N_( "Turkish, Ottoman (1500-1928)" );
      // language code: oto
      iso639_2["oto"]                                    = N_( "Otomian languages" );
      // language code: paa
      iso639_2["paa"]                                    = N_( "Papuan (Other)" );
      // language code: pag
      iso639_2["pag"]                                    = N_( "Pangasinan" );
      // language code: pal
      iso639_2["pal"]                                    = N_( "Pahlavi" );
      // language code: pam
      iso639_2["pam"]                                    = N_( "Pampanga" );
      // language code: pan pa
      iso639_2["pan"]                   = iso639_1["pa"] = N_( "Panjabi" );
      // language code: pap
      iso639_2["pap"]                                    = N_( "Papiamento" );
      // language code: pau
      iso639_2["pau"]                                    = N_( "Palauan" );
      // language code: peo
      iso639_2["peo"]                                    = N_( "Persian, Old (ca.600-400 B.C.)" );
      // language code: per fas fa
      iso639_2["per"] = iso639_2["fas"] = iso639_1["fa"] = N_( "Persian" );
      // language code: phi
      iso639_2["phi"]                                    = N_( "Philippine (Other)" );
      // language code: phn
      iso639_2["phn"]                                    = N_( "Phoenician" );
      // language code: pli pi
      iso639_2["pli"]                   = iso639_1["pi"] = N_( "Pali" );
      // language code: pol pl
      iso639_2["pol"]                   = iso639_1["pl"] = N_( "Polish" );
      // language code: pon
      iso639_2["pon"]                                    = N_( "Pohnpeian" );
      // language code: por pt
      iso639_2["por"]                   = iso639_1["pt"] = N_( "Portuguese" );
      // language code: pra
      iso639_2["pra"]                                    = N_( "Prakrit languages" );
      // language code: pro
      iso639_2["pro"]                                    = N_( "Provencal, Old (to 1500)" );
      // language code: pus ps
      iso639_2["pus"]                   = iso639_1["ps"] = N_( "Pushto" );
      // language code: que qu
      iso639_2["que"]                   = iso639_1["qu"] = N_( "Quechua" );
      // language code: raj
      iso639_2["raj"]                                    = N_( "Rajasthani" );
      // language code: rap
      iso639_2["rap"]                                    = N_( "Rapanui" );
      // language code: rar
      iso639_2["rar"]                                    = N_( "Rarotongan" );
      // language code: roa
      iso639_2["roa"]                                    = N_( "Romance (Other)" );
      // language code: roh rm
      iso639_2["roh"]                   = iso639_1["rm"] = N_( "Raeto-Romance" );
      // language code: rom
      iso639_2["rom"]                                    = N_( "Romany" );
      // language code: rum ron ro
      iso639_2["rum"] = iso639_2["ron"] = iso639_1["ro"] = N_( "Romanian" );
      // language code: run rn
      iso639_2["run"]                   = iso639_1["rn"] = N_( "Rundi" );
      // language code: rus ru
      iso639_2["rus"]                   = iso639_1["ru"] = N_( "Russian" );
      // language code: sad
      iso639_2["sad"]                                    = N_( "Sandawe" );
      // language code: sag sg
      iso639_2["sag"]                   = iso639_1["sg"] = N_( "Sango" );
      // language code: sah
      iso639_2["sah"]                                    = N_( "Yakut" );
      // language code: sai
      iso639_2["sai"]                                    = N_( "South American Indian (Other)" );
      // language code: sal
      iso639_2["sal"]                                    = N_( "Salishan languages" );
      // language code: sam
      iso639_2["sam"]                                    = N_( "Samaritan Aramaic" );
      // language code: san sa
      iso639_2["san"]                   = iso639_1["sa"] = N_( "Sanskrit" );
      // language code: sas
      iso639_2["sas"]                                    = N_( "Sasak" );
      // language code: sat
      iso639_2["sat"]                                    = N_( "Santali" );
      // language code: scc srp sr
      iso639_2["scc"] = iso639_2["srp"] = iso639_1["sr"] = N_( "Serbian" );
      // language code: scn
      iso639_2["scn"]                                    = N_( "Sicilian" );
      // language code: sco
      iso639_2["sco"]                                    = N_( "Scots" );
      // language code: scr hrv hr
      iso639_2["scr"] = iso639_2["hrv"] = iso639_1["hr"] = N_( "Croatian" );
      // language code: sel
      iso639_2["sel"]                                    = N_( "Selkup" );
      // language code: sem
      iso639_2["sem"]                                    = N_( "Semitic (Other)" );
      // language code: sga
      iso639_2["sga"]                                    = N_( "Irish, Old (to 900)" );
      // language code: sgn
      iso639_2["sgn"]                                    = N_( "Sign Languages" );
      // language code: shn
      iso639_2["shn"]                                    = N_( "Shan" );
      // language code: sid
      iso639_2["sid"]                                    = N_( "Sidamo" );
      // language code: sin si
      iso639_2["sin"]                   = iso639_1["si"] = N_( "Sinhala" );
      // language code: sio
      iso639_2["sio"]                                    = N_( "Siouan languages" );
      // language code: sit
      iso639_2["sit"]                                    = N_( "Sino-Tibetan (Other)" );
      // language code: sla
      iso639_2["sla"]                                    = N_( "Slavic (Other)" );
      // language code: slo slk sk
      iso639_2["slo"] = iso639_2["slk"] = iso639_1["sk"] = N_( "Slovak" );
      // language code: slv sl
      iso639_2["slv"]                   = iso639_1["sl"] = N_( "Slovenian" );
      // language code: sma
      iso639_2["sma"]                                    = N_( "Southern Sami" );
      // language code: sme se
      iso639_2["sme"]                   = iso639_1["se"] = N_( "Northern Sami" );
      // language code: smi
      iso639_2["smi"]                                    = N_( "Sami languages (Other)" );
      // language code: smj
      iso639_2["smj"]                                    = N_( "Lule Sami" );
      // language code: smn
      iso639_2["smn"]                                    = N_( "Inari Sami" );
      // language code: smo sm
      iso639_2["smo"]                   = iso639_1["sm"] = N_( "Samoan" );
      // language code: sms
      iso639_2["sms"]                                    = N_( "Skolt Sami" );
      // language code: sna sn
      iso639_2["sna"]                   = iso639_1["sn"] = N_( "Shona" );
      // language code: snd sd
      iso639_2["snd"]                   = iso639_1["sd"] = N_( "Sindhi" );
      // language code: snk
      iso639_2["snk"]                                    = N_( "Soninke" );
      // language code: sog
      iso639_2["sog"]                                    = N_( "Sogdian" );
      // language code: som so
      iso639_2["som"]                   = iso639_1["so"] = N_( "Somali" );
      // language code: son
      iso639_2["son"]                                    = N_( "Songhai" );
      // language code: sot st
      iso639_2["sot"]                   = iso639_1["st"] = N_( "Sotho, Southern" );
      // language code: spa es
      iso639_2["spa"]                   = iso639_1["es"] = N_( "Spanish" );
      // language code: srd sc
      iso639_2["srd"]                   = iso639_1["sc"] = N_( "Sardinian" );
      // language code: srr
      iso639_2["srr"]                                    = N_( "Serer" );
      // language code: ssa
      iso639_2["ssa"]                                    = N_( "Nilo-Saharan (Other)" );
      // language code: ssw ss
      iso639_2["ssw"]                   = iso639_1["ss"] = N_( "Swati" );
      // language code: suk
      iso639_2["suk"]                                    = N_( "Sukuma" );
      // language code: sun su
      iso639_2["sun"]                   = iso639_1["su"] = N_( "Sundanese" );
      // language code: sus
      iso639_2["sus"]                                    = N_( "Susu" );
      // language code: sux
      iso639_2["sux"]                                    = N_( "Sumerian" );
      // language code: swa sw
      iso639_2["swa"]                   = iso639_1["sw"] = N_( "Swahili" );
      // language code: swe sv
      iso639_2["swe"]                   = iso639_1["sv"] = N_( "Swedish" );
      // language code: syr
      iso639_2["syr"]                                    = N_( "Syriac" );
      // language code: tah ty
      iso639_2["tah"]                   = iso639_1["ty"] = N_( "Tahitian" );
      // language code: tai
      iso639_2["tai"]                                    = N_( "Tai (Other)" );
      // language code: tam ta
      iso639_2["tam"]                   = iso639_1["ta"] = N_( "Tamil" );
      // language code: tat tt
      iso639_2["tat"]                   = iso639_1["tt"] = N_( "Tatar" );
      // language code: tel te
      iso639_2["tel"]                   = iso639_1["te"] = N_( "Telugu" );
      // language code: tem
      iso639_2["tem"]                                    = N_( "Timne" );
      // language code: ter
      iso639_2["ter"]                                    = N_( "Tereno" );
      // language code: tet
      iso639_2["tet"]                                    = N_( "Tetum" );
      // language code: tgk tg
      iso639_2["tgk"]                   = iso639_1["tg"] = N_( "Tajik" );
      // language code: tgl tl
      iso639_2["tgl"]                   = iso639_1["tl"] = N_( "Tagalog" );
      // language code: tha th
      iso639_2["tha"]                   = iso639_1["th"] = N_( "Thai" );
      // language code: tib bod bo
      iso639_2["tib"] = iso639_2["bod"] = iso639_1["bo"] = N_( "Tibetan" );
      // language code: tig
      iso639_2["tig"]                                    = N_( "Tigre" );
      // language code: tir ti
      iso639_2["tir"]                   = iso639_1["ti"] = N_( "Tigrinya" );
      // language code: tiv
      iso639_2["tiv"]                                    = N_( "Tiv" );
      // language code: tkl
      iso639_2["tkl"]                                    = N_( "Tokelau" );
      // language code: tlh
      iso639_2["tlh"]                                    = N_( "Klingon" );
      // language code: tli
      iso639_2["tli"]                                    = N_( "Tlingit" );
      // language code: tmh
      iso639_2["tmh"]                                    = N_( "Tamashek" );
      // language code: tog
      iso639_2["tog"]                                    = N_( "Tonga (Nyasa)" );
      // language code: ton to
      iso639_2["ton"]                   = iso639_1["to"] = N_( "Tonga (Tonga Islands)" );
      // language code: tpi
      iso639_2["tpi"]                                    = N_( "Tok Pisin" );
      // language code: tsi
      iso639_2["tsi"]                                    = N_( "Tsimshian" );
      // language code: tsn tn
      iso639_2["tsn"]                   = iso639_1["tn"] = N_( "Tswana" );
      // language code: tso ts
      iso639_2["tso"]                   = iso639_1["ts"] = N_( "Tsonga" );
      // language code: tuk tk
      iso639_2["tuk"]                   = iso639_1["tk"] = N_( "Turkmen" );
      // language code: tum
      iso639_2["tum"]                                    = N_( "Tumbuka" );
      // language code: tup
      iso639_2["tup"]                                    = N_( "Tupi languages" );
      // language code: tur tr
      iso639_2["tur"]                   = iso639_1["tr"] = N_( "Turkish" );
      // language code: tut
      iso639_2["tut"]                                    = N_( "Altaic (Other)" );
      // language code: tvl
      iso639_2["tvl"]                                    = N_( "Tuvalu" );
      // language code: twi tw
      iso639_2["twi"]                   = iso639_1["tw"] = N_( "Twi" );
      // language code: tyv
      iso639_2["tyv"]                                    = N_( "Tuvinian" );
      // language code: udm
      iso639_2["udm"]                                    = N_( "Udmurt" );
      // language code: uga
      iso639_2["uga"]                                    = N_( "Ugaritic" );
      // language code: uig ug
      iso639_2["uig"]                   = iso639_1["ug"] = N_( "Uighur" );
      // language code: ukr uk
      iso639_2["ukr"]                   = iso639_1["uk"] = N_( "Ukrainian" );
      // language code: umb
      iso639_2["umb"]                                    = N_( "Umbundu" );
      // language code: und
      iso639_2["und"]                                    = N_( "Undetermined" );
      // language code: urd ur
      iso639_2["urd"]                   = iso639_1["ur"] = N_( "Urdu" );
      // language code: uzb uz
      iso639_2["uzb"]                   = iso639_1["uz"] = N_( "Uzbek" );
      // language code: vai
      iso639_2["vai"]                                    = N_( "Vai" );
      // language code: ven ve
      iso639_2["ven"]                   = iso639_1["ve"] = N_( "Venda" );
      // language code: vie vi
      iso639_2["vie"]                   = iso639_1["vi"] = N_( "Vietnamese" );
      // language code: vol vo
      iso639_2["vol"]                   = iso639_1["vo"] = N_( "Volapuk" );
      // language code: vot
      iso639_2["vot"]                                    = N_( "Votic" );
      // language code: wak
      iso639_2["wak"]                                    = N_( "Wakashan languages" );
      // language code: wal
      iso639_2["wal"]                                    = N_( "Walamo" );
      // language code: war
      iso639_2["war"]                                    = N_( "Waray" );
      // language code: was
      iso639_2["was"]                                    = N_( "Washo" );
      // language code: wel cym cy
      iso639_2["wel"] = iso639_2["cym"] = iso639_1["cy"] = N_( "Welsh" );
      // language code: wen
      iso639_2["wen"]                                    = N_( "Sorbian languages" );
      // language code: wln wa
      iso639_2["wln"]                   = iso639_1["wa"] = N_( "Walloon" );
      // language code: wol wo
      iso639_2["wol"]                   = iso639_1["wo"] = N_( "Wolof" );
      // language code: xal
      iso639_2["xal"]                                    = N_( "Kalmyk" );
      // language code: xho xh
      iso639_2["xho"]                   = iso639_1["xh"] = N_( "Xhosa" );
      // language code: yao
      iso639_2["yao"]                                    = N_( "Yao" );
      // language code: yap
      iso639_2["yap"]                                    = N_( "Yapese" );
      // language code: yid yi
      iso639_2["yid"]                   = iso639_1["yi"] = N_( "Yiddish" );
      // language code: yor yo
      iso639_2["yor"]                   = iso639_1["yo"] = N_( "Yoruba" );
      // language code: ypk
      iso639_2["ypk"]                                    = N_( "Yupik languages" );
      // language code: zap
      iso639_2["zap"]                                    = N_( "Zapotec" );
      // language code: zen
      iso639_2["zen"]                                    = N_( "Zenaga" );
      // language code: zha za
      iso639_2["zha"]                   = iso639_1["za"] = N_( "Zhuang" );
      // language code: znd
      iso639_2["znd"]                                    = N_( "Zande" );
      // language code: zul zu
      iso639_2["zul"]                   = iso639_1["zu"] = N_( "Zulu" );
      // language code: zun
      iso639_2["zun"]                                    = N_( "Zuni" );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
