/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <wchar.h>
#include <cstring>
#include <sstream>

#include "utils/text.h"

using namespace std;

// A non-ASCII string has 3 different lengths:
// - bytes
// - characters (non-ASCII ones have multiple bytes in UTF-8)
// - columns (Chinese characters are 2 columns wide)
// In #328918 see how confusing these leads to misalignment.

// return the number of columns in str, or -1 if there's an error
static
int string_to_columns_e (const string & str)
{
  // from smpppd.src.rpm/format.cc, thanks arvin

  const char* ptr = str.c_str ();
  size_t s_bytes = str.length ();
  int s_cols = 0;

  mbstate_t shift_state;
  memset (&shift_state, 0, sizeof (shift_state));

  wchar_t wc;
  size_t c_bytes;

  // mbrtowc produces one wide character from a multibyte string
  while ((c_bytes = mbrtowc (&wc, ptr, s_bytes, &shift_state)) > 0)
  {
    if (c_bytes >= (size_t) -2) // incomplete (-2) or invalid (-1) sequence
      return -1;

    s_cols += wcwidth (wc);

    s_bytes -= c_bytes;
    ptr += c_bytes;
  }

  return s_cols;
}

unsigned string_to_columns (const string& str)
{
  int c = string_to_columns_e (str);
  if (c < 0)
    return str.length();        // fallback if there was an error
  else
    return (unsigned) c;
}


void wrap_text(ostream & out, const string & text,
    unsigned indent, unsigned wrap_width, int initial)
{
  const char * s = text.c_str();
  size_t s_bytes = text.length ();
  const char * prevwp = s;
  const char * linep = s;
  wchar_t wc;
  size_t bytes_read;
  bool in_word = false;

  mbstate_t shift_state;
  memset (&shift_state, 0, sizeof (shift_state));

  unsigned col = 0;
  unsigned toindent = initial < 0 ? indent : initial;
  //wchar_t ws[2] = L" ";
  do
  {
    // indentation
    if (!col)
    {
      out << string(toindent, ' ');
      col = toindent;
    }

    bytes_read = mbrtowc (&wc, s, s_bytes, &shift_state);
    if (bytes_read > 0)
    {
      col += ::wcwidth(wc);

      if (::iswspace(wc))
        in_word = false;
      //else if (::wcscmp(wc, L"\n"))
      //{
      //  if (!in_word)
      //    prevwp = s;
      //  in_word = true;
      //}
      else
      {
        if (!in_word)
          prevwp = s;
        in_word = true;
      }

      // current wc exceeded the wrap width
      if (col > wrap_width)
      {
        // update the size of the string to read
        s_bytes -= (prevwp - linep);
        // print the line, leave linep point to the start of the last word.
        for (; linep < prevwp; ++linep)
          out << *linep;
        out << endl;
        // reset column counter
        col = 0;
        toindent = indent;
        // reset original text pointer (points to current wc, not the start of the word)
        s = linep;
        // reset shift state
        ::memset (&shift_state, 0, sizeof (shift_state));
      }
      else
        s += bytes_read;
    }
    // we're at the end of the string
    else if (bytes_read == 0)
    {
      // print the rest of the text
      for(; *linep; ++linep)
        out << *linep;
    }
    else
    {
      out << endl << "WCHAR ERROR" << endl;
      return;
    }
  }
  while(bytes_read > 0);
}
