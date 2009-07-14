#include <iostream>

#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/PathInfo.h>
#include <zypp/Capability.h>
#include <zypp/sat/Solvable.h>
#include <zypp/sat/WhatProvides.h>

int main(int argc, char **argv) {
       zypp::ZYpp::Ptr zyppPtr = zypp::ZYppFactory::instance().getZYpp();

       zypp::Pathname sysRoot( "/" );

       zyppPtr->initializeTarget( sysRoot, false );
       zyppPtr->target()->load();

       std::cout << "Looking for packages which provide " << argv[1] << std::endl;
       zypp::Capability cap(argv[1]);
       zypp::sat::WhatProvides wp(cap);

       if (wp.empty()) {
               std::cout << "No providers of " << argv[1] << " found" << std::endl;
       } else {
               zypp::sat::Solvable package(*wp.begin());
               std::cout << "Provided by " << package.name() << " version " << package.edition().version()
                       << std::endl;
       }

       return EXIT_SUCCESS;
}
