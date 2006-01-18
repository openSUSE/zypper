#include <zypp/Url.h>
#include <stdexcept>
#include <iostream>

int main(void)
{
  try
  {
    std::string str;
    zypp::Url   url;

    str = "ldap://example.net/dc=example,dc=net?cn,sn?sub?(cn=*)";
    std::cout << "STR: " << str << std::endl;

    url = str;
    std::cout << "URL: " << url.toString() << std::endl;

    zypp::url::ParamVec pvec( url.getQueryStringVec());
    zypp::url::ParamVec::const_iterator v;
    for(v=pvec.begin(); v!=pvec.end(); ++v)
    {
      std::cout << "PARAM: " << *v << std::endl;
    }
    std::cout << std::endl;

    zypp::url::ParamMap pmap( url.getQueryStringMap());
    zypp::url::ParamMap::const_iterator m;
    for(m=pmap.begin(); m!=pmap.end(); ++m)
    {
      std::cout << "KEY: " << m->first  << std::endl;
      std::cout << "VAL: " << m->second << std::endl;
      std::cout << std::endl;
    }

    url.setQueryParam("attrs", "cn,sn,uid");
    std::cout << "OUT: " << url.toString() << std::endl;
  }
  catch(const std::invalid_argument &e)
  {
    std::cout << "ERR: " << e.what() << std::endl << std::endl;
  }

	return 0;
}

// vim: set ts=2 sts=2 sw=2 ai et:
