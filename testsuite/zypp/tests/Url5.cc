#include "zypp/Url.h"
#include <stdexcept>
#include <iostream>

int main(void)
{
  struct Test {
    char *str;
    char *inf;
    int  exp;
  };
  struct Test tests[] = {
    {"ldap:///dc=foo,dc=bar",      "invalid: no host is ok for ldap", 1},
    {"ftp:///foo/bar",             "throws:  host is mandatory",      2},
    {"http:///%2f/srv/ftp",        "throws:  host is mandatory",      2},
    {"file://localhost/some/path", "valid:   host is allowed",        0},
    {"cd://localhost/some/path",   "throws:  host not allowed",       2},
    {"mailto:",                    "throws:  no path (email)",        2},
    {"cd:",                        "throws:  no path",                2},
    {"cd:///some/path",            "valid:   no host, path is there", 0},
    {NULL}
  };

  try
  {
    zypp::Url url;
    for(struct Test *test=tests; test && test->str; test++)
    {
      std::cout << "STR: " << test->str << std::endl;
      std::cout << "INF: " << test->inf << std::endl;
      try
      {
        url = test->str;

        std::cout << "URL: " << url.asString() << std::endl;

        bool valid = url.isValid();
        std::cout << "OK?: " << (valid ? "valid" : "invalid")
                             << std::endl;

        if( valid && test->exp != 0)
            ZYPP_THROW(zypp::Exception("Unexpected result: exp != 0"));
        else
        if( !valid && test->exp != 1)
            ZYPP_THROW(zypp::Exception("Unexpected result: exp == 1"));
      }
      catch(const zypp::url::UrlException &)
      {
        std::cout << "ERR: exception caught" << std::endl;
        if(test->exp != 2)
          ZYPP_THROW(zypp::Exception("Unexpected result exp != 2"));
      }
      std::cout << std::endl;
    }
  }
  catch(const zypp::Exception &e)
  {
    ZYPP_CAUGHT(e);
	  return 1;
  }
	return 0;
}

// vim: set ts=2 sts=2 sw=2 ai et:
