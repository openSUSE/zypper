INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(hal_SRCS
hal.cc
)

KDE3_AUTOMOC(${hal_SRCS})

KDE3_ADD_EXECUTABLE(hal ${hal_SRCS})

TARGET_LINK_LIBRARIES(hal  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(librpmDb_SRCS
librpmDb.cc
)

KDE3_AUTOMOC(${librpmDb_SRCS})

KDE3_ADD_EXECUTABLE(librpmDb ${librpmDb_SRCS})

TARGET_LINK_LIBRARIES(librpmDb  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(RpmDb_SRCS
RpmDb.cc
)

KDE3_AUTOMOC(${RpmDb_SRCS})

KDE3_ADD_EXECUTABLE(RpmDb ${RpmDb_SRCS})

TARGET_LINK_LIBRARIES(RpmDb  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(storagetargettest_SRCS
storagetargettest.cc
Benchmark.cc
Benchmark.h
)

KDE3_AUTOMOC(${storagetargettest_SRCS})

KDE3_ADD_EXECUTABLE(storagetargettest ${storagetargettest_SRCS})

TARGET_LINK_LIBRARIES(storagetargettest  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(import_old_sources_test_SRCS
import_old_sources_test.cc
)

KDE3_AUTOMOC(${import_old_sources_test_SRCS})

KDE3_ADD_EXECUTABLE(import_old_sources_test ${import_old_sources_test_SRCS})

TARGET_LINK_LIBRARIES(import_old_sources_test  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

SET(dumpstore_SRCS
dumpstore.cc
)

KDE3_AUTOMOC(${dumpstore_SRCS})

KDE3_ADD_EXECUTABLE(dumpstore ${dumpstore_SRCS})

TARGET_LINK_LIBRARIES(dumpstore  ${QT_AND_KDECORE_LIBS} )


########### install files ###############




#original Makefile.am contents follow:

##
## Makefile.am for testsuite/target/tests
##
#
#LDADD =	$(top_srcdir)/zypp/lib@PACKAGE@.la $(top_srcdir)/testsuite/src/utils/lib@PACKAGE@_testsuite_utils.la
#
#noinst_PROGRAMS = hal librpmDb RpmDb storagetargettest import_old_sources_test dumpstore
#
#hal_SOURCES = hal.cc
#
#librpmDb_SOURCES = librpmDb.cc 
#RpmDb_SOURCES = RpmDb.cc
#
#storagetargettest_SOURCES = storagetargettest.cc Benchmark.cc Benchmark.h
#dumpstore_SOURCES = dumpstore.cc
#
#import_old_sources_test_SOURCES = import_old_sources_test.cc
