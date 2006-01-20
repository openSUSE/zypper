// Capabilities.cc
//
// tests for Capabilities 
//
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/CapFactory.h"

using namespace std;
using namespace zypp;

int main( int argc, char * argv[] )
{
    Resolvable::Kind kind = ResTraits<zypp::Package>::kind;
    CapFactory factory;

    Edition edition ("1.0", "42");
    Capability cap = factory.parse ( kind, "foo", "=", "1.0-42");
    if (cap.asString() != "foo == 1.0-42") return 1;
    if (cap.index() != "foo") return 2;
    if (cap.op() != Rel::EQ) return 3;
    if (cap.edition() != edition) return 4;

    Capability cap2 = factory.parse ( kind, "foo", Rel::EQ, edition);
    if (cap2.index() != cap.index()) return 10;
    if (cap2.op() != cap.op()) return 11;
    if (cap2.edition() != cap.edition()) return 12;

    Capability cap3 = factory.parse ( kind, "foo = 1.0-42");
    if (cap3.index() != cap.index()) return 20;
    if (cap3.op() != cap.op()) return 21;
    if (cap3.edition() != cap.edition()) return 22;

    string bash = "/bin/bash";
    Capability cap4 = factory.parse ( kind, bash);
    if (cap4.index() != bash) return 30;
    if (cap4.op() != Rel::NONE) return 31;
    if (cap4.edition() != Edition::noedition) return 32;

    string hal = "hal(smp)";
    Capability cap5 = factory.parse ( kind, hal);
    if (cap5.index() != hal) return 40;
    if (cap5.op() != Rel::NONE) return 41;
    if (cap5.edition() != Edition::noedition) return 42;

    return 0;
}
