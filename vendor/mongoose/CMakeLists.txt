ADD_DEFINITIONS( -DNO_CGI -D_POSIX_SOURCE -D_BSD_SOURCE )
SET(mongoose_SOURCES
 mongoose.c
 mongoose.h
)

ADD_LIBRARY(mongoose ${mongoose_SOURCES})
TARGET_LINK_LIBRARIES(mongoose dl pthread)

