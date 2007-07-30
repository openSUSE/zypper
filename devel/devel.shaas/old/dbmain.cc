#include <iostream>
#include <vector>
#include <map>
#include "db.h"
#include "resolvable.h"

#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/ResStore.h>
#include <zypp/Package.h>
#include <zypp/Source.h>
#include <zypp/CapFactory.h>

using namespace zypp;

unsigned int getResolvables(ResStore*);

int main(){
	
	static ZYpp::Ptr God;
	ResStore *store = new ResStore();

	std::cout << "Returncode: " << getResolvables(store) << std::endl;
	
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
	
	std::cout << "Number of elements in pool: " << God->pool().size() << std::endl;
	std::cout << "Verify System: " << God->resolver()->verifySystem() << std::endl;

	/*for (ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it) {
			if(it->resolvable()->name() == "tvbrowser"){
				CapSet caps = it->resolvable()->dep (Dep::REQUIRES);
				for (CapSet::const_iterator itCap = caps.begin(); itCap != caps.end(); ++itCap)
					std::cout << "Requires: " << itCap->op().asString()  << " " << itCap->asString() << std::endl;
			}
	}*/

	return 0;
}

unsigned int getResolvables(ResStore *store){

	database *dbDeps = new database("lorien.suse.de", "rpmread", "Salahm1", "rpm");
	database *dbPackages = new database("lorien.suse.de", "rpmread", "Salahm1", "package");

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

	Resolvable::Kind kind = ResTraits<Package>::kind;
	CapFactory factory;

	for(unsigned int i = 2000; i < packIDs.size(); i++){

		string sqlcom("SELECT PackID FROM Packages WHERE BasedOnID=");
		sqlcom.append(packIDs[i].at(0));
		dbPackages->sqlexecute(sqlcom);
		std::vector< std::vector<string> > basedIDs = dbPackages->getResult();
		
		std::vector< std::vector<string> > binPack;
	
		for(unsigned int j = 0; j < basedIDs.size(); j++){	
		
			sqlcom = "SELECT BinPackID, Version FROM BinaryPackages WHERE PackID=";
			sqlcom.append(basedIDs[j].at(0));
			dbDeps->sqlexecute(sqlcom);
			std::vector< std::vector<string> > tempVec = dbDeps->getResult();
			for(unsigned int x = 0; x < tempVec.size(); x++)
				binPack.push_back(tempVec.at(x));
		}

		intrusive_ptr<resolvZypp> pkg;
		CapSet prov;
		CapSet preq;
		CapSet req;
		CapSet conf;
		CapSet obs;
		CapSet rec;
		CapSet sug;
		CapSet fre;
		CapSet enh;
		CapSet sup;

		string edition = "";

		// If Deps
		if(binPack.size() != 0){

				std::vector< std::vector<string> > packDeps;

			for(unsigned int k = 0; k < binPack.size(); k++){
				sqlcom = "SELECT Symbol, Kind, Compare, Version FROM PackReqProv WHERE BinPackID=";
				sqlcom.append(binPack[k].at(0));
				dbDeps->sqlexecute(sqlcom);		
				std::vector< std::vector<string> > tempVec = dbDeps->getResult();
				for(unsigned int l = 0; l < tempVec.size(); l++)
					packDeps.push_back(tempVec.at(l));


			}

			for(unsigned int y = 0; y < packDeps.size(); y++){

				string ed = "";
				Rel rel = Rel::ANY;
				
				if(packDeps[y].at(0) == "(none)")
					continue;

				if(packDeps[y].at(2) != "NULL"){
					rel = Rel(packDeps[y].at(2));
					ed = packDeps[y].at(3);
				}
				

				if(packDeps[y].at(1) == "provides"){
					prov.insert(factory.parse(kind, packDeps[y].at(0)
							   , rel, Edition(ed)));
				}else if(packDeps[y].at(1) == "prerequires"){
					preq.insert(factory.parse(kind, packDeps[y].at(0)
							   , rel, Edition(ed)));
				}else if(packDeps[y].at(1) == "requires"){
					req.insert(factory.parse(kind, packDeps[y].at(0)
							   , rel, Edition(ed)));
				}else if(packDeps[y].at(1) == "conflicts"){
					conf.insert(factory.parse(kind, packDeps[y].at(0)
							   , rel, Edition(ed)));
				}else if(packDeps[y].at(1) == "obsoletes"){
					obs.insert(factory.parse(kind, packDeps[y].at(0)
							   , rel, Edition(ed)));
				}else if(packDeps[y].at(1) == "recommends"){
					rec.insert(factory.parse(kind, packDeps[y].at(0)
							   , rel, Edition(ed)));
				}else if(packDeps[y].at(1) == "suggests"){
					sug.insert(factory.parse(kind, packDeps[y].at(0)
							   , rel, Edition(ed)));
				}else if(packDeps[y].at(1) == "freshens"){
					fre.insert(factory.parse(kind, packDeps[y].at(0)
							   , rel, Edition(ed)));
				}else if(packDeps[y].at(1) == "enhances"){
					enh.insert(factory.parse(kind, packDeps[y].at(0)
							   , rel, Edition(ed)));
				}else if(packDeps[y].at(1) == "supplements"){
					sup.insert(factory.parse(kind, packDeps[y].at(0)
							   , rel, Edition(ed)));
				}
			}

			edition = binPack[0].at(1);

		}

		Dependencies deps;
		if(prov.size() > 0)
			deps[Dep::PROVIDES] = prov;
		if(preq.size() > 0)
			deps[Dep::PREREQUIRES] = preq;
		if(req.size() > 0)
			deps[Dep::REQUIRES] = req;
		if(conf.size() > 0)
			deps[Dep::CONFLICTS] = conf;
		if(obs.size() > 0)
			deps[Dep::OBSOLETES] = obs;
		if(rec.size() > 0)
			deps[Dep::RECOMMENDS] = rec;
		if(sug.size() > 0)
			deps[Dep::SUGGESTS] = sug;
		if(fre.size() > 0)
			deps[Dep::FRESHENS] = fre;
		if(enh.size() > 0)
			deps[Dep::ENHANCES] = enh;
		if(sup.size() > 0)
			deps[Dep::SUPPLEMENTS] = sup;

		//std::cout << "Package: " << packIDs[i].at(1) << std::endl;
		NVRAD nvPkg(packIDs[i].at(1), Edition(edition), Arch("i386"), deps);

		CapSet::const_iterator testIter;

		for(testIter = req.begin(); testIter != req.end(); testIter++){
			//std::cout << testIter->asString() << std::endl;
		}

		Package::Ptr p( detail::makeResolvableAndImpl(nvPkg, pkg));		

		store->insert(p);

		if(i%1000 == 0)
			std::cout << std::endl << i << " packages parsed!\n";
	}

	dbDeps->close();
	dbPackages->close();

	return 0;

}

int old_test(){

	//* old Test-Main
	database *db = new database("lorien.suse.de", "rpmread", "Salahm1", "rpm");
	//database *db = new database("lorien.suse.de", "rpmread", "Salahm1", "package");
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
		
	//std::cout << "Anzahl an Zeilen: "  << result.size() << std::endl;
	return 0;
}
