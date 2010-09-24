#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/ZYppFactory.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"
#include "zypp/ZConfig.h"
#include "zypp/repo/PluginServices.h"
#include "zypp/ServiceInfo.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;
using namespace zypp::repo;

#define DATADIR (Pathname(TESTS_SRC_DIR) +  "/repo/yum/data")

class ServiceCollector
{
public:
  typedef std::set<ServiceInfo> ServiceSet;
    
  ServiceCollector( ServiceSet & services_r )
    : _services( services_r )
  {}

  bool operator()( const ServiceInfo & service_r ) const
  {
    _services.insert( service_r );
    return true;
  }

private:
  ServiceSet & _services;
};


BOOST_AUTO_TEST_CASE(plugin_services)
{
  ServiceCollector::ServiceSet services;
    
  PluginServices local("/space/tmp/services", ServiceCollector(services));
}

// vim: set ts=2 sts=2 sw=2 ai et:
