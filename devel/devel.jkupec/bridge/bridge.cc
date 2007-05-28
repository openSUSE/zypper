#include<iostream>

#include "zypp/base/Exception.h"

#include "Derived.h"


using namespace std;
using namespace jk;
using namespace zypp;

int main(int argc, char **argv)
{
  try
  {
    Derived d;
  }
  catch ( const Exception &e )
  {
    cout << "Oops! " << e.msg() << std::endl;
  }

  return 0;
}

// vim: set ts=2 sts=2 sw=2 et ai:
