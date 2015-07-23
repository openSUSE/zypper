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

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Hash.h"

#include "zypp/LanguageCode.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    /** Wrap static codemap data. */
    struct CodeMaps // singleton
    {
      typedef std::unordered_map<std::string,std::string> CodeMap;
      typedef CodeMap::const_iterator Index;

      /** Return the CodeMap Index for \a code_r. */
      static Index getIndex( const std::string & code_r )
      {
        static CodeMaps _maps; // the singleton instance
        return _maps.lookup( code_r );
      }

    private:
      /** Ctor initializes the code maps.
       * http://www.loc.gov/standards/iso639-2/ISO-639-2_values_8bits.txt
      */
      CodeMaps();

      /** Make shure the code is in the code maps and return it's index. */
      inline Index lookup( const std::string & code_r );

    private:
      /** All the codes. */
      CodeMap codes;
    };

    inline CodeMaps::Index CodeMaps::lookup( const std::string & code_r )
    {
      Index it = codes.find( code_r );
      if ( it != codes.end() )
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
          it = codes.find( lcode );
          if ( it != codes.end() )
            nval.second = it->second;
        }

      MIL << "Remember LanguageCode '" << code_r << "': '" << nval.second << "'" << endl;
      return codes.insert( nval ).first;
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
    : _index( CodeMaps::getIndex( std::string() ) )
    {}

    Impl( const std::string & code_r )
    : _index( CodeMaps::getIndex( code_r ) )
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
    CodeMaps::Index _index;

  public:
    /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
      static shared_ptr<Impl> _nullimpl( new Impl );
      return _nullimpl;
    }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : LanguageCode
  //
  ///////////////////////////////////////////////////////////////////

  const LanguageCode LanguageCode::noCode;

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

    CodeMaps::CodeMaps()
    {
      // Defined LanguageCode constants
      codes[""]        = N_("No Code");

      struct LangInit
      {
	  const char *iso639_2;
	  const char *iso639_1;
	  const char *name;
      };

      // some languages have more than one iso639_2 code
      // so there are items with duplicate names
      const LangInit langInit[] = {
	  // language code: aar aa
	  { "aar", "aa", N_( "Afar" ) },
	  // language code: abk ab
	  { "abk", "ab", N_( "Abkhazian" ) },
	  // language code: ace
	  { "ace", NULL, N_( "Achinese" ) },
	  // language code: ach
	  { "ach", NULL, N_( "Acoli" ) },
	  // language code: ada
	  { "ada", NULL, N_( "Adangme" ) },
	  // language code: ady
	  { "ady", NULL, N_( "Adyghe" ) },
	  // language code: afa
	  { "afa", NULL, N_( "Afro-Asiatic (Other)" ) },
	  // language code: afh
	  { "afh", NULL, N_( "Afrihili" ) },
	  // language code: afr af
	  { "afr", "af", N_( "Afrikaans" ) },
	  // language code: ain
	  { "ain", NULL, N_( "Ainu" ) },
	  // language code: aka ak
	  { "aka", "ak", N_( "Akan" ) },
	  // language code: akk
	  { "akk", NULL, N_( "Akkadian" ) },
	  // language code: alb sqi sq
	  { "alb", "sq", N_( "Albanian" ) },
	  // language code: alb sqi sq
	  { "sqi", NULL, N_( "Albanian" ) },
	  // language code: ale
	  { "ale", NULL, N_( "Aleut" ) },
	  // language code: alg
	  { "alg", NULL, N_( "Algonquian Languages" ) },
	  // language code: alt
	  { "alt", NULL, N_( "Southern Altai" ) },
	  // language code: amh am
	  { "amh", "am", N_( "Amharic" ) },
	  // language code: ang
	  { "ang", NULL, N_( "English, Old (ca.450-1100)" ) },
	  // language code: apa
	  { "apa", NULL, N_( "Apache Languages" ) },
	  // language code: ara ar
	  { "ara", "ar", N_( "Arabic" ) },
	  // language code: arc
	  { "arc", NULL, N_( "Aramaic" ) },
	  // language code: arg an
	  { "arg", "an", N_( "Aragonese" ) },
	  // language code: arm hye hy
	  { "arm", "hy", N_( "Armenian" ) },
	  // language code: arm hye hy
	  { "hye", NULL, N_( "Armenian" ) },
	  // language code: arn
	  { "arn", NULL, N_( "Araucanian" ) },
	  // language code: arp
	  { "arp", NULL, N_( "Arapaho" ) },
	  // language code: art
	  { "art", NULL, N_( "Artificial (Other)" ) },
	  // language code: arw
	  { "arw", NULL, N_( "Arawak" ) },
	  // language code: asm as
	  { "asm", "as", N_( "Assamese" ) },
	  // language code: ast
	  { "ast", NULL, N_( "Asturian" ) },
	  // language code: ath
	  { "ath", NULL, N_( "Athapascan Languages" ) },
	  // language code: aus
	  { "aus", NULL, N_( "Australian Languages" ) },
	  // language code: ava av
	  { "ava", "av", N_( "Avaric" ) },
	  // language code: ave ae
	  { "ave", "ae", N_( "Avestan" ) },
	  // language code: awa
	  { "awa", NULL, N_( "Awadhi" ) },
	  // language code: aym ay
	  { "aym", "ay", N_( "Aymara" ) },
	  // language code: aze az
	  { "aze", "az", N_( "Azerbaijani" ) },
	  // language code: bad
	  { "bad", NULL, N_( "Banda" ) },
	  // language code: bai
	  { "bai", NULL, N_( "Bamileke Languages" ) },
	  // language code: bak ba
	  { "bak", "ba", N_( "Bashkir" ) },
	  // language code: bal
	  { "bal", NULL, N_( "Baluchi" ) },
	  // language code: bam bm
	  { "bam", "bm", N_( "Bambara" ) },
	  // language code: ban
	  { "ban", NULL, N_( "Balinese" ) },
	  // language code: baq eus eu
	  { "baq", "eu", N_( "Basque" ) },
	  // language code: baq eus eu
	  { "eus", NULL, N_( "Basque" ) },
	  // language code: bas
	  { "bas", NULL, N_( "Basa" ) },
	  // language code: bat
	  { "bat", NULL, N_( "Baltic (Other)" ) },
	  // language code: bej
	  { "bej", NULL, N_( "Beja" ) },
	  // language code: bel be
	  { "bel", "be", N_( "Belarusian" ) },
	  // language code: bem
	  { "bem", NULL, N_( "Bemba" ) },
	  // language code: ben bn
	  { "ben", "bn", N_( "Bengali" ) },
	  // language code: ber
	  { "ber", NULL, N_( "Berber (Other)" ) },
	  // language code: bho
	  { "bho", NULL, N_( "Bhojpuri" ) },
	  // language code: bih bh
	  { "bih", "bh", N_( "Bihari" ) },
	  // language code: bik
	  { "bik", NULL, N_( "Bikol" ) },
	  // language code: bin
	  { "bin", NULL, N_( "Bini" ) },
	  // language code: bis bi
	  { "bis", "bi", N_( "Bislama" ) },
	  // language code: bla
	  { "bla", NULL, N_( "Siksika" ) },
	  // language code: bnt
	  { "bnt", NULL, N_( "Bantu (Other)" ) },
	  // language code: bos bs
	  { "bos", "bs", N_( "Bosnian" ) },
	  // language code: bra
	  { "bra", NULL, N_( "Braj" ) },
	  // language code: bre br
	  { "bre", "br", N_( "Breton" ) },
	  // language code: btk
	  { "btk", NULL, N_( "Batak (Indonesia)" ) },
	  // language code: bua
	  { "bua", NULL, N_( "Buriat" ) },
	  // language code: bug
	  { "bug", NULL, N_( "Buginese" ) },
	  // language code: bul bg
	  { "bul", "bg", N_( "Bulgarian" ) },
	  // language code: bur mya my
	  { "bur", "my", N_( "Burmese" ) },
	  // language code: bur mya my
	  { "mya", NULL, N_( "Burmese" ) },
	  // language code: byn
	  { "byn", NULL, N_( "Blin" ) },
	  // language code: cad
	  { "cad", NULL, N_( "Caddo" ) },
	  // language code: cai
	  { "cai", NULL, N_( "Central American Indian (Other)" ) },
	  // language code: car
	  { "car", NULL, N_( "Carib" ) },
	  // language code: cat ca
	  { "cat", "ca", N_( "Catalan" ) },
	  // language code: cau
	  { "cau", NULL, N_( "Caucasian (Other)" ) },
	  // language code: ceb
	  { "ceb", NULL, N_( "Cebuano" ) },
	  // language code: cel
	  { "cel", NULL, N_( "Celtic (Other)" ) },
	  // language code: cha ch
	  { "cha", "ch", N_( "Chamorro" ) },
	  // language code: chb
	  { "chb", NULL, N_( "Chibcha" ) },
	  // language code: che ce
	  { "che", "ce", N_( "Chechen" ) },
	  // language code: chg
	  { "chg", NULL, N_( "Chagatai" ) },
	  // language code: chi zho zh
	  { "chi", "zh", N_( "Chinese" ) },
	  // language code: chi zho zh
	  { "zho", NULL, N_( "Chinese" ) },
	  // language code: chk
	  { "chk", NULL, N_( "Chuukese" ) },
	  // language code: chm
	  { "chm", NULL, N_( "Mari" ) },
	  // language code: chn
	  { "chn", NULL, N_( "Chinook Jargon" ) },
	  // language code: cho
	  { "cho", NULL, N_( "Choctaw" ) },
	  // language code: chp
	  { "chp", NULL, N_( "Chipewyan" ) },
	  // language code: chr
	  { "chr", NULL, N_( "Cherokee" ) },
	  // language code: chu cu
	  { "chu", "cu", N_( "Church Slavic" ) },
	  // language code: chv cv
	  { "chv", "cv", N_( "Chuvash" ) },
	  // language code: chy
	  { "chy", NULL, N_( "Cheyenne" ) },
	  // language code: cmc
	  { "cmc", NULL, N_( "Chamic Languages" ) },
	  // language code: cop
	  { "cop", NULL, N_( "Coptic" ) },
	  // language code: cor kw
	  { "cor", "kw", N_( "Cornish" ) },
	  // language code: cos co
	  { "cos", "co", N_( "Corsican" ) },
	  // language code: cpe
	  { "cpe", NULL, N_( "Creoles and Pidgins, English-Based (Other)" ) },
	  // language code: cpf
	  { "cpf", NULL, N_( "Creoles and Pidgins, French-Based (Other)" ) },
	  // language code: cpp
	  { "cpp", NULL, N_( "Creoles and Pidgins, Portuguese-Based (Other)" ) },
	  // language code: cre cr
	  { "cre", "cr", N_( "Cree" ) },
	  // language code: crh
	  { "crh", NULL, N_( "Crimean Tatar" ) },
	  // language code: crp
	  { "crp", NULL, N_( "Creoles and Pidgins (Other)" ) },
	  // language code: csb
	  { "csb", NULL, N_( "Kashubian" ) },
	  // language code: cus
	  { "cus", NULL, N_( "Cushitic (Other)" ) },
	  // language code: cze ces cs
	  { "cze", "cs", N_( "Czech" ) },
	  // language code: cze ces cs
	  { "ces", NULL, N_( "Czech" ) },
	  // language code: dak
	  { "dak", NULL, N_( "Dakota" ) },
	  // language code: dan da
	  { "dan", "da", N_( "Danish" ) },
	  // language code: dar
	  { "dar", NULL, N_( "Dargwa" ) },
	  // language code: day
	  { "day", NULL, N_( "Dayak" ) },
	  // language code: del
	  { "del", NULL, N_( "Delaware" ) },
	  // language code: den
	  { "den", NULL, N_( "Slave (Athapascan)" ) },
	  // language code: dgr
	  { "dgr", NULL, N_( "Dogrib" ) },
	  // language code: din
	  { "din", NULL, N_( "Dinka" ) },
	  // language code: div dv
	  { "div", "dv", N_( "Divehi" ) },
	  // language code: doi
	  { "doi", NULL, N_( "Dogri" ) },
	  // language code: dra
	  { "dra", NULL, N_( "Dravidian (Other)" ) },
	  // language code: dsb
	  { "dsb", NULL, N_( "Lower Sorbian" ) },
	  // language code: dua
	  { "dua", NULL, N_( "Duala" ) },
	  // language code: dum
	  { "dum", NULL, N_( "Dutch, Middle (ca.1050-1350)" ) },
	  // language code: dut nld nl
	  { "dut", "nl", N_( "Dutch" ) },
	  // language code: dut nld nl
	  { "nld", NULL, N_( "Dutch" ) },
	  // language code: dyu
	  { "dyu", NULL, N_( "Dyula" ) },
	  // language code: dzo dz
	  { "dzo", "dz", N_( "Dzongkha" ) },
	  // language code: efi
	  { "efi", NULL, N_( "Efik" ) },
	  // language code: egy
	  { "egy", NULL, N_( "Egyptian (Ancient)" ) },
	  // language code: eka
	  { "eka", NULL, N_( "Ekajuk" ) },
	  // language code: elx
	  { "elx", NULL, N_( "Elamite" ) },
	  // language code: eng en
	  { "eng", "en", N_( "English" ) },
	  // language code: enm
	  { "enm", NULL, N_( "English, Middle (1100-1500)" ) },
	  // language code: epo eo
	  { "epo", "eo", N_( "Esperanto" ) },
	  // language code: est et
	  { "est", "et", N_( "Estonian" ) },
	  // language code: ewe ee
	  { "ewe", "ee", N_( "Ewe" ) },
	  // language code: ewo
	  { "ewo", NULL, N_( "Ewondo" ) },
	  // language code: fan
	  { "fan", NULL, N_( "Fang" ) },
	  // language code: fao fo
	  { "fao", "fo", N_( "Faroese" ) },
	  // language code: fat
	  { "fat", NULL, N_( "Fanti" ) },
	  // language code: fij fj
	  { "fij", "fj", N_( "Fijian" ) },
	  // language code: fil
	  { "fil", NULL, N_( "Filipino" ) },
	  // language code: fin fi
	  { "fin", "fi", N_( "Finnish" ) },
	  // language code: fiu
	  { "fiu", NULL, N_( "Finno-Ugrian (Other)" ) },
	  // language code: fon
	  { "fon", NULL, N_( "Fon" ) },
	  // language code: fre fra fr
	  { "fre", "fr", N_( "French" ) },
	  // language code: fre fra fr
	  { "fra", NULL, N_( "French" ) },
	  // language code: frm
	  { "frm", NULL, N_( "French, Middle (ca.1400-1600)" ) },
	  // language code: fro
	  { "fro", NULL, N_( "French, Old (842-ca.1400)" ) },
	  // language code: fry fy
	  { "fry", "fy", N_( "Frisian" ) },
	  // language code: ful ff
	  { "ful", "ff", N_( "Fulah" ) },
	  // language code: fur
	  { "fur", NULL, N_( "Friulian" ) },
	  // language code: gaa
	  { "gaa", NULL, N_( "Ga" ) },
	  // language code: gay
	  { "gay", NULL, N_( "Gayo" ) },
	  // language code: gba
	  { "gba", NULL, N_( "Gbaya" ) },
	  // language code: gem
	  { "gem", NULL, N_( "Germanic (Other)" ) },
	  // language code: geo kat ka
	  { "geo", "ka", N_( "Georgian" ) },
	  // language code: geo kat ka
	  { "kat", NULL, N_( "Georgian" ) },
	  // language code: ger deu de
	  { "ger", "de", N_( "German" ) },
	  // language code: ger deu de
	  { "deu", NULL, N_( "German" ) },
	  // language code: gez
	  { "gez", NULL, N_( "Geez" ) },
	  // language code: gil
	  { "gil", NULL, N_( "Gilbertese" ) },
	  // language code: gla gd
	  { "gla", "gd", N_( "Gaelic" ) },
	  // language code: gle ga
	  { "gle", "ga", N_( "Irish" ) },
	  // language code: glg gl
	  { "glg", "gl", N_( "Galician" ) },
	  // language code: glv gv
	  { "glv", "gv", N_( "Manx" ) },
	  // language code: gmh
	  { "gmh", NULL, N_( "German, Middle High (ca.1050-1500)" ) },
	  // language code: goh
	  { "goh", NULL, N_( "German, Old High (ca.750-1050)" ) },
	  // language code: gon
	  { "gon", NULL, N_( "Gondi" ) },
	  // language code: gor
	  { "gor", NULL, N_( "Gorontalo" ) },
	  // language code: got
	  { "got", NULL, N_( "Gothic" ) },
	  // language code: grb
	  { "grb", NULL, N_( "Grebo" ) },
	  // language code: grc
	  { "grc", NULL, N_( "Greek, Ancient (to 1453)" ) },
	  // language code: gre ell el
	  { "gre", "el", N_( "Greek, Modern (1453-)" ) },
	  // language code: gre ell el
	  { "ell", NULL, N_( "Greek, Modern (1453-)" ) },
	  // language code: grn gn
	  { "grn", "gn", N_( "Guarani" ) },
	  // language code: guj gu
	  { "guj", "gu", N_( "Gujarati" ) },
	  // language code: gwi
	  { "gwi", NULL, N_( "Gwich'in" ) },
	  // language code: hai
	  { "hai", NULL, N_( "Haida" ) },
	  // language code: hat ht
	  { "hat", "ht", N_( "Haitian" ) },
	  // language code: hau ha
	  { "hau", "ha", N_( "Hausa" ) },
	  // language code: haw
	  { "haw", NULL, N_( "Hawaiian" ) },
	  // language code: heb he
	  { "heb", "he", N_( "Hebrew" ) },
	  // language code: her hz
	  { "her", "hz", N_( "Herero" ) },
	  // language code: hil
	  { "hil", NULL, N_( "Hiligaynon" ) },
	  // language code: him
	  { "him", NULL, N_( "Himachali" ) },
	  // language code: hin hi
	  { "hin", "hi", N_( "Hindi" ) },
	  // language code: hit
	  { "hit", NULL, N_( "Hittite" ) },
	  // language code: hmn
	  { "hmn", NULL, N_( "Hmong" ) },
	  // language code: hmo ho
	  { "hmo", "ho", N_( "Hiri Motu" ) },
	  // language code: hsb
	  { "hsb", NULL, N_( "Upper Sorbian" ) },
	  // language code: hun hu
	  { "hun", "hu", N_( "Hungarian" ) },
	  // language code: hup
	  { "hup", NULL, N_( "Hupa" ) },
	  // language code: iba
	  { "iba", NULL, N_( "Iban" ) },
	  // language code: ibo ig
	  { "ibo", "ig", N_( "Igbo" ) },
	  // language code: ice isl is
	  { "ice", "is", N_( "Icelandic" ) },
	  // language code: ice isl is
	  { "isl", NULL, N_( "Icelandic" ) },
	  // language code: ido io
	  { "ido", "io", N_( "Ido" ) },
	  // language code: iii ii
	  { "iii", "ii", N_( "Sichuan Yi" ) },
	  // language code: ijo
	  { "ijo", NULL, N_( "Ijo" ) },
	  // language code: iku iu
	  { "iku", "iu", N_( "Inuktitut" ) },
	  // language code: ile ie
	  { "ile", "ie", N_( "Interlingue" ) },
	  // language code: ilo
	  { "ilo", NULL, N_( "Iloko" ) },
	  // language code: ina ia
	  { "ina", "ia", N_( "Interlingua (International Auxiliary Language Association)" ) },
	  // language code: inc
	  { "inc", NULL, N_( "Indic (Other)" ) },
	  // language code: ind id
	  { "ind", "id", N_( "Indonesian" ) },
	  // language code: ine
	  { "ine", NULL, N_( "Indo-European (Other)" ) },
	  // language code: inh
	  { "inh", NULL, N_( "Ingush" ) },
	  // language code: ipk ik
	  { "ipk", "ik", N_( "Inupiaq" ) },
	  // language code: ira
	  { "ira", NULL, N_( "Iranian (Other)" ) },
	  // language code: iro
	  { "iro", NULL, N_( "Iroquoian Languages" ) },
	  // language code: ita it
	  { "ita", "it", N_( "Italian" ) },
	  // language code: jav jv
	  { "jav", "jv", N_( "Javanese" ) },
	  // language code: jbo
	  { "jbo", NULL, N_( "Lojban" ) },
	  // language code: jpn ja
	  { "jpn", "ja", N_( "Japanese" ) },
	  // language code: jpr
	  { "jpr", NULL, N_( "Judeo-Persian" ) },
	  // language code: jrb
	  { "jrb", NULL, N_( "Judeo-Arabic" ) },
	  // language code: kaa
	  { "kaa", NULL, N_( "Kara-Kalpak" ) },
	  // language code: kab
	  { "kab", NULL, N_( "Kabyle" ) },
	  // language code: kac
	  { "kac", NULL, N_( "Kachin" ) },
	  // language code: kal kl
	  { "kal", "kl", N_( "Kalaallisut" ) },
	  // language code: kam
	  { "kam", NULL, N_( "Kamba" ) },
	  // language code: kan kn
	  { "kan", "kn", N_( "Kannada" ) },
	  // language code: kar
	  { "kar", NULL, N_( "Karen" ) },
	  // language code: kas ks
	  { "kas", "ks", N_( "Kashmiri" ) },
	  // language code: kau kr
	  { "kau", "kr", N_( "Kanuri" ) },
	  // language code: kaw
	  { "kaw", NULL, N_( "Kawi" ) },
	  // language code: kaz kk
	  { "kaz", "kk", N_( "Kazakh" ) },
	  // language code: kbd
	  { "kbd", NULL, N_( "Kabardian" ) },
	  // language code: kha
	  { "kha", NULL, N_( "Khasi" ) },
	  // language code: khi
	  { "khi", NULL, N_( "Khoisan (Other)" ) },
	  // language code: khm km
	  { "khm", "km", N_( "Khmer" ) },
	  // language code: kho
	  { "kho", NULL, N_( "Khotanese" ) },
	  // language code: kik ki
	  { "kik", "ki", N_( "Kikuyu" ) },
	  // language code: kin rw
	  { "kin", "rw", N_( "Kinyarwanda" ) },
	  // language code: kir ky
	  { "kir", "ky", N_( "Kirghiz" ) },
	  // language code: kmb
	  { "kmb", NULL, N_( "Kimbundu" ) },
	  // language code: kok
	  { "kok", NULL, N_( "Konkani" ) },
	  // language code: kom kv
	  { "kom", "kv", N_( "Komi" ) },
	  // language code: kon kg
	  { "kon", "kg", N_( "Kongo" ) },
	  // language code: kor ko
	  { "kor", "ko", N_( "Korean" ) },
	  // language code: kos
	  { "kos", NULL, N_( "Kosraean" ) },
	  // language code: kpe
	  { "kpe", NULL, N_( "Kpelle" ) },
	  // language code: krc
	  { "krc", NULL, N_( "Karachay-Balkar" ) },
	  // language code: kro
	  { "kro", NULL, N_( "Kru" ) },
	  // language code: kru
	  { "kru", NULL, N_( "Kurukh" ) },
	  // language code: kua kj
	  { "kua", "kj", N_( "Kuanyama" ) },
	  // language code: kum
	  { "kum", NULL, N_( "Kumyk" ) },
	  // language code: kur ku
	  { "kur", "ku", N_( "Kurdish" ) },
	  // language code: kut
	  { "kut", NULL, N_( "Kutenai" ) },
	  // language code: lad
	  { "lad", NULL, N_( "Ladino" ) },
	  // language code: lah
	  { "lah", NULL, N_( "Lahnda" ) },
	  // language code: lam
	  { "lam", NULL, N_( "Lamba" ) },
	  // language code: lao lo
	  { "lao", "lo", N_( "Lao" ) },
	  // language code: lat la
	  { "lat", "la", N_( "Latin" ) },
	  // language code: lav lv
	  { "lav", "lv", N_( "Latvian" ) },
	  // language code: lez
	  { "lez", NULL, N_( "Lezghian" ) },
	  // language code: lim li
	  { "lim", "li", N_( "Limburgan" ) },
	  // language code: lin ln
	  { "lin", "ln", N_( "Lingala" ) },
	  // language code: lit lt
	  { "lit", "lt", N_( "Lithuanian" ) },
	  // language code: lol
	  { "lol", NULL, N_( "Mongo" ) },
	  // language code: loz
	  { "loz", NULL, N_( "Lozi" ) },
	  // language code: ltz lb
	  { "ltz", "lb", N_( "Luxembourgish" ) },
	  // language code: lua
	  { "lua", NULL, N_( "Luba-Lulua" ) },
	  // language code: lub lu
	  { "lub", "lu", N_( "Luba-Katanga" ) },
	  // language code: lug lg
	  { "lug", "lg", N_( "Ganda" ) },
	  // language code: lui
	  { "lui", NULL, N_( "Luiseno" ) },
	  // language code: lun
	  { "lun", NULL, N_( "Lunda" ) },
	  // language code: luo
	  { "luo", NULL, N_( "Luo (Kenya and Tanzania)" ) },
	  // language code: lus
	  { "lus", NULL, N_( "Lushai" ) },
	  // language code: mac mkd mk
	  { "mac", "mk", N_( "Macedonian" ) },
	  // language code: mac mkd mk
	  { "mkd", NULL, N_( "Macedonian" ) },
	  // language code: mad
	  { "mad", NULL, N_( "Madurese" ) },
	  // language code: mag
	  { "mag", NULL, N_( "Magahi" ) },
	  // language code: mah mh
	  { "mah", "mh", N_( "Marshallese" ) },
	  // language code: mai
	  { "mai", NULL, N_( "Maithili" ) },
	  // language code: mak
	  { "mak", NULL, N_( "Makasar" ) },
	  // language code: mal ml
	  { "mal", "ml", N_( "Malayalam" ) },
	  // language code: man
	  { "man", NULL, N_( "Mandingo" ) },
	  // language code: mao mri mi
	  { "mao", "mi", N_( "Maori" ) },
	  // language code: mao mri mi
	  { "mri", NULL, N_( "Maori" ) },
	  // language code: map
	  { "map", NULL, N_( "Austronesian (Other)" ) },
	  // language code: mar mr
	  { "mar", "mr", N_( "Marathi" ) },
	  // language code: mas
	  { "mas", NULL, N_( "Masai" ) },
	  // language code: may msa ms
	  { "may", "ms", N_( "Malay" ) },
	  // language code: may msa ms
	  { "msa", NULL, N_( "Malay" ) },
	  // language code: mdf
	  { "mdf", NULL, N_( "Moksha" ) },
	  // language code: mdr
	  { "mdr", NULL, N_( "Mandar" ) },
	  // language code: men
	  { "men", NULL, N_( "Mende" ) },
	  // language code: mga
	  { "mga", NULL, N_( "Irish, Middle (900-1200)" ) },
	  // language code: mic
	  { "mic", NULL, N_( "Mi'kmaq" ) },
	  // language code: min
	  { "min", NULL, N_( "Minangkabau" ) },
	  // language code: mis
	  { "mis", NULL, N_( "Miscellaneous Languages" ) },
	  // language code: mkh
	  { "mkh", NULL, N_( "Mon-Khmer (Other)" ) },
	  // language code: mlg mg
	  { "mlg", "mg", N_( "Malagasy" ) },
	  // language code: mlt mt
	  { "mlt", "mt", N_( "Maltese" ) },
	  // language code: mnc
	  { "mnc", NULL, N_( "Manchu" ) },
	  // language code: mni
	  { "mni", NULL, N_( "Manipuri" ) },
	  // language code: mno
	  { "mno", NULL, N_( "Manobo Languages" ) },
	  // language code: moh
	  { "moh", NULL, N_( "Mohawk" ) },
	  // language code: mol mo
	  { "mol", "mo", N_( "Moldavian" ) },
	  // language code: mon mn
	  { "mon", "mn", N_( "Mongolian" ) },
	  // language code: mos
	  { "mos", NULL, N_( "Mossi" ) },
	  // language code: mul
	  { "mul", NULL, N_( "Multiple Languages" ) },
	  // language code: mun
	  { "mun", NULL, N_( "Munda languages" ) },
	  // language code: mus
	  { "mus", NULL, N_( "Creek" ) },
	  // language code: mwl
	  { "mwl", NULL, N_( "Mirandese" ) },
	  // language code: mwr
	  { "mwr", NULL, N_( "Marwari" ) },
	  // language code: myn
	  { "myn", NULL, N_( "Mayan Languages" ) },
	  // language code: myv
	  { "myv", NULL, N_( "Erzya" ) },
	  // language code: nah
	  { "nah", NULL, N_( "Nahuatl" ) },
	  // language code: nai
	  { "nai", NULL, N_( "North American Indian" ) },
	  // language code: nap
	  { "nap", NULL, N_( "Neapolitan" ) },
	  // language code: nau na
	  { "nau", "na", N_( "Nauru" ) },
	  // language code: nav nv
	  { "nav", "nv", N_( "Navajo" ) },
	  // language code: nbl nr
	  { "nbl", "nr", N_( "Ndebele, South" ) },
	  // language code: nde nd
	  { "nde", "nd", N_( "Ndebele, North" ) },
	  // language code: ndo ng
	  { "ndo", "ng", N_( "Ndonga" ) },
	  // language code: nds
	  { "nds", NULL, N_( "Low German" ) },
	  // language code: nep ne
	  { "nep", "ne", N_( "Nepali" ) },
	  // language code: new
	  { "new", NULL, N_( "Nepal Bhasa" ) },
	  // language code: nia
	  { "nia", NULL, N_( "Nias" ) },
	  // language code: nic
	  { "nic", NULL, N_( "Niger-Kordofanian (Other)" ) },
	  // language code: niu
	  { "niu", NULL, N_( "Niuean" ) },
	  // language code: nno nn
	  { "nno", "nn", N_( "Norwegian Nynorsk" ) },
	  // language code: nob nb
	  { "nob", "nb", N_( "Norwegian Bokmal" ) },
	  // language code: nog
	  { "nog", NULL, N_( "Nogai" ) },
	  // language code: non
	  { "non", NULL, N_( "Norse, Old" ) },
	  // language code: nor no
	  { "nor", "no", N_( "Norwegian" ) },
	  // language code: nso
	  { "nso", NULL, N_( "Northern Sotho" ) },
	  // language code: nub
	  { "nub", NULL, N_( "Nubian Languages" ) },
	  // language code: nwc
	  { "nwc", NULL, N_( "Classical Newari" ) },
	  // language code: nya ny
	  { "nya", "ny", N_( "Chichewa" ) },
	  // language code: nym
	  { "nym", NULL, N_( "Nyamwezi" ) },
	  // language code: nyn
	  { "nyn", NULL, N_( "Nyankole" ) },
	  // language code: nyo
	  { "nyo", NULL, N_( "Nyoro" ) },
	  // language code: nzi
	  { "nzi", NULL, N_( "Nzima" ) },
	  // language code: oci oc
	  { "oci", "oc", N_( "Occitan (post 1500)" ) },
	  // language code: oji oj
	  { "oji", "oj", N_( "Ojibwa" ) },
	  // language code: ori or
	  { "ori", "or", N_( "Oriya" ) },
	  // language code: orm om
	  { "orm", "om", N_( "Oromo" ) },
	  // language code: osa
	  { "osa", NULL, N_( "Osage" ) },
	  // language code: oss os
	  { "oss", "os", N_( "Ossetian" ) },
	  // language code: ota
	  { "ota", NULL, N_( "Turkish, Ottoman (1500-1928)" ) },
	  // language code: oto
	  { "oto", NULL, N_( "Otomian Languages" ) },
	  // language code: paa
	  { "paa", NULL, N_( "Papuan (Other)" ) },
	  // language code: pag
	  { "pag", NULL, N_( "Pangasinan" ) },
	  // language code: pal
	  { "pal", NULL, N_( "Pahlavi" ) },
	  // language code: pam
	  { "pam", NULL, N_( "Pampanga" ) },
	  // language code: pan pa
	  { "pan", "pa", N_( "Panjabi" ) },
	  // language code: pap
	  { "pap", NULL, N_( "Papiamento" ) },
	  // language code: pau
	  { "pau", NULL, N_( "Palauan" ) },
	  // language code: peo
	  { "peo", NULL, N_( "Persian, Old (ca.600-400 B.C.)" ) },
	  // language code: per fas fa
	  { "per", "fa", N_( "Persian" ) },
	  // language code: per fas fa
	  { "fas", NULL, N_( "Persian" ) },
	  // language code: phi
	  { "phi", NULL, N_( "Philippine (Other)" ) },
	  // language code: phn
	  { "phn", NULL, N_( "Phoenician" ) },
	  // language code: pli pi
	  { "pli", "pi", N_( "Pali" ) },
	  // language code: pol pl
	  { "pol", "pl", N_( "Polish" ) },
	  // language code: pon
	  { "pon", NULL, N_( "Pohnpeian" ) },
	  // language code: por pt
	  { "por", "pt", N_( "Portuguese" ) },
	  // language code: pra
	  { "pra", NULL, N_( "Prakrit Languages" ) },
	  // language code: pro
	  { "pro", NULL, N_( "Provencal, Old (to 1500)" ) },
	  // language code: pus ps
	  { "pus", "ps", N_( "Pushto" ) },
	  // language code: que qu
	  { "que", "qu", N_( "Quechua" ) },
	  // language code: raj
	  { "raj", NULL, N_( "Rajasthani" ) },
	  // language code: rap
	  { "rap", NULL, N_( "Rapanui" ) },
	  // language code: rar
	  { "rar", NULL, N_( "Rarotongan" ) },
	  // language code: roa
	  { "roa", NULL, N_( "Romance (Other)" ) },
	  // language code: roh rm
	  { "roh", "rm", N_( "Raeto-Romance" ) },
	  // language code: rom
	  { "rom", NULL, N_( "Romany" ) },
	  // language code: rum ron ro
	  { "rum", "ro", N_( "Romanian" ) },
	  // language code: rum ron ro
	  { "ron", NULL, N_( "Romanian" ) },
	  // language code: run rn
	  { "run", "rn", N_( "Rundi" ) },
	  // language code: rus ru
	  { "rus", "ru", N_( "Russian" ) },
	  // language code: sad
	  { "sad", NULL, N_( "Sandawe" ) },
	  // language code: sag sg
	  { "sag", "sg", N_( "Sango" ) },
	  // language code: sah
	  { "sah", NULL, N_( "Yakut" ) },
	  // language code: sai
	  { "sai", NULL, N_( "South American Indian (Other)" ) },
	  // language code: sal
	  { "sal", NULL, N_( "Salishan Languages" ) },
	  // language code: sam
	  { "sam", NULL, N_( "Samaritan Aramaic" ) },
	  // language code: san sa
	  { "san", "sa", N_( "Sanskrit" ) },
	  // language code: sas
	  { "sas", NULL, N_( "Sasak" ) },
	  // language code: sat
	  { "sat", NULL, N_( "Santali" ) },
	  // language code: scc srp sr
	  { "scc", "sr", N_( "Serbian" ) },
	  // language code: scc srp sr
	  { "srp", NULL, N_( "Serbian" ) },
	  // language code: scn
	  { "scn", NULL, N_( "Sicilian" ) },
	  // language code: sco
	  { "sco", NULL, N_( "Scots" ) },
	  // language code: scr hrv hr
	  { "scr", "hr", N_( "Croatian" ) },
	  // language code: scr hrv hr
	  { "hrv", NULL, N_( "Croatian" ) },
	  // language code: sel
	  { "sel", NULL, N_( "Selkup" ) },
	  // language code: sem
	  { "sem", NULL, N_( "Semitic (Other)" ) },
	  // language code: sga
	  { "sga", NULL, N_( "Irish, Old (to 900)" ) },
	  // language code: sgn
	  { "sgn", NULL, N_( "Sign Languages" ) },
	  // language code: shn
	  { "shn", NULL, N_( "Shan" ) },
	  // language code: sid
	  { "sid", NULL, N_( "Sidamo" ) },
	  // language code: sin si
	  { "sin", "si", N_( "Sinhala" ) },
	  // language code: sio
	  { "sio", NULL, N_( "Siouan Languages" ) },
	  // language code: sit
	  { "sit", NULL, N_( "Sino-Tibetan (Other)" ) },
	  // language code: sla
	  { "sla", NULL, N_( "Slavic (Other)" ) },
	  // language code: slo slk sk
	  { "slo", "sk", N_( "Slovak" ) },
	  // language code: slo slk sk
	  { "slk", NULL, N_( "Slovak" ) },
	  // language code: slv sl
	  { "slv", "sl", N_( "Slovenian" ) },
	  // language code: sma
	  { "sma", NULL, N_( "Southern Sami" ) },
	  // language code: sme se
	  { "sme", "se", N_( "Northern Sami" ) },
	  // language code: smi
	  { "smi", NULL, N_( "Sami Languages (Other)" ) },
	  // language code: smj
	  { "smj", NULL, N_( "Lule Sami" ) },
	  // language code: smn
	  { "smn", NULL, N_( "Inari Sami" ) },
	  // language code: smo sm
	  { "smo", "sm", N_( "Samoan" ) },
	  // language code: sms
	  { "sms", NULL, N_( "Skolt Sami" ) },
	  // language code: sna sn
	  { "sna", "sn", N_( "Shona" ) },
	  // language code: snd sd
	  { "snd", "sd", N_( "Sindhi" ) },
	  // language code: snk
	  { "snk", NULL, N_( "Soninke" ) },
	  // language code: sog
	  { "sog", NULL, N_( "Sogdian" ) },
	  // language code: som so
	  { "som", "so", N_( "Somali" ) },
	  // language code: son
	  { "son", NULL, N_( "Songhai" ) },
	  // language code: sot st
	  { "sot", "st", N_( "Sotho, Southern" ) },
	  // language code: spa es
	  { "spa", "es", N_( "Spanish" ) },
	  // language code: srd sc
	  { "srd", "sc", N_( "Sardinian" ) },
	  // language code: srr
	  { "srr", NULL, N_( "Serer" ) },
	  // language code: ssa
	  { "ssa", NULL, N_( "Nilo-Saharan (Other)" ) },
	  // language code: ssw ss
	  { "ssw", "ss", N_( "Swati" ) },
	  // language code: suk
	  { "suk", NULL, N_( "Sukuma" ) },
	  // language code: sun su
	  { "sun", "su", N_( "Sundanese" ) },
	  // language code: sus
	  { "sus", NULL, N_( "Susu" ) },
	  // language code: sux
	  { "sux", NULL, N_( "Sumerian" ) },
	  // language code: swa sw
	  { "swa", "sw", N_( "Swahili" ) },
	  // language code: swe sv
	  { "swe", "sv", N_( "Swedish" ) },
	  // language code: syr
	  { "syr", NULL, N_( "Syriac" ) },
	  // language code: tah ty
	  { "tah", "ty", N_( "Tahitian" ) },
	  // language code: tai
	  { "tai", NULL, N_( "Tai (Other)" ) },
	  // language code: tam ta
	  { "tam", "ta", N_( "Tamil" ) },
	  // language code: tat tt
	  { "tat", "tt", N_( "Tatar" ) },
	  // language code: tel te
	  { "tel", "te", N_( "Telugu" ) },
	  // language code: tem
	  { "tem", NULL, N_( "Timne" ) },
	  // language code: ter
	  { "ter", NULL, N_( "Tereno" ) },
	  // language code: tet
	  { "tet", NULL, N_( "Tetum" ) },
	  // language code: tgk tg
	  { "tgk", "tg", N_( "Tajik" ) },
	  // language code: tgl tl
	  { "tgl", "tl", N_( "Tagalog" ) },
	  // language code: tha th
	  { "tha", "th", N_( "Thai" ) },
	  // language code: tib bod bo
	  { "tib", "bo", N_( "Tibetan" ) },
	  // language code: tib bod bo
	  { "bod", NULL, N_( "Tibetan" ) },
	  // language code: tig
	  { "tig", NULL, N_( "Tigre" ) },
	  // language code: tir ti
	  { "tir", "ti", N_( "Tigrinya" ) },
	  // language code: tiv
	  { "tiv", NULL, N_( "Tiv" ) },
	  // language code: tkl
	  { "tkl", NULL, N_( "Tokelau" ) },
	  // language code: tlh
	  { "tlh", NULL, N_( "Klingon" ) },
	  // language code: tli
	  { "tli", NULL, N_( "Tlingit" ) },
	  // language code: tmh
	  { "tmh", NULL, N_( "Tamashek" ) },
	  // language code: tog
	  { "tog", NULL, N_( "Tonga (Nyasa)" ) },
	  // language code: ton to
	  { "ton", "to", N_( "Tonga (Tonga Islands)" ) },
	  // language code: tpi
	  { "tpi", NULL, N_( "Tok Pisin" ) },
	  // language code: tsi
	  { "tsi", NULL, N_( "Tsimshian" ) },
	  // language code: tsn tn
	  { "tsn", "tn", N_( "Tswana" ) },
	  // language code: tso ts
	  { "tso", "ts", N_( "Tsonga" ) },
	  // language code: tuk tk
	  { "tuk", "tk", N_( "Turkmen" ) },
	  // language code: tum
	  { "tum", NULL, N_( "Tumbuka" ) },
	  // language code: tup
	  { "tup", NULL, N_( "Tupi Languages" ) },
	  // language code: tur tr
	  { "tur", "tr", N_( "Turkish" ) },
	  // language code: tut
	  { "tut", NULL, N_( "Altaic (Other)" ) },
	  // language code: tvl
	  { "tvl", NULL, N_( "Tuvalu" ) },
	  // language code: twi tw
	  { "twi", "tw", N_( "Twi" ) },
	  // language code: tyv
	  { "tyv", NULL, N_( "Tuvinian" ) },
	  // language code: udm
	  { "udm", NULL, N_( "Udmurt" ) },
	  // language code: uga
	  { "uga", NULL, N_( "Ugaritic" ) },
	  // language code: uig ug
	  { "uig", "ug", N_( "Uighur" ) },
	  // language code: ukr uk
	  { "ukr", "uk", N_( "Ukrainian" ) },
	  // language code: umb
	  { "umb", NULL, N_( "Umbundu" ) },
	  // language code: und
	  { "und", NULL, N_( "Undetermined" ) },
	  // language code: urd ur
	  { "urd", "ur", N_( "Urdu" ) },
	  // language code: uzb uz
	  { "uzb", "uz", N_( "Uzbek" ) },
	  // language code: vai
	  { "vai", NULL, N_( "Vai" ) },
	  // language code: ven ve
	  { "ven", "ve", N_( "Venda" ) },
	  // language code: vie vi
	  { "vie", "vi", N_( "Vietnamese" ) },
	  // language code: vol vo
	  { "vol", "vo", N_( "Volapuk" ) },
	  // language code: vot
	  { "vot", NULL, N_( "Votic" ) },
	  // language code: wak
	  { "wak", NULL, N_( "Wakashan Languages" ) },
	  // language code: wal
	  { "wal", NULL, N_( "Walamo" ) },
	  // language code: war
	  { "war", NULL, N_( "Waray" ) },
	  // language code: was
	  { "was", NULL, N_( "Washo" ) },
	  // language code: wel cym cy
	  { "wel", "cy", N_( "Welsh" ) },
	  // language code: wel cym cy
	  { "cym", NULL, N_( "Welsh" ) },
	  // language code: wen
	  { "wen", NULL, N_( "Sorbian Languages" ) },
	  // language code: wln wa
	  { "wln", "wa", N_( "Walloon" ) },
	  // language code: wol wo
	  { "wol", "wo", N_( "Wolof" ) },
	  // language code: xal
	  { "xal", NULL, N_( "Kalmyk" ) },
	  // language code: xho xh
	  { "xho", "xh", N_( "Xhosa" ) },
	  // language code: yao
	  { "yao", NULL, N_( "Yao" ) },
	  // language code: yap
	  { "yap", NULL, N_( "Yapese" ) },
	  // language code: yid yi
	  { "yid", "yi", N_( "Yiddish" ) },
	  // language code: yor yo
	  { "yor", "yo", N_( "Yoruba" ) },
	  // language code: ypk
	  { "ypk", NULL, N_( "Yupik Languages" ) },
	  // language code: zap
	  { "zap", NULL, N_( "Zapotec" ) },
	  // language code: zen
	  { "zen", NULL, N_( "Zenaga" ) },
	  // language code: zha za
	  { "zha", "za", N_( "Zhuang" ) },
	  // language code: znd
	  { "znd", NULL, N_( "Zande" ) },
	  // language code: zul zu
	  { "zul", "zu", N_( "Zulu" ) },
	  // language code: zun
	  { "zun", NULL, N_( "Zuni" ) },

	  { NULL, NULL, NULL }
      };

      for (const LangInit * i = langInit; i->iso639_2 != NULL; ++i)
      {
	  std::string name( i->name );
	  codes[i->iso639_2] = name;
	  if (i->iso639_1 != NULL)
	      codes[i->iso639_1] = name;
      }
    }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
