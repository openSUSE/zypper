#include "zypp/Url.h"
#include <stdexcept>
#include <iostream>

int main(void)
{
  std::string str;

  str = "http://user:pass@localhost:/path/to;version=1.1?arg=val#frag";
  std::cout << "STR:  " << str << std::endl << std::endl;

  zypp::Url   url;
  url = str;

  str = url.asString();
  std::cout << "URL1: " << str << std::endl << std::endl;

  str = url.asString(zypp::url::ViewOptions() +
                     zypp::url::ViewOptions::WITH_PASSWORD);
  std::cout << "URL2: " << str << std::endl << std::endl;

  str = url.asString(zypp::url::ViewOptions() +
                     zypp::url::ViewOptions::WITH_PATH_PARAMS);
  std::cout << "URL3: " << str << std::endl << std::endl;

  str = url.asCompleteString();
  std::cout << "URL4: " << str << std::endl << std::endl;

	return 0;
}

// vim: set ts=2 sts=2 sw=2 ai et:
