/*
** Zlib for sqlite3
**
** Compile: gcc -o zlib.so -shared zlibext.c -lsqlite3 -lz
**
** based on sqaux code from	James P. Lyon
** ported to sqlite3 by Duncan Mac-Vicar
**
** The authors disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <assert.h>

#include <sqlite3ext.h>
#include <zlib.h>

#include <sqlite3ext.h>
SQLITE_EXTENSION_INIT1

/*
** Compute the maximum size required by sqlite_encode_binary().
** This doesn't include the NUL byte which terminates the string.
*/
int sqaux_encode_maxsize(int datasize)
{
	int nEncMax = (256*datasize + 1262)/253;
	return nEncMax;
}

/*
** Encode a binary buffer "in" of size n bytes so that it contains
** no instances of characters '\'' or '\000'.  The output is 
** null-terminated and can be used as a string value in an INSERT
** or UPDATE statement.  Use sqlite_decode_binary() to convert the
** string back into its original binary.
**
** The result is written into a preallocated output buffer "out".
** "out" must be able to hold at least 2 +(257*n)/254 bytes.
** In other words, the output will be expanded by as much as 3
** bytes for every 254 bytes of input plus 2 bytes of fixed overhead.
** (This is approximately 2 + 1.0118*n or about a 1.2% size increase.)
**
** The return value is the number of characters in the encoded
** string, excluding the "\000" terminator.
*/
int sqlite3_encode_binary(const unsigned char *in, int n, unsigned char *out){
  int i, j, e, m;
  int cnt[256];
  if( n<=0 ){
    out[0] = 'x';
    out[1] = 0;
    return 1;
  }
  memset(cnt, 0, sizeof(cnt));
  for(i=n-1; i>=0; i--){ cnt[in[i]]++; }
  m = n;
  for(i=1; i<256; i++){
    int sum;
    if( i=='\'' ) continue;
    sum = cnt[i] + cnt[(i+1)&0xff] + cnt[(i+'\'')&0xff];
    if( sum<m ){
      m = sum;
      e = i;
      if( m==0 ) break;
    }
  }
  out[0] = e;
  j = 1;
  for(i=0; i<n; i++){
    int c = (in[i] - e)&0xff;
    if( c==0 ){
      out[j++] = 1;
      out[j++] = 1;
    }else if( c==1 ){
      out[j++] = 1;
      out[j++] = 2;
    }else if( c=='\'' ){
      out[j++] = 1;
      out[j++] = 3;
    }else{
      out[j++] = c;
    }
  }
  out[j] = 0;
  return j;
}

/*
** Decode the string "in" into binary data and write it into "out".
** This routine reverses the encoding created by sqlite_encode_binary().
** The output will always be a few bytes less than the input.  The number
** of bytes of output is returned.  If the input is not a well-formed
** encoding, -1 is returned.
**
** The "in" and "out" parameters may point to the same buffer in order
** to decode a string in place.
*/
int sqlite3_decode_binary(const unsigned char *in, unsigned char *out){
  int i, c, e;
  e = *(in++);
  i = 0;
  while( (c = *(in++))!=0 ){
    if( c==1 ){
      c = *(in++);
      if( c==1 ){
        c = 0;
      }else if( c==2 ){
        c = 1;
      }else if( c==3 ){
        c = '\'';
      }else{
        return -1;
      }
    }
    out[i++] = (c + e)&0xff;
  }
  return i;
}


/*
** Compute the adler32 checksum of a string.
** This function is exported from the zlib library.
** Return the checksum as a hex string.
** THIS IS AN SQLITE USER FUNCTION.
**
** argv[0] Data		** encoded data to compute checksum of
*/
void FnAdler32(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	unsigned long checksum;
	char buf[8+1]; /* Buffer to hold 8 hex digits. */

	/* Validate arguments. */
	assert(argc == 1 && argv && argv[0]);

	checksum = adler32(0L, Z_NULL, 0);

	/* Compute the checksum */
	if (argc == 1 && argv && argv[0])
	{
		int len = strlen(sqlite3_value_text(argv[0]));
		checksum = adler32(checksum, (const unsigned char*)argv[0], len);
	}

	/* Convert checksum to (upper-case) hexadecimal string. */
	sprintf(buf, "%08X", checksum);

	/* 'Return' the string.
	** -1 means use entire string.
	*/
	sqlite3_result_text(context, buf, -1, SQLITE_STATIC);
}

/*
** Decode string data stored using ZipString().
** This undoes sqlite binary encoding and zip compression.
** <pData> is the NUL-terminated data string stored in the sqlite database.
** <nSize> is the uncompressed size of the data.
**   If -1 is passed for the size, it will be computed from strlen().
** <pzErrMsg> points to a string pointer, to allow returning an error message.
**   This argument can be NULL. It should not be freed by the caller.
** Returns a pointer to dynamically allocated string.
** Returns NULL on failure.
*/
char* UnzipString(const char* pData, long nSize, const char **pzErrMsg)
{
	long nEncSize, nZipSize;
	long nXmlSize;
	char *pZip, *pXml;
	int zret;

	assert(pData); if (!pData) return NULL;
	assert(nSize >= 0); if (nSize < 0) return NULL;

	/*
	** Set up a buffer to hold the unencoded zip data.
	** This data can contain NUL bytes.
	** This will always no larger in size than the encoded binary data.
	*/
	nEncSize = strlen(pData);
	pZip = (char*)malloc(nEncSize+1);
	if (!pZip)
	{
		if (pzErrMsg)
			*pzErrMsg = "UnzipString: malloc() failure.";
		return NULL;
	}

	/*
	** Decode the sqlite encoding of the zip data.
	** This returns the actual size of the unencoded [zip] data.
	*/
	nZipSize = sqlite3_decode_binary((const unsigned char *)pData, (unsigned char *)pZip);
	if (nZipSize < 0) /* error */
	{
		if (pzErrMsg)
			*pzErrMsg = "UnzipString: sqlite3_decode_binary() failure.";
		free(pZip);
		return NULL;
	}

	/*
	** Set up a buffer to hold the uncompressed data.
	** This data can contain NUL bytes.
	** We allocate extra size just to be safe.
	** NEED to find out how much extra we really need to pad.
	** This will be generally less than 10kB in size.
	*/
	pXml = (char*)malloc(nSize+1);
	if (!pXml)
	{
		if (pzErrMsg)
			*pzErrMsg = "UnzipString: malloc() failure.";
		free(pZip);
		return NULL;
	}

	/* Decompress the data into the XML string. */
	nXmlSize = nSize;
	zret = uncompress((unsigned char*)pXml, (unsigned long*)&nXmlSize, (const unsigned char*)pZip, nZipSize);
	if (zret != Z_OK)
	{
		if (pzErrMsg)
			*pzErrMsg = "UnzipString: uncompress() failure.";
		free(pZip);
		free(pXml);
		return NULL;
	}
	assert(nXmlSize == nSize);

	/* Terminate the string. */
	pXml[nXmlSize] = 0;

	/* Free the zipped data. */
	free(pZip);

	return pXml;
}

/*
** Compress and encode string data to be stored in an sqlite database.
** This does zip compression and sqlite binary encoding on the string.
** <pXml> is the NUL-terminated xml string to be stored in the sqlite Files.Data field.
** <nSize> is the uncompressed size of the xml string. Can be -1 to cause to compute it.
** <pzErrMsg> points to a string pointer, to allow returning an error message.
**   This argument can be NULL. It should not be freed.
** Return pointer to dynamically allocated encoded string.
** Returns NULL on failure.
*/
char* ZipString(const char* pXml, long nXmlSize, const char **pzErrMsg)
{
	unsigned long nZipSize, nZipMax, nDataSize, nDataMax, i;
	char *pZip, *pData;
	int zret;

	assert(pXml); if (!pXml) return NULL;

	/* Compute the size if necessary. */
	if (nXmlSize < 0)
		nXmlSize = strlen(pXml);
	assert((unsigned long)nXmlSize == strlen(pXml));

	/*
	** Set up a buffer to hold the unencoded zip data.
	** This data can contain NUL bytes.
	** This can be larger than the XML data.
	*/
	nZipMax = nXmlSize + nXmlSize/512 + 12;
	pZip = (char*)malloc(nZipMax);
	if (!pZip)
	{
		if (pzErrMsg)
			*pzErrMsg = "ZipString: malloc() failure.";
		return NULL;
	}

	/*
	** Compress the xml data into the zip buffer.
	** This returns the actual size in <nZipSize>.
	*/
	nZipSize = nZipMax;
	zret = compress2((unsigned char*)pZip, &nZipSize, (const unsigned char*)pXml, nXmlSize, Z_BEST_COMPRESSION);
	if (zret != Z_OK)
	{
		if (pzErrMsg)
			*pzErrMsg = "ZipString: compress2() failure.";
		free (pZip);
		return NULL;
	}
	assert(nZipSize <= nZipMax);

	/*
	** Allocate the buffer to hold the encoded binary data.
	** This data will not contain NUL bytes.
	** This buffer will generally be larger than the unencoded data.
	** In general it will be smaller than the size allocated for the zipped data.
	*/
	nDataMax = sqaux_encode_maxsize(nZipSize);
	pData = (char*)malloc(nDataMax+1);
	if (!pData)
	{
		if (pzErrMsg)
			*pzErrMsg = "ZipString: realloc() failure.";
		free(pZip);
		return NULL;
	}

	/*
	** Encode the binary data to convert to a form safe to store in sqlite.
	** This is done in place on the buffer holding the zipped data.
	** The actual size of the encoded data string will be returned.
	*/
	nDataSize = sqlite3_encode_binary((const unsigned char*)pZip, nZipSize, (unsigned char*)pData);
	assert(nDataSize <= nDataMax);

	free(pZip);

	/* Terminate the Data string. */
	pData[nDataSize] = 0;

	/* Return the data string. */
	return pData;
}

/*
** Compress and binary encode a data string for storing in an sqlite database.
** Returns NULL if the argument is NULL.
** THIS IS AN SQLITE USER FUNCTION.
**
** argv[0] = data string
*/
void FnZipString( sqlite3_context *context, int argc, sqlite3_value **argv)
{
	const char *pXml, *zErrMsg;
	char *pEncXml;
	long nSize;

	/* TODO: validate arguments. */
	assert(argc == 1 && argv);

	pXml = sqlite3_value_text(argv[0]);

	/* Handle NULL */
	if (pXml == NULL)
	{
    sqlite3_result_null(context);
		return;
	}

	nSize = strlen(pXml);

	/*
	** If an error occurs, buffer[] will hold the error string.
	** This error message doesn't have to be freed.
	*/
	zErrMsg = NULL;
	pEncXml = ZipString(pXml, nSize, &zErrMsg);
	if (pEncXml)
    sqlite3_result_text(context,  pEncXml, -1, SQLITE_STATIC);
	else
		sqlite3_result_error(context, zErrMsg, -1);

	free(pEncXml);
}

/*
** Uncompress and binary decode a string compressed with [Fn]ZipString().
** Returns NULL if the argument is NULL.
** THIS IS AN SQLITE USER FUNCTION.
**
** argv[0] = compressed data string
*/
void FnUnzipString(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	const char *pZip, *zErrMsg;
	char *pData;
	long nZipSize;

	/* TODO: validate arguments. */
	assert(argc == 1 && argv);

	pZip = sqlite3_value_text(argv[0]);

	/* Handle NULL */
	if (pZip == NULL)
	{
		sqlite3_result_null(context);
		return;
	}

	nZipSize = strlen(pZip);

	/*
	** If an error occurs, buffer[] will hold the error string.
	** This error message doesn't have to be freed.
	*/
	zErrMsg = NULL;
	pData = UnzipString(pZip, nZipSize, &zErrMsg);
	if (pData)
		sqlite3_result_text(context, pData, -1, SQLITE_STATIC);
	else
		sqlite3_result_error(context, zErrMsg, -1);

	free(pData);
}


/************************************************************************/

    /* SQLite invokes this routine once when it loads the extension.
    ** Create new functions, collating sequences, and virtual table
    ** modules here.  This is usually the only exported symbol in
    ** the shared library.
    */
    int sqlite3_extension_init(
      sqlite3 *db,
      char **pzErrMsg,
      const sqlite3_api_routines *pApi
    ){
      SQLITE_EXTENSION_INIT2(pApi)
      sqlite3_create_function(db, "unzip", 1, SQLITE_ANY, 0, FnUnzipString, 0, 0);
      sqlite3_create_function(db, "zip", 1, SQLITE_ANY, 0, FnUnzipString, 0, 0);
      return 0;
    }

