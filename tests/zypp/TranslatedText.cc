// Arch.cc
//
// tests for Arch
//

#include <iostream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/TranslatedText.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

using namespace std;
using namespace zypp;

int main( int argc, char * argv[] )
{
  ZYpp::Ptr god;
  
  try { 
    god = getZYpp();
  }
  catch( const Exception &e )
  {
    return 99;
  }
  
  TranslatedText testTT;
  MIL << "Locale: en" << std::endl;
  god->setTextLocale(Locale("en"));
  testTT.setText("default");
  if ( testTT.text() != "default" )
    return 2;
  
  testTT.setText("default english", Locale("en"));
  if ( testTT.text() != "default english" )
    return 3;
  
  MIL << "Locale: es_ES" << std::endl;
  god->setTextLocale(Locale("es_ES"));
  
  if ( testTT.text() != "default english" )
  {
    ERR << testTT.text() << std::endl;
    return 4;
  }
    
  testTT.setText("hola esto es neutro", Locale("es"));
  testTT.setText("this is neutral", Locale("en"));
  
  if ( testTT.text() != "hola esto es neutro" )
    return 5;
    
  testTT.setText("hola Spain", Locale("es_ES"));
  if ( testTT.text() != "hola Spain" )
    return 6;
  
  MIL << "Locale: null" << std::endl;
  god->setTextLocale(Locale());
  if ( testTT.text() != "default" )
  {
    ERR << testTT.text() << std::endl;
    return 7;
  }
  
  return 0;
}
