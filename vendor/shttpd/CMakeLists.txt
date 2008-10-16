SET(shttp_SOURCES
 auth.c
 cgi.c
 compat_unix.c
 config.h
 defs.h
 io_cgi.c
 io_dir.c
 io_emb.c
 io_file.c
 io.h
 io_socket.c
 io_ssi.c
 io_ssl.c
 llist.h
 log.c
 md5.c
 md5.h
 shttpd.c
 shttpd.h
 ssl.h
 standalone.c
 std_includes.h
 string.c
)

ADD_LIBRARY(shttp ${shttp_SOURCES})
TARGET_LINK_LIBRARIES(shttp dl pthread)

