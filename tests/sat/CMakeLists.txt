
# to find the KeyRingTest receiver
INCLUDE_DIRECTORIES( ${LIBZYPP_SOURCE_DIR}/tests/zypp )

ADD_TESTS(
  AttrMatcher
  IdString
  LookupAttr
  Pool
  Solvable
  SolvParsing
  WhatObsoletes
  WhatProvides
)
