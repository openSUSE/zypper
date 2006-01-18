#include <zypp/Url.h>
#include <stdexcept>
#include <iostream>

int main(void)
{
  zypp::Url   url("http://localhost/path/to#frag");
  std::string key;
  std::string val;

  // will be encoded as "hoho=ha%20ha"
  key = "hoho";
  val = "ha ha";
  url.setQueryParam(key, val);
  std::cout << "ADD1: '" << key << "' = '" << val << "'" << std::endl;
  std::cout << "URL1: " << url.toString() << std::endl << std::endl;

  // will be encoded as "foo%3Dbar%26key=foo%26bar%3Dvalue"
  key = "foo=bar&key";
  val = "foo&bar=value";
  url.setQueryParam(key, val);
  std::cout << "ADD2: '" << key << "' = '" << val << "'" << std::endl;
  std::cout << "URL2: " << url.toString() << std::endl << std::endl;

  // will be encoded as "foo%25bar=is%25de%25ad"
  key = "foo%bar";
  val = "is%de%ad";
  url.setQueryParam(key, val);
  std::cout << "ADD3: '" << key << "' = '" << val << "'" << std::endl;
  std::cout << "URL3: " << url.toString() << std::endl << std::endl;

  // get encoded query parameters:
  std::cout << "QUERY PARAMS:" << std::endl;
  zypp::url::ParamVec params( url.getQueryStringVec());
  for( size_t i=0; i<params.size(); i++)
  {
    std::cout << "\t" << params[i] << std::endl;
  }
  std::cout << std::endl;

	return 0;
}

// vim: set ts=2 sts=2 sw=2 ai et:
