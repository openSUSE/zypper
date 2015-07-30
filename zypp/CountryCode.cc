/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CountryCode.cc
 *
*/
#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Hash.h"

#include "zypp/CountryCode.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace
  {

    /** Wrap static codemap data. */
    struct CodeMaps // singleton
    {
      /** The singleton */
      static CodeMaps & instance()
      {
	static CodeMaps _instance;
	return _instance;
      }

      /** Lookup (translated) name for \a index_r.*/
      std::string name( IdString index_r )
      {
	Link link( getIndex( index_r ) );

	std::string ret;
	if ( link->second )
	{ ret = _(link->second); }
	else
	{
	  ret = _("Unknown country: ");
	  ret += "'";
	  ret += index_r.c_str();
	  ret += "'";
	}
	return ret;
      }

    private:
      typedef std::unordered_map<std::string,const char *> CodeMap;
      typedef CodeMap::const_iterator Link;

      typedef std::unordered_map<IdString,Link> IndexMap;

      /** Ctor initializes the code maps.
       * http://www.iso.org/iso/en/prods-services/iso3166ma/02iso-3166-code-lists/list-en1.html
       */
      CodeMaps();

      /** Return \ref Link for \a index_r, creating it if necessary. */
      Link getIndex( IdString index_r )
      {
	auto it = _indexMap.find( index_r );
	return( it != _indexMap.end()
	      ? it->second
	      : newIndex( index_r, index_r.asString() ) );
      }

      /** Return the CodeMap Index for \a code_r. */
      Link newIndex( IdString index_r, const std::string & code_r )
      {
	Link link = _codeMap.find( code_r );
	if ( link != _codeMap.end() )
	  return (_indexMap[index_r] = link);

	// not found: Remember a new code
	CodeMap::value_type nval( code_r, nullptr );

	if ( code_r.size() != 2 )
	  WAR << "Malformed CountryCode '" << code_r << "' (expect 2-letter)" << endl;

	std::string ucode( str::toUpper( code_r ) );
	if ( ucode != code_r )
	{
	  WAR << "Malformed CountryCode '" << code_r << "' (not upper case)" << endl;
	  // but maybe we're lucky with the lower case code
	  // and find a language name.
	  link = _codeMap.find( ucode );
	  if ( link != _codeMap.end() )
	  {
	    nval.second = link->second;
	  }
	}
	MIL << "Remember CountryCode '" << code_r << "': '" << nval.second << "'" << endl;
	return (_indexMap[index_r] = _codeMap.insert( nval ).first);
      }

    private:
      CodeMap _codeMap;
      IndexMap _indexMap;
    };
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //	class CountryCode
  ///////////////////////////////////////////////////////////////////

  const CountryCode CountryCode::noCode;

  CountryCode::CountryCode()
  {}

  CountryCode::CountryCode( IdString str_r )
  : _str( str_r )
  {}

  CountryCode::CountryCode( const std::string & str_r )
  : _str( str_r )
  {}

  CountryCode::CountryCode( const char * str_r )
  : _str( str_r )
  {}

  CountryCode::~CountryCode()
  {}


  std::string CountryCode::name() const
  { return CodeMaps::instance().name( _str ); }

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    CodeMaps::CodeMaps()
    {
      // Defined CountryCode constants
      _codeMap[""]        = N_( "No Code" );

      struct Init
      {
	  const char *iso3166;
	  const char *name;
      };

      const Init init[] = {
          {"AD", N_( "Andorra" ) }, 				// :AND:020:
          {"AE", N_( "United Arab Emirates" ) }, 		// :ARE:784:
          {"AF", N_( "Afghanistan" ) }, 			// :AFG:004:
          {"AG", N_( "Antigua and Barbuda" ) }, 		// :ATG:028:
          {"AI", N_( "Anguilla" ) }, 				// :AIA:660:
          {"AL", N_( "Albania" ) }, 				// :ALB:008:
          {"AM", N_( "Armenia" ) }, 				// :ARM:051:
          {"AN", N_( "Netherlands Antilles" ) }, 		// :ANT:530:
          {"AO", N_( "Angola" ) }, 				// :AGO:024:
          {"AQ", N_( "Antarctica" ) }, 				// :ATA:010:
          {"AR", N_( "Argentina" ) },	 			// :ARG:032:
          {"AS", N_( "American Samoa" ) }, 			// :ASM:016:
          {"AT", N_( "Austria" ) }, 				// :AUT:040:
          {"AU", N_( "Australia" ) }, 				// :AUS:036:
          {"AW", N_( "Aruba" ) }, 				// :ABW:533:
          {"AX", N_( "Aland Islands" ) }, 			// :ALA:248:
          {"AZ", N_( "Azerbaijan" ) }, 				// :AZE:031:
          {"BA", N_( "Bosnia and Herzegovina" ) }, 		// :BIH:070:
          {"BB", N_( "Barbados" ) }, 				// :BRB:052:
          {"BD", N_( "Bangladesh" ) }, 				// :BGD:050:
          {"BE", N_( "Belgium" ) }, 				// :BEL:056:
          {"BF", N_( "Burkina Faso" ) }, 			// :BFA:854:
          {"BG", N_( "Bulgaria" ) }, 				// :BGR:100:
          {"BH", N_( "Bahrain" ) }, 				// :BHR:048:
          {"BI", N_( "Burundi" ) }, 				// :BDI:108:
          {"BJ", N_( "Benin" ) }, 				// :BEN:204:
          {"BM", N_( "Bermuda" ) }, 				// :BMU:060:
          {"BN", N_( "Brunei Darussalam" ) }, 			// :BRN:096:
          {"BO", N_( "Bolivia" ) }, 				// :BOL:068:
          {"BR", N_( "Brazil" ) }, 				// :BRA:076:
          {"BS", N_( "Bahamas" ) }, 				// :BHS:044:
          {"BT", N_( "Bhutan" ) }, 				// :BTN:064:
          {"BV", N_( "Bouvet Island" ) }, 			// :BVT:074:
          {"BW", N_( "Botswana" ) }, 				// :BWA:072:
          {"BY", N_( "Belarus" ) }, 				// :BLR:112:
          {"BZ", N_( "Belize" ) }, 				// :BLZ:084:
          {"CA", N_( "Canada" ) }, 				// :CAN:124:
          {"CC", N_( "Cocos (Keeling) Islands" ) }, 		// :CCK:166:
          {"CD", N_( "Congo" ) }, 				// :COD:180:
          {"CF", N_( "Central African Republic" ) }, 		// :CAF:140:
          {"CG", N_( "Congo" ) }, 				// :COG:178:
          {"CH", N_( "Switzerland" ) }, 			// :CHE:756:
          {"CI", N_( "Cote D'Ivoire" ) }, 			// :CIV:384:
          {"CK", N_( "Cook Islands" ) }, 			// :COK:184:
          {"CL", N_( "Chile" ) }, 				// :CHL:152:
          {"CM", N_( "Cameroon" ) }, 				// :CMR:120:
          {"CN", N_( "China" ) }, 				// :CHN:156:
          {"CO", N_( "Colombia" ) }, 				// :COL:170:
          {"CR", N_( "Costa Rica" ) }, 				// :CRI:188:
          {"CU", N_( "Cuba" ) }, 				// :CUB:192:
          {"CV", N_( "Cape Verde" ) }, 				// :CPV:132:
          {"CX", N_( "Christmas Island" ) }, 			// :CXR:162:
          {"CY", N_( "Cyprus" ) }, 				// :CYP:196:
          {"CZ", N_( "Czech Republic" ) }, 			// :CZE:203:
          {"DE", N_( "Germany" ) }, 				// :DEU:276:
          {"DJ", N_( "Djibouti" ) }, 				// :DJI:262:
          {"DK", N_( "Denmark" ) }, 				// :DNK:208:
          {"DM", N_( "Dominica" ) }, 				// :DMA:212:
          {"DO", N_( "Dominican Republic" ) }, 			// :DOM:214:
          {"DZ", N_( "Algeria" ) }, 				// :DZA:012:
          {"EC", N_( "Ecuador" ) }, 				// :ECU:218:
          {"EE", N_( "Estonia" ) }, 				// :EST:233:
          {"EG", N_( "Egypt" ) }, 				// :EGY:818:
          {"EH", N_( "Western Sahara" ) }, 			// :ESH:732:
          {"ER", N_( "Eritrea" ) }, 				// :ERI:232:
          {"ES", N_( "Spain" ) }, 				// :ESP:724:
          {"ET", N_( "Ethiopia" ) }, 				// :ETH:231:
          {"FI", N_( "Finland" ) }, 				// :FIN:246:
          {"FJ", N_( "Fiji" ) }, 				// :FJI:242:
          {"FK", N_( "Falkland Islands (Malvinas)" ) }, 	// :FLK:238:
          {"FM", N_( "Federated States of Micronesia" ) }, 	// :FSM:583:
          {"FO", N_( "Faroe Islands" ) }, 			// :FRO:234:
          {"FR", N_( "France" ) }, 				// :FRA:250:
          {"FX", N_( "Metropolitan France" ) }, 		// :FXX:249:
          {"GA", N_( "Gabon" ) }, 				// :GAB:266:
          {"GB", N_( "United Kingdom" ) }, 			// :GBR:826:
          {"GD", N_( "Grenada" ) }, 				// :GRD:308:
          {"GE", N_( "Georgia" ) }, 				// :GEO:268:
          {"GF", N_( "French Guiana" ) }, 			// :GUF:254:
          {"GG", N_( "Guernsey" ) },
          {"GH", N_( "Ghana" ) }, 				// :GHA:288:
          {"GI", N_( "Gibraltar" ) }, 				// :GIB:292:
          {"GL", N_( "Greenland" ) }, 				// :GRL:304:
          {"GM", N_( "Gambia" ) }, 				// :GMB:270:
          {"GN", N_( "Guinea" ) }, 				// :GIN:324:
          {"GP", N_( "Guadeloupe" ) }, 				// :GLP:312:
          {"GQ", N_( "Equatorial Guinea" ) }, 			// :GNQ:226:
          {"GR", N_( "Greece" ) }, 				// :GRC:300:
          {"GS", N_( "South Georgia and the South Sandwich Islands" ) },	// :SGS:239:
          {"GT", N_( "Guatemala" ) }, 				// :GTM:320:
          {"GU", N_( "Guam" ) }, 				// :GUM:316:
          {"GW", N_( "Guinea-Bissau" ) }, 			// :GNB:624:
          {"GY", N_( "Guyana" ) }, 				// :GUY:328:
          {"HK", N_( "Hong Kong" ) }, 				// :HKG:344:
          {"HM", N_( "Heard Island and McDonald Islands" ) }, // :HMD:334:
          {"HN", N_( "Honduras" ) }, 				// :HND:340:
          {"HR", N_( "Croatia" ) }, 				// :HRV:191:
          {"HT", N_( "Haiti" ) }, 				// :HTI:332:
          {"HU", N_( "Hungary" ) }, 				// :HUN:348:
          {"ID", N_( "Indonesia" ) }, 				// :IDN:360:
          {"IE", N_( "Ireland" ) }, 				// :IRL:372:
          {"IL", N_( "Israel" ) }, 				// :ISR:376:
          {"IM", N_( "Isle of Man" ) },
          {"IN", N_( "India" ) }, 				// :IND:356:
          {"IO", N_( "British Indian Ocean Territory" ) }, 	// :IOT:086:
          {"IQ", N_( "Iraq" ) }, 				// :IRQ:368:
          {"IR", N_( "Iran" ) }, 				// :IRN:364:
          {"IS", N_( "Iceland" ) }, 				// :ISL:352:
          {"IT", N_( "Italy" ) }, 				// :ITA:380:
          {"JE", N_( "Jersey" ) },
          {"JM", N_( "Jamaica" ) }, 				// :JAM:388:
          {"JO", N_( "Jordan" ) }, 				// :JOR:400:
          {"JP", N_( "Japan" ) }, 				// :JPN:392:
          {"KE", N_( "Kenya" ) }, 				// :KEN:404:
          {"KG", N_( "Kyrgyzstan" ) }, 				// :KGZ:417:
          {"KH", N_( "Cambodia" ) }, 				// :KHM:116:
          {"KI", N_( "Kiribati" ) }, 				// :KIR:296:
          {"KM", N_( "Comoros" ) }, 				// :COM:174:
          {"KN", N_( "Saint Kitts and Nevis" ) }, 		// :KNA:659:
          {"KP", N_( "North Korea" ) }, 			// :PRK:408:
          {"KR", N_( "South Korea" ) }, 			// :KOR:410:
          {"KW", N_( "Kuwait" ) }, 				// :KWT:414:
          {"KY", N_( "Cayman Islands" ) }, 			// :CYM:136:
          {"KZ", N_( "Kazakhstan" ) }, 				// :KAZ:398:
          {"LA", N_( "Lao People's Democratic Republic" ) },	// :LAO:418:
          {"LB", N_( "Lebanon" ) }, 				// :LBN:422:
          {"LC", N_( "Saint Lucia" ) }, 			// :LCA:662:
          {"LI", N_( "Liechtenstein" ) }, 			// :LIE:438:
          {"LK", N_( "Sri Lanka" ) }, 				// :LKA:144:
          {"LR", N_( "Liberia" ) }, 				// :LBR:430:
          {"LS", N_( "Lesotho" ) }, 				// :LSO:426:
          {"LT", N_( "Lithuania" ) }, 				// :LTU:440:
          {"LU", N_( "Luxembourg" ) }, 				// :LUX:442:
          {"LV", N_( "Latvia" ) }, 				// :LVA:428:
          {"LY", N_( "Libya" ) }, 				// :LBY:434:
          {"MA", N_( "Morocco" ) }, 				// :MAR:504:
          {"MC", N_( "Monaco" ) }, 				// :MCO:492:
          {"MD", N_( "Moldova" ) }, 				// :MDA:498:
          {"ME", N_( "Montenegro" ) },
          {"MF", N_( "Saint Martin" ) },
          {"MG", N_( "Madagascar" ) }, 				// :MDG:450:
          {"MH", N_( "Marshall Islands" ) }, 			// :MHL:584:
          {"MK", N_( "Macedonia" ) }, 				// :MKD:807:
          {"ML", N_( "Mali" ) }, 				// :MLI:466:
          {"MM", N_( "Myanmar" ) }, 				// :MMR:104:
          {"MN", N_( "Mongolia" ) }, 				// :MNG:496:
          {"MO", N_( "Macao" ) }, 				// :MAC:446:
          {"MP", N_( "Northern Mariana Islands" ) }, 		// :MNP:580:
          {"MQ", N_( "Martinique" ) }, 				// :MTQ:474:
          {"MR", N_( "Mauritania" ) }, 				// :MRT:478:
          {"MS", N_( "Montserrat" ) }, 				// :MSR:500:
          {"MT", N_( "Malta" ) }, 				// :MLT:470:
          {"MU", N_( "Mauritius" ) }, 				// :MUS:480:
          {"MV", N_( "Maldives" ) }, 				// :MDV:462:
          {"MW", N_( "Malawi" ) }, 				// :MWI:454:
          {"MX", N_( "Mexico" ) }, 				// :MEX:484:
          {"MY", N_( "Malaysia" ) }, 				// :MYS:458:
          {"MZ", N_( "Mozambique" ) }, 				// :MOZ:508:
          {"NA", N_( "Namibia" ) }, 				// :NAM:516:
          {"NC", N_( "New Caledonia" ) }, 			// :NCL:540:
          {"NE", N_( "Niger" ) }, 				// :NER:562:
          {"NF", N_( "Norfolk Island" ) }, 			// :NFK:574:
          {"NG", N_( "Nigeria" ) }, 				// :NGA:566:
          {"NI", N_( "Nicaragua" ) }, 				// :NIC:558:
          {"NL", N_( "Netherlands" ) }, 			// :NLD:528:
          {"NO", N_( "Norway" ) }, 				// :NOR:578:
          {"NP", N_( "Nepal" ) }, 				// :NPL:524:
          {"NR", N_( "Nauru" ) }, 				// :NRU:520:
          {"NU", N_( "Niue" ) }, 				// :NIU:570:
          {"NZ", N_( "New Zealand" ) }, 			// :NZL:554:
          {"OM", N_( "Oman" ) }, 				// :OMN:512:
          {"PA", N_( "Panama" ) }, 				// :PAN:591:
          {"PE", N_( "Peru" ) }, 				// :PER:604:
          {"PF", N_( "French Polynesia" ) }, 			// :PYF:258:
          {"PG", N_( "Papua New Guinea" ) }, 			// :PNG:598:
          {"PH", N_( "Philippines" ) }, 			// :PHL:608:
          {"PK", N_( "Pakistan" ) }, 				// :PAK:586:
          {"PL", N_( "Poland" ) }, 				// :POL:616:
          {"PM", N_( "Saint Pierre and Miquelon" ) }, 		// :SPM:666:
          {"PN", N_( "Pitcairn" ) }, 				// :PCN:612:
          {"PR", N_( "Puerto Rico" ) }, 			// :PRI:630:
          {"PS", N_( "Palestinian Territory" ) }, 		// :PSE:275:
          {"PT", N_( "Portugal" ) }, 				// :PRT:620:
          {"PW", N_( "Palau" ) }, 				// :PLW:585:
          {"PY", N_( "Paraguay" ) }, 				// :PRY:600:
          {"QA", N_( "Qatar" ) }, 				// :QAT:634:
          {"RE", N_( "Reunion" ) }, 				// :REU:638:
          {"RO", N_( "Romania" ) }, 				// :ROU:642:
          {"RS", N_( "Serbia" ) },
          {"RU", N_( "Russian Federation" ) }, 			// :RUS:643:
          {"RW", N_( "Rwanda" ) }, 				// :RWA:646:
          {"SA", N_( "Saudi Arabia" ) }, 			// :SAU:682:
          {"SB", N_( "Solomon Islands" ) }, 			// :SLB:090:
          {"SC", N_( "Seychelles" ) }, 				// :SYC:690:
          {"SD", N_( "Sudan" ) }, 				// :SDN:736:
          {"SE", N_( "Sweden" ) }, 				// :SWE:752:
          {"SG", N_( "Singapore" ) }, 				// :SGP:702:
          {"SH", N_( "Saint Helena" ) }, 			// :SHN:654:
          {"SI", N_( "Slovenia" ) }, 				// :SVN:705:
          {"SJ", N_( "Svalbard and Jan Mayen" ) }, 		// :SJM:744:
          {"SK", N_( "Slovakia" ) }, 				// :SVK:703:
          {"SL", N_( "Sierra Leone" ) }, 			// :SLE:694:
          {"SM", N_( "San Marino" ) }, 				// :SMR:674:
          {"SN", N_( "Senegal" ) }, 				// :SEN:686:
          {"SO", N_( "Somalia" ) }, 				// :SOM:706:
          {"SR", N_( "Suriname" ) }, 				// :SUR:740:
          {"ST", N_( "Sao Tome and Principe" ) }, 		// :STP:678:
          {"SV", N_( "El Salvador" ) }, 			// :SLV:222:
          {"SY", N_( "Syria" ) }, 				// :SYR:760:
          {"SZ", N_( "Swaziland" ) }, 				// :SWZ:748:
          {"TC", N_( "Turks and Caicos Islands" ) }, 		// :TCA:796:
          {"TD", N_( "Chad" ) }, 				// :TCD:148:
          {"TF", N_( "French Southern Territories" ) }, 	// :ATF:260:
          {"TG", N_( "Togo" ) }, 				// :TGO:768:
          {"TH", N_( "Thailand" ) }, 				// :THA:764:
          {"TJ", N_( "Tajikistan" ) }, 				// :TJK:762:
          {"TK", N_( "Tokelau" ) }, 				// :TKL:772:
          {"TM", N_( "Turkmenistan" ) }, 			// :TKM:795:
          {"TN", N_( "Tunisia" ) }, 				// :TUN:788:
          {"TO", N_( "Tonga" ) }, 				// :TON:776:
          {"TL", N_( "East Timor" ) }, 				// :TLS:626:
          {"TR", N_( "Turkey" ) }, 				// :TUR:792:
          {"TT", N_( "Trinidad and Tobago" ) }, 		// :TTO:780:
          {"TV", N_( "Tuvalu" ) }, 				// :TUV:798:
          {"TW", N_( "Taiwan" ) }, 				// :TWN:158:
          {"TZ", N_( "Tanzania" ) }, 				// :TZA:834:
          {"UA", N_( "Ukraine" ) }, 				// :UKR:804:
          {"UG", N_( "Uganda" ) }, 				// :UGA:800:
          {"UM", N_( "United States Minor Outlying Islands" ) },// :UMI:581:
          {"US", N_( "United States" ) }, 			// :USA:840:
          {"UY", N_( "Uruguay" ) }, 				// :URY:858:
          {"UZ", N_( "Uzbekistan" ) }, 				// :UZB:860:
          {"VA", N_( "Holy See (Vatican City State)" ) }, 	// :VAT:336:
          {"VC", N_( "Saint Vincent and the Grenadines" ) },	// :VCT:670:
          {"VE", N_( "Venezuela" ) }, 				// :VEN:862:
          {"VG", N_( "British Virgin Islands" ) }, 		// :VGB:092:
          {"VI", N_( "Virgin Islands, U.S." ) }, 		// :VIR:850:
          {"VN", N_( "Vietnam" ) }, 				// :VNM:704:
          {"VU", N_( "Vanuatu" ) }, 				// :VUT:548:
          {"WF", N_( "Wallis and Futuna" ) }, 			// :WLF:876:
          {"WS", N_( "Samoa" ) }, 				// :WSM:882:
          {"YE", N_( "Yemen" ) }, 				// :YEM:887:
          {"YT", N_( "Mayotte" ) }, 				// :MYT:175:
          {"ZA", N_( "South Africa" ) }, 			// :ZAF:710:
          {"ZM", N_( "Zambia" ) }, 				// :ZMB:894:
          {"ZW", N_( "Zimbabwe" ) }, 				// :ZWE:716:

	  { NULL, NULL }
      };

      for (const Init * i = init; i->iso3166 != NULL; ++i)
	  _codeMap[i->iso3166] = i->name;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
