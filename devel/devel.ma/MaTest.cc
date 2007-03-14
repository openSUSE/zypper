#include <iostream>

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qvbox.h>
#include <qdir.h>

#include "Tools.h"
#include "FakePool.h"

#include <zypp/base/PtrTypes.h>
#include <zypp/base/Exception.h>

#include "zypp/ZYppFactory.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/ResTraits.h"
#include "zypp/ResPool.h"
#include "zypp/Package.h"

using std::endl;
using namespace zypp;

///////////////////////////////////////////////////////////////////

void setup( QVBox & top, int argc, char **argv )
{
  if ( ! argc )
    return;

  MIL << argv[0] << endl;
}

///////////////////////////////////////////////////////////////////

int main( int argc, char **argv )
{
  base::LogControl::instance().logfile( "-" );
  INT << "===[START]==========================================" << endl;

  QApplication app( argc, argv );
  QVBox        top;

  --argc;
  ++argv;
  setup( top, argc, argv );

  QPushButton quit( "Quit", &top );
  QObject::connect( &quit,SIGNAL(clicked()), &app, SLOT(quit()) );
  app.setMainWidget( &top );
  top.show();
  INT << "===[LOOP]==========================================" << endl;
  return app.exec();
}

///////////////////////////////////////////////////////////////////
//#include "QPool.moc"
///////////////////////////////////////////////////////////////////


