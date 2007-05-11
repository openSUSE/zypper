#include "resolvable.h"

resolvable::resolvable(string _name, string _kind, string _arch, string _version, string _release){
		name = _name;
		kind = _kind;
		arch = _arch;
		version = _version;
		release = _release;
}

resolvable::~resolvable(){
}

/*
void resolvable::addDep(depType _type, string _dep){

		if(deps.find(_type) == deps.end()){
				std::vector<string> temp;
				deps[_type] = temp;
		}		
		
		deps[_type].push_back(_dep);

}
*/
