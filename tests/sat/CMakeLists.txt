
# to find the KeyRingTest receiver
INCLUDE_DIRECTORIES( ${LIBZYPP_SOURCE_DIR}/tests/zypp )

ADD_TESTS(
  AttrMatcher
  IdString
  LookupAttr
  Pool
  Map
  Solvable
  SolvParsing
  WhatObsoletes
  WhatProvides
)
