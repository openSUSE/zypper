# zypp/base

INCLUDE_DIRECTORIES(  ${CMAKE_SOURCE_DIR}  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

SET (localedir, "${CMAKE_INSTALL_PREFIX}/share/locale")

ADD_DEFINITIONS( -DLOCALEDIR=\\\"${localedir}\\\" )
ADD_DEFINITIONS( -DTEXTDOMAIN="\\\"zypp\\\"" )

SET( zypp_base_SRCS
Measure.cc
Fd.cc
Gettext.cc
GzStream.cc
IOStream.cc
InputStream.cc
ReferenceCounted.cc
String.cc
Unit.cc
ExternalDataSource.cc
Exception.cc
Sysconfig.cc
ProfilingFormater.cc
LogControl.cc
)
ADD_LIBRARY(zypp_base STATIC ${zypp_base_SRCS})

SET_SOURCE_FILES_PROPERTIES(${zypp_base_SRCS} PROPERTIES COMPILE_FLAGS -DZYPP_BASE_LOGGER_LOGGROUP=\\\"base\\\")

INSTALL_FILES( FILES
Algorithm.h
Counter.h
Debug.h
DefaultFalseBool.h
DefaultIntegral.h
Deprecated.h
Exception.h
ExternalDataSource.h
Fd.h
Functional.h
Function.h
Gettext.h
GzStream.h
InputStream.h
IOStream.h
Iterator.h
KindOf.h
LogControl.h
Logger.h
LogTools.h
Measure.h
NonCopyable.h
ProfilingFormater.h
ProvideNumericId.h
PtrTypes.h
ReferenceCounted.h
SafeBool.h
String.h
Sysconfig.h
Unit.h
DESTINATION
${CMAKE_INSTALL_PREFIX}/include/zypp/base
)