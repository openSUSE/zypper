#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/Package.h"

#include "zypp/TmpPath.h"
#include "zypp/ExternalProgram.h"
#include "zypp/ProgressData.h"
#include "zypp/repo/yum/Downloader.h"

#include "zypp/sat/Pool.h"

#include "zypp/PoolQuery.h"

using namespace std;
using namespace zypp;
using namespace zypp::repo;
using namespace zypp::filesystem;

class RpmBuilder
{
public:
    RpmBuilder()
    {
        Pathname top = _tmptop.path();
        Pathname rcpath = _tmprc.path();
        
        assert_dir(top / "build" );
        assert_dir(top / "rpms" );
        
        /* create a rpm configuration file and
           setup the macros file */
        std::ofstream rcfile(rcpath.c_str());
        if (!rcfile)
            ZYPP_THROW (Exception( "Can't open " + rcpath.asString() ) );
        
        rcfile << "macrofiles: " << _tmpmacros.path() << endl;
        rcfile.close();
    }
    
    Pathname rpmsDir() const
    {
        return _tmptop.path() / "rpms";
    }
    
    void createRpmMetadata() const
    {
        const char* argv[] =
        {
            "createrepo",
            rpmsDir().c_str(),
            NULL
        };
        ExternalProgram prog(argv,ExternalProgram::Normal_Stderr, false, -1, true);
        string line;
        int count;
        for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
        {
            cout << line;
        }
        prog.close();

    }
    
    void buildSpec( const Pathname &spec )
    {
        Pathname basedir = spec.dirname();
        Pathname rcpath = _tmprc.path();
        Pathname macrospath = _tmpmacros.path();
        
        std::ofstream macrosfile(macrospath.c_str());
        if (!macrosfile)
            ZYPP_THROW (Exception( "Can't open " + macrospath.asString() ) );
        
        macrosfile << "%topdir " << _tmptop.path() << endl;
        macrosfile << "%_builddir %{topdir}/build" << endl;
        macrosfile << "%_rpmdir %{topdir}/rpms" << endl;
        macrosfile << "%_srcrpmdir %{topdir}/rpms" << endl;
        macrosfile << "%_sourcedir " << basedir << endl;
        macrosfile << "%_specdir " << basedir << endl;
 
        macrosfile.close();
        
        const char* argv[] =
        {
            "rpmbuild",
            "--rcfile",
            rcpath.c_str(),
            "-bb",
            //"--clean",
            "--buildroot",
            _tmpbuildroot.path().c_str(),
            spec.c_str(),
            NULL
        };
        ExternalProgram prog(argv,ExternalProgram::Normal_Stderr, false, -1, true);
        string line;
        int count;
        for(line = prog.receiveLine(), count=0; !line.empty(); line = prog.receiveLine(), count++ )
        {
            cout << line;
        }
        prog.close();

    }
    
private:
    TmpDir _tmptop;
    TmpFile _tmprc;
    TmpFile _tmpmacros;
    TmpDir _tmpbuildroot;
};

int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
    
      //z->initializeTarget("/");
      //z->target()->load();

      //sat::Pool::instance().addRepoSolv("./foo.solv");

//       for ( ResPool::const_iterator it = z->pool().begin(); it != z->pool().end(); ++it )
//       {
//         ResObject::constPtr res = it->resolvable();
//         if ( res->name() == "kde4-kcolorchooser")
//         {
//           cout << res << endl;
//           cout << res->summary() << " | " << res->size() << endl;
//         }
//       }

      //query.execute("kde", &result_cb);
      
      RpmBuilder builder;
      builder.buildSpec("/space/git/hwenable/spec/testdriver.spec");
      builder.createRpmMetadata();
      
      
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      cout << e.msg() << endl;
    }
    
    return 0;
}



