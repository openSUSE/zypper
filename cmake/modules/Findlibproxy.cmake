# The "real" libproxy provides its own Findlibproxy.cmake but saner, simpler
# alternatives like the PacRunner replacement which *just* queries PacRunner
# directly will only provide a .pc file. So use pkg-config to find it...

INCLUDE ( FindPkgConfig )

PKG_SEARCH_MODULE( LIBPROXY libproxy-1.0 )
