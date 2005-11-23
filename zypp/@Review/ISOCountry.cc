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

  File:       ISOCountry.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Textdomain "iso-countries"

/-*/

// Using dgettext() to avoid interfering with a previously set textdomain
#define TEXTDOMAIN "iso-countries"
#define _(MSG)  dgettext(TEXTDOMAIN, (MSG))
#define __(MSG) MSG


#include <iostream>
#include <map>
#include <libintl.h>

#include <y2util/Y2SLog.h>
#include <y2util/stringutil.h>

#include <y2util/ISOCountry.h>

using namespace std;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ISOCountry::_D
/**
 *
 **/
struct ISOCountry::_D : public Rep {

  typedef map<string,string> CodeMap;

  static CodeMap _defCodeMap;
  static CodeMap _altCodeMap;

  static const string _noCode;
  static const string _noName;

  static CodeMap defaultCodeMap();

  typedef CodeMap::const_iterator Index;

  Index _index;

  Index _assert( const std::string & code_r ) {
    if ( _defCodeMap.empty() ) {
      _defCodeMap = defaultCodeMap();
    }
    CodeMap::iterator dit = _defCodeMap.find( code_r );
    if ( dit != _defCodeMap.end() ) {
      return dit;
    }
    CodeMap::iterator ait = _altCodeMap.find( code_r );
    if ( ait != _altCodeMap.end() ) {
      return ait;
    }

    // remember a new code
    CodeMap::value_type nval( code_r, string() );

    if ( code_r.size() != 2 ) {
      WAR << "Malformed ISOCountry code '" << code_r << "' (size != 2)" << endl;
    }

    string lcode( stringutil::toUpper( code_r ) );
    if ( lcode != code_r ) {
      WAR << "Malformed ISOCountry code '" << code_r << "' (not upper case)" << endl;
      // but maybe we're lucky with the upper case code
      // and find a country name.
      dit = _defCodeMap.find( lcode );
      if ( dit != _defCodeMap.end() ) {
	nval.second = dit->second;
      }
    }

    if ( ! nval.second.size() ) {
      nval.second = string( "Unknown country '" ) + code_r + "'";
    }

    MIL << "Remember country code '" << code_r << "': " << nval.second << endl;
    return _altCodeMap.insert( nval ).first;
  }

  _D( const std::string & code_r )
    : _index( _assert(code_r) )
  {}

  string code() const {
    return _index->first;
  }

  string name() const {
    return _( _index->second.c_str() );
  }
};

ISOCountry::_D::CodeMap ISOCountry::_D::_defCodeMap;
ISOCountry::_D::CodeMap ISOCountry::_D::_altCodeMap;

const string ISOCountry::_D::_noCode( "" );
const string ISOCountry::_D::_noName( "default" );

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOCountry::ISOCountry
//	METHOD TYPE : Constructor
//
ISOCountry::ISOCountry()
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOCountry::ISOCountry
//	METHOD TYPE : Constructor
//
ISOCountry::ISOCountry( const std::string & code_r )
{
  if ( code_r.size() ) {
    _d = makeVarPtr( new _D( code_r ) );
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOCountry::~ISOCountry
//	METHOD TYPE : Destructor
//
ISOCountry::~ISOCountry()
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOCountry::isSet
//	METHOD TYPE : bool
//
bool ISOCountry::isSet() const
{
  return _d;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOCountry::code
//	METHOD TYPE : std::string
//
std::string ISOCountry::code() const
{
  return _d ? _d->code() : _D::_noCode;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOCountry::name
//	METHOD TYPE : std::string
//
std::string ISOCountry::name() const
{
  return _d ? _d->name() : _D::_noName;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
*/
ostream & operator<<( ostream & str, const ISOCountry & obj )
{
  return str << obj.code();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : ISOCountry::_D::defaultCodeMap
//	METHOD TYPE : ISOCountry::_D::CodeMap
//
ISOCountry::_D::CodeMap ISOCountry:: _D::defaultCodeMap()
{
  bindtextdomain( TEXTDOMAIN, LOCALEDIR );
  bind_textdomain_codeset( TEXTDOMAIN, "UTF-8" );

  CodeMap cm;
  cm["AD"] = __( "Andorra" ); 				// :AND:020:
  cm["AE"] = __( "United Arab Emirates" ); 		// :ARE:784:
  cm["AF"] = __( "Afghanistan" ); 			// :AFG:004:
  cm["AG"] = __( "Antigua and Barbuda" ); 		// :ATG:028:
  cm["AI"] = __( "Anguilla" ); 				// :AIA:660:
  cm["AL"] = __( "Albania" ); 				// :ALB:008:
  cm["AM"] = __( "Armenia" ); 				// :ARM:051:
  cm["AN"] = __( "Netherlands Antilles" ); 		// :ANT:530:
  cm["AO"] = __( "Angola" ); 				// :AGO:024:
  cm["AQ"] = __( "Antarctica" ); 			// :ATA:010:
  cm["AR"] = __( "Argentina" ); 			// :ARG:032:
  cm["AS"] = __( "American Samoa" ); 			// :ASM:016:
  cm["AT"] = __( "Austria" ); 				// :AUT:040:
  cm["AU"] = __( "Australia" ); 			// :AUS:036:
  cm["AW"] = __( "Aruba" ); 				// :ABW:533:
  cm["AX"] = __( "Aland Islands" ); 			// :ALA:248:
  cm["AZ"] = __( "Azerbaijan" ); 			// :AZE:031:
  cm["BA"] = __( "Bosnia and Herzegovina" ); 		// :BIH:070:
  cm["BB"] = __( "Barbados" ); 				// :BRB:052:
  cm["BD"] = __( "Bangladesh" ); 			// :BGD:050:
  cm["BE"] = __( "Belgium" ); 				// :BEL:056:
  cm["BF"] = __( "Burkina Faso" ); 			// :BFA:854:
  cm["BG"] = __( "Bulgaria" ); 				// :BGR:100:
  cm["BH"] = __( "Bahrain" ); 				// :BHR:048:
  cm["BI"] = __( "Burundi" ); 				// :BDI:108:
  cm["BJ"] = __( "Benin" ); 				// :BEN:204:
  cm["BM"] = __( "Bermuda" ); 				// :BMU:060:
  cm["BN"] = __( "Brunei Darussalam" ); 		// :BRN:096:
  cm["BO"] = __( "Bolivia" ); 				// :BOL:068:
  cm["BR"] = __( "Brazil" ); 				// :BRA:076:
  cm["BS"] = __( "Bahamas" ); 				// :BHS:044:
  cm["BT"] = __( "Bhutan" ); 				// :BTN:064:
  cm["BV"] = __( "Bouvet Island" ); 			// :BVT:074:
  cm["BW"] = __( "Botswana" ); 				// :BWA:072:
  cm["BY"] = __( "Belarus" ); 				// :BLR:112:
  cm["BZ"] = __( "Belize" ); 				// :BLZ:084:
  cm["CA"] = __( "Canada" ); 				// :CAN:124:
  cm["CC"] = __( "Cocos (Keeling) Islands" ); 		// :CCK:166:
  cm["CD"] = __( "Congo" ); 				// :COD:180:
  cm["CF"] = __( "Centruual African Republic" ); 	// :CAF:140:
  cm["CG"] = __( "Congo" ); 				// :COG:178:
  cm["CH"] = __( "Switzerland" ); 			// :CHE:756:
  cm["CI"] = __( "Cote D'Ivoire" ); 			// :CIV:384:
  cm["CK"] = __( "Cook Islands" ); 			// :COK:184:
  cm["CL"] = __( "Chile" ); 				// :CHL:152:
  cm["CM"] = __( "Cameroon" ); 				// :CMR:120:
  cm["CN"] = __( "China" ); 				// :CHN:156:
  cm["CO"] = __( "Colombia" ); 				// :COL:170:
  cm["CR"] = __( "Costa Rica" ); 			// :CRI:188:
  cm["CS"] = __( "Serbia and Montenegro" ); 		// :SCG:891:
  cm["CU"] = __( "Cuba" ); 				// :CUB:192:
  cm["CV"] = __( "Cape Verde" ); 			// :CPV:132:
  cm["CX"] = __( "Christmas Island" ); 			// :CXR:162:
  cm["CY"] = __( "Cyprus" ); 				// :CYP:196:
  cm["CZ"] = __( "Czech Republic" ); 			// :CZE:203:
  cm["DE"] = __( "Germany" ); 				// :DEU:276:
  cm["DJ"] = __( "Djibouti" ); 				// :DJI:262:
  cm["DK"] = __( "Denmark" ); 				// :DNK:208:
  cm["DM"] = __( "Dominica" ); 				// :DMA:212:
  cm["DO"] = __( "Dominican Republic" ); 		// :DOM:214:
  cm["DZ"] = __( "Algeria" ); 				// :DZA:012:
  cm["EC"] = __( "Ecuador" ); 				// :ECU:218:
  cm["EE"] = __( "Estonia" ); 				// :EST:233:
  cm["EG"] = __( "Egypt" ); 				// :EGY:818:
  cm["EH"] = __( "Western Sahara" ); 			// :ESH:732:
  cm["ER"] = __( "Eritrea" ); 				// :ERI:232:
  cm["ES"] = __( "Spain" ); 				// :ESP:724:
  cm["ET"] = __( "Ethiopia" ); 				// :ETH:231:
  cm["FI"] = __( "Finland" ); 				// :FIN:246:
  cm["FJ"] = __( "Fiji" ); 				// :FJI:242:
  cm["FK"] = __( "Falkland Islands (Malvinas)" ); 	// :FLK:238:
  cm["FM"] = __( "Federated States of Micronesia" ); 	// :FSM:583:
  cm["FO"] = __( "Faroe Islands" ); 			// :FRO:234:
  cm["FR"] = __( "France" ); 				// :FRA:250:
  cm["FX"] = __( "Metropolitan France" ); 		// :FXX:249:
  cm["GA"] = __( "Gabon" ); 				// :GAB:266:
  cm["GB"] = __( "United Kingdom" ); 			// :GBR:826:
  cm["GD"] = __( "Grenada" ); 				// :GRD:308:
  cm["GE"] = __( "Georgia" ); 				// :GEO:268:
  cm["GF"] = __( "French Guiana" ); 			// :GUF:254:
  cm["GH"] = __( "Ghana" ); 				// :GHA:288:
  cm["GI"] = __( "Gibraltar" ); 			// :GIB:292:
  cm["GL"] = __( "Greenland" ); 			// :GRL:304:
  cm["GM"] = __( "Gambia" ); 				// :GMB:270:
  cm["GN"] = __( "Guinea" ); 				// :GIN:324:
  cm["GP"] = __( "Guadeloupe" ); 			// :GLP:312:
  cm["GQ"] = __( "Equatorial Guinea" ); 		// :GNQ:226:
  cm["GR"] = __( "Greece" ); 				// :GRC:300:
  cm["GS"] = __( "South Georgia and the South Sandwich Islands" );	// :SGS:239:
  cm["GT"] = __( "Guatemala" ); 			// :GTM:320:
  cm["GU"] = __( "Guam" ); 				// :GUM:316:
  cm["GW"] = __( "Guinea-Bissau" ); 			// :GNB:624:
  cm["GY"] = __( "Guyana" ); 				// :GUY:328:
  cm["HK"] = __( "Hong Kong" ); 			// :HKG:344:
  cm["HM"] = __( "Heard Island and McDonald Islands" ); // :HMD:334:
  cm["HN"] = __( "Honduras" ); 				// :HND:340:
  cm["HR"] = __( "Croatia" ); 				// :HRV:191:
  cm["HT"] = __( "Haiti" ); 				// :HTI:332:
  cm["HU"] = __( "Hungary" ); 				// :HUN:348:
  cm["ID"] = __( "Indonesia" ); 			// :IDN:360:
  cm["IE"] = __( "Ireland" ); 				// :IRL:372:
  cm["IL"] = __( "Israel" ); 				// :ISR:376:
  cm["IN"] = __( "India" ); 				// :IND:356:
  cm["IO"] = __( "British Indian Ocean Territory" ); 	// :IOT:086:
  cm["IQ"] = __( "Iraq" ); 				// :IRQ:368:
  cm["IR"] = __( "Iran" ); 				// :IRN:364:
  cm["IS"] = __( "Iceland" ); 				// :ISL:352:
  cm["IT"] = __( "Italy" ); 				// :ITA:380:
  cm["JM"] = __( "Jamaica" ); 				// :JAM:388:
  cm["JO"] = __( "Jordan" ); 				// :JOR:400:
  cm["JP"] = __( "Japan" ); 				// :JPN:392:
  cm["KE"] = __( "Kenya" ); 				// :KEN:404:
  cm["KG"] = __( "Kyrgyzstan" ); 			// :KGZ:417:
  cm["KH"] = __( "Cambodia" ); 				// :KHM:116:
  cm["KI"] = __( "Kiribati" ); 				// :KIR:296:
  cm["KM"] = __( "Comoros" ); 				// :COM:174:
  cm["KN"] = __( "Saint Kitts and Nevis" ); 		// :KNA:659:
  cm["KP"] = __( "North Korea" ); 			// :PRK:408:
  cm["KR"] = __( "South Korea" ); 			// :KOR:410:
  cm["KW"] = __( "Kuwait" ); 				// :KWT:414:
  cm["KY"] = __( "Cayman Islands" ); 			// :CYM:136:
  cm["KZ"] = __( "Kazakhstan" ); 			// :KAZ:398:
  cm["LA"] = __( "Lao People's Democratic Republic" ); 	// :LAO:418:
  cm["LB"] = __( "Lebanon" ); 				// :LBN:422:
  cm["LC"] = __( "Saint Lucia" ); 			// :LCA:662:
  cm["LI"] = __( "Liechtenstein" ); 			// :LIE:438:
  cm["LK"] = __( "Sri Lanka" ); 			// :LKA:144:
  cm["LR"] = __( "Liberia" ); 				// :LBR:430:
  cm["LS"] = __( "Lesotho" ); 				// :LSO:426:
  cm["LT"] = __( "Lithuania" ); 			// :LTU:440:
  cm["LU"] = __( "Luxembourg" ); 			// :LUX:442:
  cm["LV"] = __( "Latvia" ); 				// :LVA:428:
  cm["LY"] = __( "Libya" ); 				// :LBY:434:
  cm["MA"] = __( "Morocco" ); 				// :MAR:504:
  cm["MC"] = __( "Monaco" ); 				// :MCO:492:
  cm["MD"] = __( "Moldova" ); 				// :MDA:498:
  cm["MG"] = __( "Madagascar" ); 			// :MDG:450:
  cm["MH"] = __( "Marshall Islands" ); 			// :MHL:584:
  cm["MK"] = __( "Macedonia" ); 			// :MKD:807:
  cm["ML"] = __( "Mali" ); 				// :MLI:466:
  cm["MM"] = __( "Myanmar" ); 				// :MMR:104:
  cm["MN"] = __( "Mongolia" ); 				// :MNG:496:
  cm["MO"] = __( "Macao" ); 				// :MAC:446:
  cm["MP"] = __( "Northern Mariana Islands" ); 		// :MNP:580:
  cm["MQ"] = __( "Martinique" ); 			// :MTQ:474:
  cm["MR"] = __( "Mauritania" ); 			// :MRT:478:
  cm["MS"] = __( "Montserrat" ); 			// :MSR:500:
  cm["MT"] = __( "Malta" ); 				// :MLT:470:
  cm["MU"] = __( "Mauritius" ); 			// :MUS:480:
  cm["MV"] = __( "Maldives" ); 				// :MDV:462:
  cm["MW"] = __( "Malawi" ); 				// :MWI:454:
  cm["MX"] = __( "Mexico" ); 				// :MEX:484:
  cm["MY"] = __( "Malaysia" ); 				// :MYS:458:
  cm["MZ"] = __( "Mozambique" ); 			// :MOZ:508:
  cm["NA"] = __( "Namibia" ); 				// :NAM:516:
  cm["NC"] = __( "New Caledonia" ); 			// :NCL:540:
  cm["NE"] = __( "Niger" ); 				// :NER:562:
  cm["NF"] = __( "Norfolk Island" ); 			// :NFK:574:
  cm["NG"] = __( "Nigeria" ); 				// :NGA:566:
  cm["NI"] = __( "Nicaragua" ); 			// :NIC:558:
  cm["NL"] = __( "Netherlands" ); 			// :NLD:528:
  cm["NO"] = __( "Norway" ); 				// :NOR:578:
  cm["NP"] = __( "Nepal" ); 				// :NPL:524:
  cm["NR"] = __( "Nauru" ); 				// :NRU:520:
  cm["NU"] = __( "Niue" ); 				// :NIU:570:
  cm["NZ"] = __( "New Zealand" ); 			// :NZL:554:
  cm["OM"] = __( "Oman" ); 				// :OMN:512:
  cm["PA"] = __( "Panama" ); 				// :PAN:591:
  cm["PE"] = __( "Peru" ); 				// :PER:604:
  cm["PF"] = __( "French Polynesia" ); 			// :PYF:258:
  cm["PG"] = __( "Papua New Guinea" ); 			// :PNG:598:
  cm["PH"] = __( "Philippines" ); 			// :PHL:608:
  cm["PK"] = __( "Pakistan" ); 				// :PAK:586:
  cm["PL"] = __( "Poland" ); 				// :POL:616:
  cm["PM"] = __( "Saint Pierre and Miquelon" ); 	// :SPM:666:
  cm["PN"] = __( "Pitcairn" ); 				// :PCN:612:
  cm["PR"] = __( "Puerto Rico" ); 			// :PRI:630:
  cm["PS"] = __( "Palestinian Territory" ); 		// :PSE:275:
  cm["PT"] = __( "Portugal" ); 				// :PRT:620:
  cm["PW"] = __( "Palau" ); 				// :PLW:585:
  cm["PY"] = __( "Paraguay" ); 				// :PRY:600:
  cm["QA"] = __( "Qatar" ); 				// :QAT:634:
  cm["RE"] = __( "Reunion" ); 				// :REU:638:
  cm["RO"] = __( "Romania" ); 				// :ROU:642:
  cm["RU"] = __( "Russian Federation" ); 		// :RUS:643:
  cm["RW"] = __( "Rwanda" ); 				// :RWA:646:
  cm["SA"] = __( "Saudi Arabia" ); 			// :SAU:682:
  cm["SB"] = __( "Solomon Islands" ); 			// :SLB:090:
  cm["SC"] = __( "Seychelles" ); 			// :SYC:690:
  cm["SD"] = __( "Sudan" ); 				// :SDN:736:
  cm["SE"] = __( "Sweden" ); 				// :SWE:752:
  cm["SG"] = __( "Singapore" ); 			// :SGP:702:
  cm["SH"] = __( "Saint Helena" ); 			// :SHN:654:
  cm["SI"] = __( "Slovenia" ); 				// :SVN:705:
  cm["SJ"] = __( "Svalbard and Jan Mayen" ); 		// :SJM:744:
  cm["SK"] = __( "Slovakia" ); 				// :SVK:703:
  cm["SL"] = __( "Sierra Leone" ); 			// :SLE:694:
  cm["SM"] = __( "San Marino" ); 			// :SMR:674:
  cm["SN"] = __( "Senegal" ); 				// :SEN:686:
  cm["SO"] = __( "Somalia" ); 				// :SOM:706:
  cm["SR"] = __( "Suriname" ); 				// :SUR:740:
  cm["ST"] = __( "Sao Tome and Principe" ); 		// :STP:678:
  cm["SV"] = __( "El Salvador" ); 			// :SLV:222:
  cm["SY"] = __( "Syria" ); 				// :SYR:760:
  cm["SZ"] = __( "Swaziland" ); 			// :SWZ:748:
  cm["TC"] = __( "Turks and Caicos Islands" ); 		// :TCA:796:
  cm["TD"] = __( "Chad" ); 				// :TCD:148:
  cm["TF"] = __( "French Southern Territories" ); 	// :ATF:260:
  cm["TG"] = __( "Togo" ); 				// :TGO:768:
  cm["TH"] = __( "Thailand" ); 				// :THA:764:
  cm["TJ"] = __( "Tajikistan" ); 			// :TJK:762:
  cm["TK"] = __( "Tokelau" ); 				// :TKL:772:
  cm["TM"] = __( "Turkmenistan" ); 			// :TKM:795:
  cm["TN"] = __( "Tunisia" ); 				// :TUN:788:
  cm["TO"] = __( "Tonga" ); 				// :TON:776:
  cm["TL"] = __( "East Timor" ); 			// :TLS:626:
  cm["TR"] = __( "Turkey" ); 				// :TUR:792:
  cm["TT"] = __( "Trinidad and Tobago" ); 		// :TTO:780:
  cm["TV"] = __( "Tuvalu" ); 				// :TUV:798:
  cm["TW"] = __( "Taiwan" ); 				// :TWN:158:
  cm["TZ"] = __( "Tanzania" ); 				// :TZA:834:
  cm["UA"] = __( "Ukraine" ); 				// :UKR:804:
  cm["UG"] = __( "Uganda" ); 				// :UGA:800:
  cm["UM"] = __( "United States Minor Outlying Islands" );	// :UMI:581:
  cm["US"] = __( "United States" ); 			// :USA:840:
  cm["UY"] = __( "Uruguay" ); 				// :URY:858:
  cm["UZ"] = __( "Uzbekistan" ); 			// :UZB:860:
  cm["VA"] = __( "Holy See (Vatican City State)" ); 	// :VAT:336:
  cm["VC"] = __( "Saint Vincent and the Grenadines" ); 	// :VCT:670:
  cm["VE"] = __( "Venezuela" ); 			// :VEN:862:
  cm["VG"] = __( "British Virgin Islands" ); 		// :VGB:092:
  cm["VI"] = __( "Virgin Islands, U.S." ); 		// :VIR:850:
  cm["VN"] = __( "Vietnam" ); 				// :VNM:704:
  cm["VU"] = __( "Vanuatu" ); 				// :VUT:548:
  cm["WF"] = __( "Wallis and Futuna" ); 		// :WLF:876:
  cm["WS"] = __( "Samoa" ); 				// :WSM:882:
  cm["YE"] = __( "Yemen" ); 				// :YEM:887:
  cm["YT"] = __( "Mayotte" ); 				// :MYT:175:
  cm["ZA"] = __( "South Africa" ); 			// :ZAF:710:
  cm["ZM"] = __( "Zambia" ); 				// :ZMB:894:
  cm["ZW"] = __( "Zimbabwe" ); 				// :ZWE:716:
  return cm;
}
