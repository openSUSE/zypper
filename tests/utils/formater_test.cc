#include "TestSetup.h"
#include "output/Out.h"

typedef std::vector<std::string> Container;
Container container {
  "aaaa",  "bbbb",  "cccc",  "dddd",  "eeee",
  "ffff",  "gggg",  "hhhh",  "iiii",  "jjjj",
};

struct Formater
{
  //typedef out::DefaultListLayout NormalLayout;

  std::string listElement( const std::string & val_r ) const
  { return val_r; }
};

BOOST_AUTO_TEST_CASE(init)
{
  struct OutTest : public OutNormal
  {
    using OutNormal::OutNormal;
    unsigned termwidth() const override { return 30; }
  };
  Zypper::instance().setOutputWriter( new OutTest );
}


template <class TFormater, class TLayout>
std::ostream & writeout( std::ostream & str_r, const Container & container_r, TFormater && formater_r, TLayout && layout_r )
{
  str_r << "+++++" << endl;
  out::writeContainer( str_r, container_r, std::forward<TFormater>(formater_r), std::forward<TLayout>(layout_r) );
  str_r << "-----" << endl;
  return str_r;
}

template <class TFormater, class TLayout>
std::string writestr( const Container & container_r, TFormater && formater_r, TLayout && layout_r )
{
  std::ostringstream out;
  writeout( out, container_r, std::forward<TFormater>(formater_r), std::forward<TLayout>(layout_r) );
  return out.str();
}


BOOST_AUTO_TEST_CASE(list_formater)
{
  BOOST_CHECK_EQUAL( writestr( container, Formater(), out::DefaultListLayout() ),
"+++++\n\
aaaa\n\
bbbb\n\
cccc\n\
dddd\n\
eeee\n\
ffff\n\
gggg\n\
hhhh\n\
iiii\n\
jjjj\n\
-----\n" );
  BOOST_CHECK_EQUAL( writestr( container, Formater(), out::DefaultGapedListLayout() ),
"+++++\n\
\n\
aaaa\n\
\n\
bbbb\n\
\n\
cccc\n\
\n\
dddd\n\
\n\
eeee\n\
\n\
ffff\n\
\n\
gggg\n\
\n\
hhhh\n\
\n\
iiii\n\
\n\
jjjj\n\
-----\n" );
  BOOST_CHECK_EQUAL( writestr( container, Formater(), out::IndentedListLayout() ),
"+++++\n\
  aaaa\n\
  bbbb\n\
  cccc\n\
  dddd\n\
  eeee\n\
  ffff\n\
  gggg\n\
  hhhh\n\
  iiii\n\
  jjjj\n\
-----\n" );

  BOOST_CHECK_EQUAL( writestr( container, Formater(), out::IndentedGapedListLayout() ),
"+++++\n\
\n\
  aaaa\n\
\n\
  bbbb\n\
\n\
  cccc\n\
\n\
  dddd\n\
\n\
  eeee\n\
\n\
  ffff\n\
\n\
  gggg\n\
\n\
  hhhh\n\
\n\
  iiii\n\
\n\
  jjjj\n\
-----\n" );

  BOOST_CHECK_EQUAL( writestr( container, Formater(), out::CompressedListLayout() ),
"+++++\n\
  aaaa bbbb cccc dddd eeee\n\
  ffff gggg hhhh iiii jjjj\n\
-----\n" );
}
