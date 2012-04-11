
# to find the KeyRingTest receiver
INCLUDE_DIRECTORIES( ${LIBZYPP_SOURCE_DIR}/tests/zypp )

ADD_TESTS(
  IdString
  LookupAttr
  Pool
  Map
  Solvable
  SolvParsing
  WhatObsoletes
  WhatProvides
)
