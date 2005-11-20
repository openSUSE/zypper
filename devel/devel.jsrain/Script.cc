#include <iostream>
#include <zypp/base/Logger.h>
#include <zypp/detail/ScriptImpl.h>
#include <zypp/Script.h>


using namespace std;
using namespace zypp;

class MyScriptImpl : public detail::ScriptImpl
{
  public:
    MyScriptImpl (std::string do_script, std::string undo_script = "") 
    {
      _do_script = do_script;
      _undo_script = undo_script;
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

  std::string do_script = "#!/bin/bash\ndo_script";
  std::string undo_script = "#!/bin/bash\nundo_script";
  std::string _name( "script1" );
  Edition _edition( "1.0", "42" );
  Arch    _arch( "noarch" );
  base::shared_ptr<detail::ScriptImpl> sp1(new MyScriptImpl(do_script));
  Script::Ptr s( detail::makeResolvableFromImpl( _name, _edition, _arch, sp1));

  DBG << *s << endl;
  DBG << "  \"" << s->do_script() << "\"" << endl;
  DBG << "  \"" << s->undo_script() << "\"" << endl;
  DBG << "  " << s->undo_available() << endl;

  INT << "====================================================" << endl;

  base::shared_ptr<detail::ScriptImpl> sp2(new MyScriptImpl(do_script, undo_script));
  Script::Ptr s2( detail::makeResolvableFromImpl( _name, _edition, _arch, sp2));

  DBG << *s2 << endl;
  DBG << "  \"" << s2->do_script() << "\"" << endl;
  DBG << "  \"" << s2->undo_script() << "\"" << endl;
  DBG << "  " << s2->undo_available() << endl;

  INT << "===[END]============================================" << endl;
  return 0;
}
