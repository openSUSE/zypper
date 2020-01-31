#!/usr/bin/sh

if [ "${ZYPP_TEST_USE_SSL}" == "1" ]; then
	echo 'server.modules += ( "mod_openssl" )'
	echo 'ssl.engine = "enable"'
	echo 'ssl.pemfile = ssl_dir + "/server.pem"'
fi
