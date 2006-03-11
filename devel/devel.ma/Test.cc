#include <iostream>

#include "zypp/base/LogControl.h"

#include "zypp/Locale.h"


using std::endl;
using namespace zypp;

inline std::string vstr( const char * str )
{
  std::string ret;
  if ( str )
    {
      ret+="\"";
      ret+=str;
      ret+="\"";
    }
  return ret;
}

void test( const Locale & val_r )
{
  DBG << " <<:        " << (val_r) << endl;
  DBG << " code:      " << (val_r.code()) << endl;
  DBG << " name:      " << (val_r.name()) << endl;
  DBG << " ==noCode:  " << (val_r == Locale::noCode) << endl;
  DBG << " !=noCode:  " << (val_r != Locale::noCode) << endl;
  DBG << " Language:  " << (val_r.language()) << endl;
  DBG << " Country:   " << (val_r.country()) << endl;
}

void test( const LanguageCode & val_r )
{
  DBG << " <<:        " << (val_r) << endl;
  DBG << " code:      " << (val_r.code()) << endl;
  DBG << " name:      " << (val_r.name()) << endl;
  DBG << " ==noCode:  " << (val_r == LanguageCode::noCode) << endl;
  DBG << " !=noCode:  " << (val_r != LanguageCode::noCode) << endl;
  DBG << " ==default: " << (val_r == LanguageCode::useDefault) << endl;
  DBG << " !=default: " << (val_r != LanguageCode::useDefault) << endl;
}

void test( const CountryCode & val_r )
{
  DBG << " <<:       " << (val_r) << endl;
  DBG << " code:     " << (val_r.code()) << endl;
  DBG << " name:     " << (val_r.name()) << endl;
  DBG << " ==noCode: " << (val_r == CountryCode::noCode) << endl;
  DBG << " !=noCode: " << (val_r != CountryCode::noCode) << endl;
}

void testLocale( const char * str = 0 )
{
  MIL << "Locale(" << vstr(str) << ")" << endl;
  test( str ? Locale( str ) : Locale() );
}

void testLanguage( const char * str = 0 )
{
  MIL << "Language(" << vstr(str) << ")" << endl;
  test( str ? LanguageCode( str ) : LanguageCode() );
}

void testCountry( const char * str = 0 )
{
  MIL << "Country(" << vstr(str) << ")" << endl;
  test( str ? CountryCode( str ) : CountryCode() );
}

/******************************************************************
**
**      FUNCTION NAME : main
**      FUNCTION TYPE : int
*/
int main( int argc, char * argv[] )
{
  //zypp::base::LogControl::instance().logfile( "xxx" );
  INT << "===[START]==========================================" << endl;

  testLocale();
  testLocale( "" );
  testLocale( "en" );
  testLocale( "de_DE" );
  testLocale( "C" );
  testLocale( "POSIX" );
  testLocale( "default" );

  testLanguage();
  testLanguage( "" );
  testLanguage( "en" );
  testLanguage( "default" );

  testCountry();
  testCountry( "" );
  testCountry( "US" );
  testCountry( "DE" );

  INT << "===[END]============================================" << endl;
  return 0;
}
