#include <iostream>
#include <zypp/base/Logger.h>
#include <zypp/detail/ScriptImpl.h>


using namespace std;
using namespace zypp;

class MyScriptImpl : public detail::ScriptImpl
{
  public:
    MyScriptImpl (std::string do_script, std::string undo_script = "") 
    : ScriptImpl ("script1",
		  Edition (),
		  Arch ("noarch"))
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

  MyScriptImpl p ("#!/bin/bash\ndo_script");

  DBG << p << endl;
  DBG << "  \"" << p.do_script() << "\"" << endl;
  DBG << "  \"" << p.undo_script() << "\"" << endl;
  DBG << "  " << p.undo_available() << endl;

  INT << "====================================================" << endl;

  MyScriptImpl q ("#!/bin/bash\ndo_script", "#!/bin/bash\nundo_script");

  DBG << q << endl;
  DBG << "  \"" << q.do_script() << "\"" << endl;
  DBG << "  \"" << q.undo_script() << "\"" << endl;
  DBG << "  " << q.undo_available() << endl;

  INT << "===[END]============================================" << endl;
  return 0;
}
