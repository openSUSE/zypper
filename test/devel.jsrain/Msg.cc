#include <iostream>
#include <zypp/base/Logger.h>
#include <zypp/detail/MessageImpl.h>
#include <zypp/Message.h>


using namespace std;
using namespace zypp;

class MyMessageImpl : public detail::MessageImpl
{
  public:
    MyMessageImpl (std::string text)
    {
      _text = text;
    }
};

/******************************************************************
**
**
**	FUNCTION NAME : main
**	FUNCTION TYPE : int
**
**	DESCRIPTION :
*/
int main( int argc, char * argv[] )
{
  INT << "===[START]==========================================" << endl;

  std::string text = "My message to be shown to user";
  std::string _name( "msg1" );
  Edition _edition( "1.0", "42" );
  Arch    _arch( "noarch" );
  base::shared_ptr<detail::MessageImpl> mp1(new MyMessageImpl(text));
  Message::Ptr m( detail::makeResolvableFromImpl( _name, _edition, _arch, mp1));

  DBG << *m << endl;
  DBG << "  \"" << m->text() << "\"" << endl;

  INT << "===[END]============================================" << endl;
  return 0;
}
