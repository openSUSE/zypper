#include <zypp/Url.h>
#include <stdexcept>
#include <iostream>

int main(void)
{
  zypp::Url   url("http://localhost/path/to?foo=bar#frag");
  std::string str;

  str = url.toString();
  std::cout << "URL1: " << str << std::endl << std::endl;

  // will be encoded to "foo%3Dbar%26key=foo%26bar%3Dvalue"
  url.setQueryParam("foo=bar&key", "foo&bar=value");

  str = url.toString();
  std::cout << "URL2: " << str << std::endl << std::endl;

	return 0;
}

// vim: set ts=2 sts=2 sw=2 ai et:
