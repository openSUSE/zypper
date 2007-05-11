#include <iostream>
#include <vector>
#include "db.h"
#include "resolvable.h"

#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ResStore.h>
#include <zypp/Package.h>
#include <zypp/Source.h>
#include <zypp/CapFactory.h>

using namespace zypp;

unsigned int getResolvables();

int main(){
	//* old Test-Main
	//database *db = new database("lorien.suse.de", "rpmread", "Salahm1", "rpm");
	database *db = new database("lorien.suse.de", "rpmread", "Salahm1", "package");
	//resolvable *test = new resolvable("art", "386", "1.0", "99", "tr");

	string sqlcom = "";

	if(db->connect() == 1){
		while(1){
			std::cout << "SQL-Kommando: ";
			std::getline(std::cin, sqlcom);

			if(sqlcom.compare("quit") == 0)
				break;

			db->sqlexecute(sqlcom);
		}
	}

	//test->addDep(REQUIRES, "test2");
	std::vector< std::vector<string> > result = db->getResult();
	db->close();

	for(unsigned int i = 0; i < result.size(); i++){
		for(unsigned int y = 0; y < result[i].size(); y++)
			std::cout << result[i].at(y) << " | ";
		std::cout << std::endl;
	}
	//*/	
	//std::cout << "Anzahl an Zeilen: "  << result.size() << std::endl;
	
	
	//std::cout << "Returncode: " << getResolvables() << std::endl;
	
	return 0;
}

unsigned int getResolvables(){

	database *dbDeps = new database("lorien.suse.de", "rpmread", "Salahm1", "rpm");
	database *dbPackages = new database("lorien.suse.de", "rpmread", "Salahm1", "package");

	ResStore *store = new ResStore();

	static ZYpp::Ptr God;

	if(dbPackages->connect() != 1){
		std::cout << "NO DB CONNECTION!!!\n";
		return 1;
	}

	if(dbDeps->connect() != 1){
		std::cout << "NO DB CONNECTION!!!\n";
		return 1;
	}

	dbPackages->sqlexecute("SELECT PackID, PackNameShort, PackStatus FROM Packages WHERE CDReleaseID = 10 AND PackStatus IN (0, 6, 7, 8) AND BasedOnID IS NULL");

	std::vector< std::vector<string> > packIDs = dbPackages->getResult();
	std::cout << "get packages from db...\n";
	for(unsigned int i = 0; i < packIDs.size(); i++){

		string sqlcom("SELECT BinPackID, Version FROM BinaryPackages WHERE PackID=");
		sqlcom.append(packIDs[i].at(0));
		dbDeps->sqlexecute(sqlcom);
		std::vector< std::vector<string> > binPack = dbDeps->getResult();
		
		intrusive_ptr<resolvZypp> pkg;
		CapSet caps;
		string edition = "";

		// If Deps
		if(binPack.size() != 0){

			sqlcom = "SELECT Symbol, Kind, Compare, Version FROM PackReqProv WHERE BinPackID=";
			sqlcom.append(binPack[0].at(0));
			dbDeps->sqlexecute(sqlcom);		
			std::vector< std::vector<string> > packDeps = dbDeps->getResult();

			for(unsigned int y = 0; y < packDeps.size(); y++){

				string ed = "";
				string rel = "";
				
				if(packDeps[y].at(2) != "NULL"){
					rel = packDeps[y].at(2);
					ed = packDeps[y].at(3);
				}
				caps.insert(CapFactory().parse(Resolvable::Kind(packDeps[y].at(1)), packDeps[y].at(0)
							 , Rel(rel), Edition(ed)));
			}

			edition = binPack[0].at(1);

		}

		//Dependencies deps(caps);

		NVRAD nvPkg(packIDs[i].at(1), Edition(edition), Arch("i386"));
		Package::Ptr p( detail::makeResolvableAndImpl(nvPkg, pkg));		

		store->insert(p);

		if(i%1000 == 0)
				std::cout << i << " packages parsed!\n";
	}

	dbDeps->close();
	dbPackages->close();


	// Get Zypp lock	
	try {
	    God = zypp::getZYpp();
    }
    catch (const Exception & excpt_r ) {
    	ZYPP_CAUGHT( excpt_r );
		std::cerr << "Can't aquire ZYpp lock" << std::endl;
    	return 1;
    }	

	God->addResolvables(*store);

	return 0;

}

