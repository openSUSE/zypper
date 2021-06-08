
# to find the KeyRingTest receiver
INCLUDE_DIRECTORIES( ${LIBZYPP_SOURCE_DIR}/tests/zypp )

ADD_TESTS(
  Blacklisted
  IdString
  LookupAttr
  Pool
  Queue
  Map
  Solvable
  SolvableSpec
  SolvParsing
  WhatObsoletes
  WhatProvides
)
