/*
** Check if the url by scheme repository works, e.g.
** if there are some initialization order problems
** (ViewOption) causing asString to format its string
** differently than configured.
*/
#include "zypp/Url.h"
#include <stdexcept>
#include <iostream>
#include <cassert>

int main(void)
{
  try
  {
    std::string str, one, two;
    zypp::Url   url;


    // asString & asCompleteString should not print "mailto://"
    str = "mailto:feedback@example.com?subject=hello";
    one = str;
    two = str;
    url = str;
    std::cout << "STR: " << str                    << std::endl;
    std::cout << "ONE: " << url.asString()         << std::endl;
    std::cout << "TWO: " << url.asCompleteString() << std::endl;
    assert( one == url.asString());
    assert( two == url.asCompleteString());
    std::cout << std::endl;


    // asString & asCompleteString should add empty authority
    // "dvd://...", except we request to avoid it.
    str = "dvd:/srv/ftp";
    one = "dvd:///srv/ftp";
    two = "dvd:///srv/ftp";
    url = str;
    std::cout << "STR: " << str                    << std::endl;
    std::cout << "ONE: " << url.asString()         << std::endl;
    std::cout << "TWO: " << url.asCompleteString() << std::endl;
    assert( one == url.asString());
    assert( two == url.asCompleteString());
    assert( str  == url.asString(zypp::url::ViewOptions() -
                                 zypp::url::ViewOption::EMPTY_AUTHORITY));
    std::cout << std::endl;


    // asString shouldn't print the password, asCompleteString should
    // further, the "//" at the begin of the path should become "/%2F"
    str = "ftp://user:pass@localhost//srv/ftp";
    one = "ftp://user@localhost/%2Fsrv/ftp";
    two = "ftp://user:pass@localhost/%2Fsrv/ftp";
    url = str;
    std::cout << "STR: " << str                    << std::endl;
    std::cout << "ONE: " << url.asString()         << std::endl;
    std::cout << "TWO: " << url.asCompleteString() << std::endl;
    assert( one == url.asString());
    assert( two == url.asCompleteString());
    std::cout << std::endl;


    // asString shouldn't print the password, asCompleteString should
    // further, the "//" at the begin of the path should be keept.
    str = "http://user:pass@localhost//srv/ftp";
    one = "http://user@localhost//srv/ftp";
    two = str;
    url = str;
    std::cout << "STR: " << str                    << std::endl;
    std::cout << "ONE: " << url.asString()         << std::endl;
    std::cout << "TWO: " << url.asCompleteString() << std::endl;
    assert( one == url.asString());
    assert( two == url.asCompleteString());
    std::cout << std::endl;

  }
  catch(const zypp::url::UrlException &e)
  {
    ZYPP_CAUGHT(e);
    return 2;
  }

  return 0;
}

// vim: set ts=2 sts=2 sw=2 ai et:
