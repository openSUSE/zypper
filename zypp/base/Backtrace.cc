/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/Backtrace.cc
 */
#include <execinfo.h>
#include <cxxabi.h>

#include <iostream>
#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/Backtrace.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  std::ostream & dumpBacktrace( std::ostream & stream_r )
  {
    // get void*'s for all entries on the stack
    static const size_t arraySize = 50;
    void *array[arraySize];
    size_t size = ::backtrace( array, arraySize );

    // Print out all frames to stderr. Separate sigsegvHandler stack
    // [dumpBacktrace][sigsegvHandler][libc throwing] from actual
    // code stack.
    char ** messages = ::backtrace_symbols( array, size );
    if ( messages )
    {
      static const size_t handlerStack = 3; // [dumpBacktrace][sigsegvHandler][libc throwing]
      static const size_t first = 0;
      for ( size_t i = first; i < size; ++i )
      {
	char * mangled_name = 0;
	char * offset_begin = 0;
	char * offset_end = 0;

	// find parentheses and +address offset surrounding mangled name
	for ( char * p = messages[i]; *p; ++p )
	{
	  if ( *p == '(' )
	  {
	    mangled_name = p;
	  }
	  else if ( *p == '+' )
	  {
	    offset_begin = p;
	  }
	  else if ( *p == ')' )
	  {
	    offset_end = p;
	    break;
	  }
	}

	int btLevel = i-handlerStack; // negative level in sigsegvHandler
	if ( i > first )
	{
	  stream_r << endl;
	  if ( btLevel == 0 )
	    stream_r << "vvvvvvvvvv----------------------------------------" << endl;
	}
	stream_r << "[" << (btLevel<0 ?"hd":"bt") << "]: (" << btLevel << ") ";

	// if the line could be processed, attempt to demangle the symbol
	if ( mangled_name && offset_begin && offset_end && mangled_name < offset_begin )
	{
	  *mangled_name++ = '\0';
	  *offset_begin++ = '\0';
	  *offset_end++ = '\0';

	  int status;
	  char * real_name = ::abi::__cxa_demangle( mangled_name, 0, 0, &status );

	  // if demangling is successful, output the demangled function name
	  if ( status == 0 )
	  {
	    stream_r << messages[i] << " : " << real_name << "+" << offset_begin << offset_end;
	  }
	  // otherwise, output the mangled function name
	  else
	  {
	    stream_r << messages[i] << " : " << mangled_name << "+" << offset_begin << offset_end;
	  }
	  ::free( real_name );
	}
	else
	{
	  // otherwise, print the whole line
	  stream_r << messages[i];
	}
      }
      ::free( messages );
    }
    return stream_r;
  }

} // namespace zypp
///////////////////////////////////////////////////////////////////
