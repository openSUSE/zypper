// ============================================================================
// gzstream, C++ iostream classes wrapping the zlib compression library.
// Copyright (C) 2001  Deepak Bandyopadhyay, Lutz Kettner
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// ============================================================================
//
// File          : test_gunzip.C
// Revision      : $Revision: 1.3 $
// Revision_date : $Date: 2001/10/04 15:09:28 $
// Author(s)     : Deepak Bandyopadhyay, Lutz Kettner
// 
// Short test program reading a file, uncompressing it, and writing it.
// ============================================================================

#include <gzstream.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>

int main( int argc, char*argv[]) {
    if ( argc != 3) {
	std::cerr << "Usage: " << argv[0] <<" <in-file> <out-file>\n";
	return EXIT_FAILURE;
    }
    // check alternate way of opening file
    igzstream    in2;
    in2.open( argv[1]);
    if ( ! in2.good()) {
        std::cerr << "ERROR: Opening file `" << argv[1] << "' failed.\n";
	return EXIT_FAILURE;
    }
    in2.close();
    if ( ! in2.good()) {
        std::cerr << "ERROR: Closing file `" << argv[1] << "' failed.\n";
	return EXIT_FAILURE;
    }
    // now use the shorter way with the constructor to open the same file
    igzstream in(  argv[1]);
    if ( ! in.good()) {
        std::cerr << "ERROR: Opening file `" << argv[1] << "' failed.\n";
	return EXIT_FAILURE;
    }
    std::ofstream  out( argv[2]);
    if ( ! out.good()) {
        std::cerr << "ERROR: Opening file `" << argv[2] << "' failed.\n";
	return EXIT_FAILURE;
    }
    char c;
    while ( in.get(c))
	out << c;
    in.close();
    out.close();
    if ( ! in.eof()) {
        std::cerr << "ERROR: Reading file `" << argv[1] << "' failed.\n";
	return EXIT_FAILURE;
    }
    if ( ! out.good()) {
        std::cerr << "ERROR: Writing file `" << argv[2] << "' failed.\n";
	return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// ============================================================================
// EOF
