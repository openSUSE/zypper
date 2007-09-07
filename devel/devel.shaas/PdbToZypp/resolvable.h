#include <string>
#include <vector>
#include <map>
#include <zypp/detail/PackageImplIf.h>

#ifndef RESOLVABLE_H
#define RESOLVABLE_H

using std::string;

enum depType{
		REQUIRES,
		PROVIDES,
		OBSOLETES,
		CONFLICTS
};

class resolvable{
	private:
		string name;
		string kind;
		string arch;
		string version;
		string release;
		std::map < depType, std::vector<string> > deps;
	
	public:
		resolvable(string name, string version, string kind = "package", string arch = "i386", string release ="0");
		~resolvable();
		//void addDep(depType type, string dep, string compare, string version);
};

class resolvZypp : public zypp::detail::PackageImplIf {};
#endif
