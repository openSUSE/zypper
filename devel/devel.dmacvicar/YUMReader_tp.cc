#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/CapFactory.h"
#include "zypp/data/ResolvableDataConsumer.h"
#include "zypp/base/Measure.h"
#include "zypp/detail/ResObjectFactory.h"
#include "zypp/parser/yum/RepoParser.h"
#include "zypp/repo/memory/PackageImpl.h"


#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "yumparsertest"

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;
using zypp::debug::Measure;
using namespace zypp::repo::memory;

bool progress_function(ProgressData::value_type p)
{
  cout << "Parsing YUM source [" << p << "%]" << endl;
//  cout << "\rParsing YUM source [" << p << "%]" << flush;
  return true;
}

class ResolvableConsumer : public data::ResolvableDataConsumer
{
  public:
    
  typedef detail::ResImplTraits<PackageImpl>::Ptr PkgImplPtr;
  typedef detail::ResImplTraits<PackageImpl>::Ptr SrcPkgImplPtr;
  
  ResolvableConsumer()
  {
  
  }
  
  void collectDeps( Dependencies &deps, const data::Dependencies &data_deps)
  {
    CapFactory factory;
    for ( data::Dependencies::const_iterator i = data_deps.begin(); i != data_deps.end(); ++i )
    {
      data::DependencyList list(i->second);
      zypp::Dep deptype(i->first);
      for ( data::DependencyList::const_iterator it = list.begin(); it != list.end(); ++it )
      {
        deps[deptype].insert(factory.fromImpl(*it));
      }
    }
  }
  
  virtual ~ResolvableConsumer()
  {
  
  }

  virtual void consumePackage( const data::RecordId &repository_id, data::Package_Ptr ptr )
  {
    PkgImplPtr impl = PkgImplPtr( new PackageImpl(ptr) );
    Dependencies deps;
    collectDeps( deps, ptr->deps );
    
    Package::Ptr pkg = detail::makeResolvableFromImpl( NVRAD( ptr->name, ptr->edition, ptr->arch, deps), impl );
    _store.insert(pkg);
  }
  virtual void consumeProduct( const data::RecordId &repository_id, data::Product_Ptr )
  {
  }
  virtual void consumePatch( const data::RecordId &repository_id, data::Patch_Ptr )
  {
  }
  virtual void consumeMessage( const data::RecordId &repository_id, data::Message_Ptr )
  {
  
  }
  
  virtual void consumeScript( const data::RecordId &repository_id, data::Script_Ptr )
  {
  
  }

  virtual void consumeChangelog( const data::RecordId & repository_id, const data::Resolvable_Ptr &, const Changelog & )
  {
  
  }
  
  virtual void consumeFilelist( const data::RecordId & repository_id, const data::Resolvable_Ptr &, const data::Filenames & )
  {}

  
  virtual void consumeSourcePackage(const zypp::data::RecordId&, zypp::data::SrcPackage_Ptr)
  {}
  
  virtual void consumePackageAtom(const zypp::data::RecordId&, const zypp::data::PackageAtom_Ptr&)
  {}
  
  virtual void consumePattern(const zypp::data::RecordId&, zypp::data::Pattern_Ptr)
  {}

    //virtual void consumeSourcePackage( const data::SrcPackage_Ptr ) = 0;
  ResStore _store;
};
 

int main(int argc, char **argv)
{
  base::LogControl::instance().logfile("yumparsertest.log");
  
  if (argc < 2)
  {
    cout << "usage: yumparsertest path/to/yumsourcedir" << endl << endl;
    return 1;
  }

  try
  {
    ZYpp::Ptr z = getZYpp();

    MIL << "creating PrimaryFileParser" << endl;
    Measure parse_primary_timer("primary.xml.gz parsing");
    ResolvableConsumer store;
    parser::yum::RepoParser parser( 0, store, &progress_function);
    parser.parse(argv[1]);
    parse_primary_timer.stop();

    cout << endl;
  }
  catch ( const Exception &e )
  {
    cout << "Oops! " << e.msg() << std::endl;
  }

  return 0;
}

// vim: set ts=2 sts=2 sw=2 et ai:
