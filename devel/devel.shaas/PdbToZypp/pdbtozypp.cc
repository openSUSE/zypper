#include "pdbtozypp.h"
#include <iostream>

using namespace zypp;

//Constructor
PdbToZypp::PdbToZypp(){
   //store = _store;
}

PdbToZypp::~PdbToZypp(){

}

int PdbToZypp::readOut(){

   //store = new ResStore;

	database *dbDeps = new database("lorien.suse.de", "rpmread", "rrrrrrr", "rpm");
	database *dbPackages = new database("lorien.suse.de", "rpmread", "rrrrrrr", "package");

	if(dbPackages->connect() != 1){
		std::cout << "NO DB CONNECTION!!!\n";
		return 0;
	}

	if(dbDeps->connect() != 1){
		std::cout << "NO DB CONNECTION!!!\n";
		return 0;
	}

	dbPackages->sqlexecute("SELECT PackID, PackNameShort, PackStatus FROM Packages WHERE CDReleaseID IN (10, 64) AND PackStatus IN (0, 6, 7, 8) OR PackStatus IS NULL AND BasedOnID IS NULL");

	std::vector< std::vector<string> > packIDs = dbPackages->getResult();

	Resolvable::Kind kind = ResTraits<Package>::kind;
	CapFactory factory;

	for(unsigned int i = 0; i < packIDs.size(); i++){

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

            sqlcom = "SELECT name, DirID FROM PackFilelist WHERE DirID IN(1, 22, 24, 96, 178, 756, 1981) AND BinPackID=";
            sqlcom.append(binPack[k].at(0));
            dbDeps->sqlexecute(sqlcom);
            tempVec = dbDeps->getResult();

            for(unsigned int m = 0; m < tempVec.size(); m++){
               sqlcom = "SELECT dir FROM PackFileDirs WHERE DirID=";
               sqlcom.append(tempVec[m].at(1));
               dbDeps->sqlexecute(sqlcom);
               std::vector< std::vector<string> > tempVec2 = dbDeps->getResult();
               for(unsigned int n = 0; n < tempVec2.size(); n++){
                  string fileprov = tempVec2[n].at(0) + "/" + tempVec[m].at(0);
                  prov.insert(factory.parse(kind, fileprov, Rel::ANY, Edition("")));
               }

            }
			}

			for(unsigned int y = 0; y < packDeps.size(); y++){

				string ed = "";
				string symbol = packDeps[y].at(0);
				Rel rel = Rel::ANY;
				
				if(packDeps[y].at(0) == "(none)")
					continue;

				if(packDeps[y].at(2) != "NULL"){
					rel = Rel(packDeps[y].at(2));
					ed = packDeps[y].at(3);
				}
				

				if(packDeps[y].at(1) == "provides"){
					if(symbol.find(" = ")){
						prov.insert(factory.parse(kind, packDeps[y].at(0)));	
					}else{
						prov.insert(factory.parse(kind, packDeps[y].at(0)
						           , rel, Edition(ed)));
					}

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

		NVRAD nvPkg(packIDs[i].at(1), Edition(edition), Arch("i386"), deps);

		Package::Ptr p( detail::makeResolvableAndImpl(nvPkg, pkg));		

		//set Status to install
		/*for(unsigned int ii = 0;  ii < pToInst.size(); ii++){
			if(pToInst.at(ii) == packIDs[i].at(1)){
				PoolItem_Ref poolItem(p);
				poolItem.status().setToBeInstalled(ResStatus::USER);
			}
		}*/

		store.insert(p);
      return 1;
	}

	dbDeps->close();
	dbPackages->close();

}

ResStore PdbToZypp::getStore(){
	return store;
}
