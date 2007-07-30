#include <pdbtozypp/pdbtozypp.h>

using namespace zypp;
using namespace std;
int main(){
	
	static ZYpp::Ptr God;
	ResStore store = pdbToZypp();

	try {
		God = zypp::getZYpp();
	}
	catch (const Exception & excpt_r ) {
		ZYPP_CAUGHT( excpt_r );
		cerr << "ZYPP no available" << endl;
		return 1;
	}

	God->addResolvables(store);
	cout << "Elements in pool: " << God->pool().size() << endl;
	return 0;
}
