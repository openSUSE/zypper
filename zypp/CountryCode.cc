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
#include <map>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include "zypp/CountryCode.h"

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
      typedef std::map<std::string,std::string> CodeMap;
      typedef CodeMap::const_iterator Index;

      /** Return the CodeMap Index for \a code_r. */
      static Index getIndex( const std::string & code_r )
      {
        static CodeMaps _maps; // the singleton instance
        return _maps.lookup( code_r );
      }

    private:
      /** Ctor initializes the code maps.
       * http://www.iso.org/iso/en/prods-services/iso3166ma/02iso-3166-code-lists/list-en1.html
      */
      CodeMaps();

      /** Make shure the code is in the code maps and return it's index. */
      inline Index lookup( const std::string & code_r );

      /** Return index of \a code_r, if it's in the code maps. */
      inline Index lookupCode( const std::string & code_r );

    private:
      /** Two letter codes. */
      CodeMap iso3166;
      /** All the stuff the application injects. */
      CodeMap others;
    };

    inline CodeMaps::Index CodeMaps::lookupCode( const std::string & code_r )
    {
      switch ( code_r.size() )
        {
        case 2:
          {
            Index it = iso3166.find( code_r );
            if ( it != iso3166.end() )
              return it;
          }
          break;
        }
      // not found: check others
      // !!! not found at all returns others.end()
      return others.find( code_r );
    }

    inline CodeMaps::Index CodeMaps::lookup( const std::string & code_r )
    {
      Index it = lookupCode( code_r );
      if ( it != others.end() )
        return it;

      // not found: Remember a new code
      CodeMap::value_type nval( code_r, std::string() );

      if ( code_r.size() != 2 )
        WAR << "Malformed CountryCode '" << code_r << "' (expect 2-letter)" << endl;

      std::string lcode( str::toUpper( code_r ) );
      if ( lcode != code_r )
        {
          WAR << "Malformed CountryCode '" << code_r << "' (not upper case)" << endl;
          // but maybe we're lucky with the upper case code
          // and find a country name.
          it = lookupCode( lcode );
          if ( it != others.end() )
            nval.second = it->second;
        }

      MIL << "Remember CountryCode '" << code_r << "': '" << nval.second << "'" << endl;
      return others.insert( nval ).first;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : CountryCode::Impl
  //
  /** CountryCode implementation.
   * \note CodeMaps contain the untranslated country names.
   * Translation is done in \ref name.
  */
  struct CountryCode::Impl
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
          std::string ret( _("Unknown country: ") );
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
  //	CLASS NAME : CountryCode
  //
  ///////////////////////////////////////////////////////////////////

  const CountryCode CountryCode::noCode;

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CountryCode::CountryCode
  //	METHOD TYPE : Ctor
  //
  CountryCode::CountryCode()
  : _pimpl( Impl::nullimpl() )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CountryCode::CountryCode
  //	METHOD TYPE : Ctor
  //
  CountryCode::CountryCode( const std::string & code_r )
  : _pimpl( new Impl( code_r ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CountryCode::~CountryCode
  //	METHOD TYPE : Dtor
  //
  CountryCode::~CountryCode()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CountryCode::code
  //	METHOD TYPE : std::string
  //
  std::string CountryCode::code() const
  { return _pimpl->code(); }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : CountryCode::name
  //	METHOD TYPE : std::string
  //
  std::string CountryCode::name() const
  { return _pimpl->name(); }

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    CodeMaps::CodeMaps()
    {
      // Defined CountryCode constants
      others[""]        = N_( "noCode" );

      iso3166["AD"] = N_( "Andorra" ); 				// :AND:020:
      iso3166["AE"] = N_( "United Arab Emirates" ); 		// :ARE:784:
      iso3166["AF"] = N_( "Afghanistan" ); 			// :AFG:004:
      iso3166["AG"] = N_( "Antigua and Barbuda" ); 		// :ATG:028:
      iso3166["AI"] = N_( "Anguilla" ); 			// :AIA:660:
      iso3166["AL"] = N_( "Albania" ); 				// :ALB:008:
      iso3166["AM"] = N_( "Armenia" ); 				// :ARM:051:
      iso3166["AN"] = N_( "Netherlands Antilles" ); 		// :ANT:530:
      iso3166["AO"] = N_( "Angola" ); 				// :AGO:024:
      iso3166["AQ"] = N_( "Antarctica" ); 			// :ATA:010:
      iso3166["AR"] = N_( "Argentina" ); 			// :ARG:032:
      iso3166["AS"] = N_( "American Samoa" ); 			// :ASM:016:
      iso3166["AT"] = N_( "Austria" ); 				// :AUT:040:
      iso3166["AU"] = N_( "Australia" ); 			// :AUS:036:
      iso3166["AW"] = N_( "Aruba" ); 				// :ABW:533:
      iso3166["AX"] = N_( "Aland Islands" ); 			// :ALA:248:
      iso3166["AZ"] = N_( "Azerbaijan" ); 			// :AZE:031:
      iso3166["BA"] = N_( "Bosnia and Herzegovina" ); 		// :BIH:070:
      iso3166["BB"] = N_( "Barbados" ); 			// :BRB:052:
      iso3166["BD"] = N_( "Bangladesh" ); 			// :BGD:050:
      iso3166["BE"] = N_( "Belgium" ); 				// :BEL:056:
      iso3166["BF"] = N_( "Burkina Faso" ); 			// :BFA:854:
      iso3166["BG"] = N_( "Bulgaria" ); 			// :BGR:100:
      iso3166["BH"] = N_( "Bahrain" ); 				// :BHR:048:
      iso3166["BI"] = N_( "Burundi" ); 				// :BDI:108:
      iso3166["BJ"] = N_( "Benin" ); 				// :BEN:204:
      iso3166["BM"] = N_( "Bermuda" ); 				// :BMU:060:
      iso3166["BN"] = N_( "Brunei Darussalam" ); 		// :BRN:096:
      iso3166["BO"] = N_( "Bolivia" ); 				// :BOL:068:
      iso3166["BR"] = N_( "Brazil" ); 				// :BRA:076:
      iso3166["BS"] = N_( "Bahamas" ); 				// :BHS:044:
      iso3166["BT"] = N_( "Bhutan" ); 				// :BTN:064:
      iso3166["BV"] = N_( "Bouvet Island" ); 			// :BVT:074:
      iso3166["BW"] = N_( "Botswana" ); 			// :BWA:072:
      iso3166["BY"] = N_( "Belarus" ); 				// :BLR:112:
      iso3166["BZ"] = N_( "Belize" ); 				// :BLZ:084:
      iso3166["CA"] = N_( "Canada" ); 				// :CAN:124:
      iso3166["CC"] = N_( "Cocos (Keeling) Islands" ); 		// :CCK:166:
      iso3166["CD"] = N_( "Congo" ); 				// :COD:180:
      iso3166["CF"] = N_( "Centruual African Republic" ); 	// :CAF:140:
      iso3166["CG"] = N_( "Congo" ); 				// :COG:178:
      iso3166["CH"] = N_( "Switzerland" ); 			// :CHE:756:
      iso3166["CI"] = N_( "Cote D'Ivoire" ); 			// :CIV:384:
      iso3166["CK"] = N_( "Cook Islands" ); 			// :COK:184:
      iso3166["CL"] = N_( "Chile" ); 				// :CHL:152:
      iso3166["CM"] = N_( "Cameroon" ); 			// :CMR:120:
      iso3166["CN"] = N_( "China" ); 				// :CHN:156:
      iso3166["CO"] = N_( "Colombia" ); 			// :COL:170:
      iso3166["CR"] = N_( "Costa Rica" ); 			// :CRI:188:
      iso3166["CS"] = N_( "Serbia and Montenegro" ); 		// :SCG:891:
      iso3166["CU"] = N_( "Cuba" ); 				// :CUB:192:
      iso3166["CV"] = N_( "Cape Verde" ); 			// :CPV:132:
      iso3166["CX"] = N_( "Christmas Island" ); 		// :CXR:162:
      iso3166["CY"] = N_( "Cyprus" ); 				// :CYP:196:
      iso3166["CZ"] = N_( "Czech Republic" ); 			// :CZE:203:
      iso3166["DE"] = N_( "Germany" ); 				// :DEU:276:
      iso3166["DJ"] = N_( "Djibouti" ); 			// :DJI:262:
      iso3166["DK"] = N_( "Denmark" ); 				// :DNK:208:
      iso3166["DM"] = N_( "Dominica" ); 			// :DMA:212:
      iso3166["DO"] = N_( "Dominican Republic" ); 		// :DOM:214:
      iso3166["DZ"] = N_( "Algeria" ); 				// :DZA:012:
      iso3166["EC"] = N_( "Ecuador" ); 				// :ECU:218:
      iso3166["EE"] = N_( "Estonia" ); 				// :EST:233:
      iso3166["EG"] = N_( "Egypt" ); 				// :EGY:818:
      iso3166["EH"] = N_( "Western Sahara" ); 			// :ESH:732:
      iso3166["ER"] = N_( "Eritrea" ); 				// :ERI:232:
      iso3166["ES"] = N_( "Spain" ); 				// :ESP:724:
      iso3166["ET"] = N_( "Ethiopia" ); 			// :ETH:231:
      iso3166["FI"] = N_( "Finland" ); 				// :FIN:246:
      iso3166["FJ"] = N_( "Fiji" ); 				// :FJI:242:
      iso3166["FK"] = N_( "Falkland Islands (Malvinas)" ); 	// :FLK:238:
      iso3166["FM"] = N_( "Federated States of Micronesia" ); 	// :FSM:583:
      iso3166["FO"] = N_( "Faroe Islands" ); 			// :FRO:234:
      iso3166["FR"] = N_( "France" ); 				// :FRA:250:
      iso3166["FX"] = N_( "Metropolitan France" ); 		// :FXX:249:
      iso3166["GA"] = N_( "Gabon" ); 				// :GAB:266:
      iso3166["GB"] = N_( "United Kingdom" ); 			// :GBR:826:
      iso3166["GD"] = N_( "Grenada" ); 				// :GRD:308:
      iso3166["GE"] = N_( "Georgia" ); 				// :GEO:268:
      iso3166["GF"] = N_( "French Guiana" ); 			// :GUF:254:
      iso3166["GH"] = N_( "Ghana" ); 				// :GHA:288:
      iso3166["GI"] = N_( "Gibraltar" ); 			// :GIB:292:
      iso3166["GL"] = N_( "Greenland" ); 			// :GRL:304:
      iso3166["GM"] = N_( "Gambia" ); 				// :GMB:270:
      iso3166["GN"] = N_( "Guinea" ); 				// :GIN:324:
      iso3166["GP"] = N_( "Guadeloupe" ); 			// :GLP:312:
      iso3166["GQ"] = N_( "Equatorial Guinea" ); 		// :GNQ:226:
      iso3166["GR"] = N_( "Greece" ); 				// :GRC:300:
      iso3166["GS"] = N_( "South Georgia and the South Sandwich Islands" );	// :SGS:239:
      iso3166["GT"] = N_( "Guatemala" ); 			// :GTM:320:
      iso3166["GU"] = N_( "Guam" ); 				// :GUM:316:
      iso3166["GW"] = N_( "Guinea-Bissau" ); 			// :GNB:624:
      iso3166["GY"] = N_( "Guyana" ); 				// :GUY:328:
      iso3166["HK"] = N_( "Hong Kong" ); 			// :HKG:344:
      iso3166["HM"] = N_( "Heard Island and McDonald Islands" ); // :HMD:334:
      iso3166["HN"] = N_( "Honduras" ); 			// :HND:340:
      iso3166["HR"] = N_( "Croatia" ); 				// :HRV:191:
      iso3166["HT"] = N_( "Haiti" ); 				// :HTI:332:
      iso3166["HU"] = N_( "Hungary" ); 				// :HUN:348:
      iso3166["ID"] = N_( "Indonesia" ); 			// :IDN:360:
      iso3166["IE"] = N_( "Ireland" ); 				// :IRL:372:
      iso3166["IL"] = N_( "Israel" ); 				// :ISR:376:
      iso3166["IN"] = N_( "India" ); 				// :IND:356:
      iso3166["IO"] = N_( "British Indian Ocean Territory" ); 	// :IOT:086:
      iso3166["IQ"] = N_( "Iraq" ); 				// :IRQ:368:
      iso3166["IR"] = N_( "Iran" ); 				// :IRN:364:
      iso3166["IS"] = N_( "Iceland" ); 				// :ISL:352:
      iso3166["IT"] = N_( "Italy" ); 				// :ITA:380:
      iso3166["JM"] = N_( "Jamaica" ); 				// :JAM:388:
      iso3166["JO"] = N_( "Jordan" ); 				// :JOR:400:
      iso3166["JP"] = N_( "Japan" ); 				// :JPN:392:
      iso3166["KE"] = N_( "Kenya" ); 				// :KEN:404:
      iso3166["KG"] = N_( "Kyrgyzstan" ); 			// :KGZ:417:
      iso3166["KH"] = N_( "Cambodia" ); 			// :KHM:116:
      iso3166["KI"] = N_( "Kiribati" ); 			// :KIR:296:
      iso3166["KM"] = N_( "Comoros" ); 				// :COM:174:
      iso3166["KN"] = N_( "Saint Kitts and Nevis" ); 		// :KNA:659:
      iso3166["KP"] = N_( "North Korea" ); 			// :PRK:408:
      iso3166["KR"] = N_( "South Korea" ); 			// :KOR:410:
      iso3166["KW"] = N_( "Kuwait" ); 				// :KWT:414:
      iso3166["KY"] = N_( "Cayman Islands" ); 			// :CYM:136:
      iso3166["KZ"] = N_( "Kazakhstan" ); 			// :KAZ:398:
      iso3166["LA"] = N_( "Lao People's Democratic Republic" );	// :LAO:418:
      iso3166["LB"] = N_( "Lebanon" ); 				// :LBN:422:
      iso3166["LC"] = N_( "Saint Lucia" ); 			// :LCA:662:
      iso3166["LI"] = N_( "Liechtenstein" ); 			// :LIE:438:
      iso3166["LK"] = N_( "Sri Lanka" ); 			// :LKA:144:
      iso3166["LR"] = N_( "Liberia" ); 				// :LBR:430:
      iso3166["LS"] = N_( "Lesotho" ); 				// :LSO:426:
      iso3166["LT"] = N_( "Lithuania" ); 			// :LTU:440:
      iso3166["LU"] = N_( "Luxembourg" ); 			// :LUX:442:
      iso3166["LV"] = N_( "Latvia" ); 				// :LVA:428:
      iso3166["LY"] = N_( "Libya" ); 				// :LBY:434:
      iso3166["MA"] = N_( "Morocco" ); 				// :MAR:504:
      iso3166["MC"] = N_( "Monaco" ); 				// :MCO:492:
      iso3166["MD"] = N_( "Moldova" ); 				// :MDA:498:
      iso3166["MG"] = N_( "Madagascar" ); 			// :MDG:450:
      iso3166["MH"] = N_( "Marshall Islands" ); 		// :MHL:584:
      iso3166["MK"] = N_( "Macedonia" ); 			// :MKD:807:
      iso3166["ML"] = N_( "Mali" ); 				// :MLI:466:
      iso3166["MM"] = N_( "Myanmar" ); 				// :MMR:104:
      iso3166["MN"] = N_( "Mongolia" ); 			// :MNG:496:
      iso3166["MO"] = N_( "Macao" ); 				// :MAC:446:
      iso3166["MP"] = N_( "Northern Mariana Islands" ); 	// :MNP:580:
      iso3166["MQ"] = N_( "Martinique" ); 			// :MTQ:474:
      iso3166["MR"] = N_( "Mauritania" ); 			// :MRT:478:
      iso3166["MS"] = N_( "Montserrat" ); 			// :MSR:500:
      iso3166["MT"] = N_( "Malta" ); 				// :MLT:470:
      iso3166["MU"] = N_( "Mauritius" ); 			// :MUS:480:
      iso3166["MV"] = N_( "Maldives" ); 			// :MDV:462:
      iso3166["MW"] = N_( "Malawi" ); 				// :MWI:454:
      iso3166["MX"] = N_( "Mexico" ); 				// :MEX:484:
      iso3166["MY"] = N_( "Malaysia" ); 			// :MYS:458:
      iso3166["MZ"] = N_( "Mozambique" ); 			// :MOZ:508:
      iso3166["NA"] = N_( "Namibia" ); 				// :NAM:516:
      iso3166["NC"] = N_( "New Caledonia" ); 			// :NCL:540:
      iso3166["NE"] = N_( "Niger" ); 				// :NER:562:
      iso3166["NF"] = N_( "Norfolk Island" ); 			// :NFK:574:
      iso3166["NG"] = N_( "Nigeria" ); 				// :NGA:566:
      iso3166["NI"] = N_( "Nicaragua" ); 			// :NIC:558:
      iso3166["NL"] = N_( "Netherlands" ); 			// :NLD:528:
      iso3166["NO"] = N_( "Norway" ); 				// :NOR:578:
      iso3166["NP"] = N_( "Nepal" ); 				// :NPL:524:
      iso3166["NR"] = N_( "Nauru" ); 				// :NRU:520:
      iso3166["NU"] = N_( "Niue" ); 				// :NIU:570:
      iso3166["NZ"] = N_( "New Zealand" ); 			// :NZL:554:
      iso3166["OM"] = N_( "Oman" ); 				// :OMN:512:
      iso3166["PA"] = N_( "Panama" ); 				// :PAN:591:
      iso3166["PE"] = N_( "Peru" ); 				// :PER:604:
      iso3166["PF"] = N_( "French Polynesia" ); 		// :PYF:258:
      iso3166["PG"] = N_( "Papua New Guinea" ); 		// :PNG:598:
      iso3166["PH"] = N_( "Philippines" ); 			// :PHL:608:
      iso3166["PK"] = N_( "Pakistan" ); 			// :PAK:586:
      iso3166["PL"] = N_( "Poland" ); 				// :POL:616:
      iso3166["PM"] = N_( "Saint Pierre and Miquelon" ); 	// :SPM:666:
      iso3166["PN"] = N_( "Pitcairn" ); 			// :PCN:612:
      iso3166["PR"] = N_( "Puerto Rico" ); 			// :PRI:630:
      iso3166["PS"] = N_( "Palestinian Territory" ); 		// :PSE:275:
      iso3166["PT"] = N_( "Portugal" ); 			// :PRT:620:
      iso3166["PW"] = N_( "Palau" ); 				// :PLW:585:
      iso3166["PY"] = N_( "Paraguay" ); 			// :PRY:600:
      iso3166["QA"] = N_( "Qatar" ); 				// :QAT:634:
      iso3166["RE"] = N_( "Reunion" ); 				// :REU:638:
      iso3166["RO"] = N_( "Romania" ); 				// :ROU:642:
      iso3166["RU"] = N_( "Russian Federation" ); 		// :RUS:643:
      iso3166["RW"] = N_( "Rwanda" ); 				// :RWA:646:
      iso3166["SA"] = N_( "Saudi Arabia" ); 			// :SAU:682:
      iso3166["SB"] = N_( "Solomon Islands" ); 			// :SLB:090:
      iso3166["SC"] = N_( "Seychelles" ); 			// :SYC:690:
      iso3166["SD"] = N_( "Sudan" ); 				// :SDN:736:
      iso3166["SE"] = N_( "Sweden" ); 				// :SWE:752:
      iso3166["SG"] = N_( "Singapore" ); 			// :SGP:702:
      iso3166["SH"] = N_( "Saint Helena" ); 			// :SHN:654:
      iso3166["SI"] = N_( "Slovenia" ); 			// :SVN:705:
      iso3166["SJ"] = N_( "Svalbard and Jan Mayen" ); 		// :SJM:744:
      iso3166["SK"] = N_( "Slovakia" ); 			// :SVK:703:
      iso3166["SL"] = N_( "Sierra Leone" ); 			// :SLE:694:
      iso3166["SM"] = N_( "San Marino" ); 			// :SMR:674:
      iso3166["SN"] = N_( "Senegal" ); 				// :SEN:686:
      iso3166["SO"] = N_( "Somalia" ); 				// :SOM:706:
      iso3166["SR"] = N_( "Suriname" ); 			// :SUR:740:
      iso3166["ST"] = N_( "Sao Tome and Principe" ); 		// :STP:678:
      iso3166["SV"] = N_( "El Salvador" ); 			// :SLV:222:
      iso3166["SY"] = N_( "Syria" ); 				// :SYR:760:
      iso3166["SZ"] = N_( "Swaziland" ); 			// :SWZ:748:
      iso3166["TC"] = N_( "Turks and Caicos Islands" ); 	// :TCA:796:
      iso3166["TD"] = N_( "Chad" ); 				// :TCD:148:
      iso3166["TF"] = N_( "French Southern Territories" ); 	// :ATF:260:
      iso3166["TG"] = N_( "Togo" ); 				// :TGO:768:
      iso3166["TH"] = N_( "Thailand" ); 			// :THA:764:
      iso3166["TJ"] = N_( "Tajikistan" ); 			// :TJK:762:
      iso3166["TK"] = N_( "Tokelau" ); 				// :TKL:772:
      iso3166["TM"] = N_( "Turkmenistan" ); 			// :TKM:795:
      iso3166["TN"] = N_( "Tunisia" ); 				// :TUN:788:
      iso3166["TO"] = N_( "Tonga" ); 				// :TON:776:
      iso3166["TL"] = N_( "East Timor" ); 			// :TLS:626:
      iso3166["TR"] = N_( "Turkey" ); 				// :TUR:792:
      iso3166["TT"] = N_( "Trinidad and Tobago" ); 		// :TTO:780:
      iso3166["TV"] = N_( "Tuvalu" ); 				// :TUV:798:
      iso3166["TW"] = N_( "Taiwan" ); 				// :TWN:158:
      iso3166["TZ"] = N_( "Tanzania" ); 			// :TZA:834:
      iso3166["UA"] = N_( "Ukraine" ); 				// :UKR:804:
      iso3166["UG"] = N_( "Uganda" ); 				// :UGA:800:
      iso3166["UM"] = N_( "United States Minor Outlying Islands" );	// :UMI:581:
      iso3166["US"] = N_( "United States" ); 			// :USA:840:
      iso3166["UY"] = N_( "Uruguay" ); 				// :URY:858:
      iso3166["UZ"] = N_( "Uzbekistan" ); 			// :UZB:860:
      iso3166["VA"] = N_( "Holy See (Vatican City State)" ); 	// :VAT:336:
      iso3166["VC"] = N_( "Saint Vincent and the Grenadines" ); // :VCT:670:
      iso3166["VE"] = N_( "Venezuela" ); 			// :VEN:862:
      iso3166["VG"] = N_( "British Virgin Islands" ); 		// :VGB:092:
      iso3166["VI"] = N_( "Virgin Islands, U.S." ); 		// :VIR:850:
      iso3166["VN"] = N_( "Vietnam" ); 				// :VNM:704:
      iso3166["VU"] = N_( "Vanuatu" ); 				// :VUT:548:
      iso3166["WF"] = N_( "Wallis and Futuna" ); 		// :WLF:876:
      iso3166["WS"] = N_( "Samoa" ); 				// :WSM:882:
      iso3166["YE"] = N_( "Yemen" ); 				// :YEM:887:
      iso3166["YT"] = N_( "Mayotte" ); 				// :MYT:175:
      iso3166["ZA"] = N_( "South Africa" ); 			// :ZAF:710:
      iso3166["ZM"] = N_( "Zambia" ); 				// :ZMB:894:
      iso3166["ZW"] = N_( "Zimbabwe" ); 			// :ZWE:716:
    }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
