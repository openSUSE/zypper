#include <vector>
#include "db.h"
#include "resolvable.h"

#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ResStore.h>
#include <zypp/Package.h>
#include <zypp/Source.h>
#include <zypp/CapFactory.h>

class PdbToZypp{
	private:
		zypp::ResStore store;
	public:
		PdbToZypp();
		zypp::ResStore getStore();
};
