#include <vector>
#include "db.h"
#include "resolvable.h"

#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ResStore.h>
#include <zypp/Package.h>
//#include <zypp/Source.h>
#include <zypp/CapFactory.h>

class PdbToZypp{
	public:
      //typedef zypp::ResStore ResStore;
		//PdbToZypp(zypp::ResStore  & _store);
		PdbToZypp();
      ~PdbToZypp();
      void readOut();
      zypp::ResStore getStore();
   private:
      zypp::ResStore store;
};
