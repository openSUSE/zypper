
#ifndef TEST_MYMEDIAVERIFIER
#define TEST_MYMEDIAVERIFIER

#include <zypp/media/MediaManager.h>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

using namespace zypp;
using namespace zypp::media;

/*
** Very basic example verifier.
**
** This one does not know anything about the product, it
** just checks if /media.1 (limited to 1st CD) exists...
*/
class MyMediaVerifier: public MediaVerifierBase
{
  private:
  // std::string _productname;
  public:
    MyMediaVerifier(/* std::string &productname */)
  : MediaVerifierBase()
    //, _productname(productname)
    {}

    virtual
        ~MyMediaVerifier()
    {}

    virtual bool
        isDesiredMedia(const MediaAccessRef &ref)
    {
      DBG << "isDesiredMedia(): for media nr 1 " << std::endl;

      if( !ref)
        DBG << "isDesiredMedia(): invalid media handle" << std::endl;

      std::list<std::string> lst;
      Pathname               dir("/media.1");

      DBG << "isDesiredMedia(): checking " << dir.asString() << std::endl;

    // check the product e.g. via /media.1/products as well...
      try
      {
        if( ref)
          ref->dirInfo(lst, dir, false);
      }
      catch(const zypp::Exception &e)
      {
        ZYPP_CAUGHT(e);
      }
      DBG << "isDesiredMedia(): media "
          << (lst.empty() ? "does not contain" : "contains")
          << " the " << dir.asString() << " directory."
          << std::endl;

      return !lst.empty();
    }
};

#endif
