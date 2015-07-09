/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <cstring>
#include <boost/utility/string_ref.hpp>
#include "utils/text.h"

using namespace std;

namespace {
  inline int wcwidth_without_ctrlseq(wchar_t wc, bool &in_ctrlseq)
  {
    // ignore the length of terminal control sequences in order
    // to compute the length of colored text correctly
    if (!in_ctrlseq && ::wcsncmp(&wc, L"\033", 1) == 0)
      in_ctrlseq = true;
    else if (in_ctrlseq && ::wcsncmp(&wc, L"m", 1) == 0)
      in_ctrlseq = false;
    else if (!in_ctrlseq)
      return ::wcwidth(wc);
    return 0;
  }
}

// A non-ASCII string has 3 different lengths:
// - bytes
// - characters (non-ASCII ones have multiple bytes in UTF-8)
// - columns (Chinese characters are 2 columns wide)
// In #328918 see how confusing these leads to misalignment.

// return the number of columns in str, or -1 if there's an error
static
int mbs_width_e (const string & str)
{
  // from smpppd.src.rpm/format.cc, thanks arvin

  const char* ptr = str.c_str ();
  size_t s_bytes = str.length ();
  int s_cols = 0;
  bool in_ctrlseq = false;

  mbstate_t shift_state;
  memset (&shift_state, 0, sizeof (shift_state));

  wchar_t wc;
  size_t c_bytes;

  // mbrtowc produces one wide character from a multibyte string
  while ((c_bytes = mbrtowc (&wc, ptr, s_bytes, &shift_state)) > 0)
  {
    if (c_bytes >= (size_t) -2) // incomplete (-2) or invalid (-1) sequence
      return -1;

    s_cols += wcwidth_without_ctrlseq(wc, in_ctrlseq);

    s_bytes -= c_bytes;
    ptr += c_bytes;
  }

  return s_cols;
}

unsigned mbs_width (const string& str)
{
  int c = mbs_width_e(str);
  if (c < 0)
    return str.length();        // fallback if there was an error
  else
    return (unsigned) c;
}

// ---------------------------------------------------------------------------

std::string mbs_substr_by_width(
    const std::string & str,
    std::string::size_type pos,
    std::string::size_type n)
{
  if (n == 0)
    return string();

  const char * ptr = str.c_str();
  const char * sptr = NULL;
  const char * eptr = NULL;
  size_t s_bytes = str.length();
  int s_cols = 0, s_cols_prev;
  bool in_ctrlseq = false;

  mbstate_t shift_state;
  memset (&shift_state, 0, sizeof (shift_state));

  wchar_t wc;
  size_t c_bytes;

  // mbrtowc produces one wide character from a multibyte string
  while ((c_bytes = mbrtowc (&wc, ptr, s_bytes, &shift_state)) > 0)
  {
    if (c_bytes >= (size_t) -2) // incomplete (-2) or invalid (-1) sequence
      return str.substr(pos, n); // default to normal string substr

    s_cols_prev = s_cols;
    s_cols += wcwidth_without_ctrlseq(wc, in_ctrlseq);

    // mark the beginning
    if (sptr == NULL && (unsigned) s_cols >= pos)
    {
      // cut at the right column, include also the current character
      if ((unsigned) s_cols_prev == pos)
        sptr = ptr;
      // current character cut into pieces, don't include it
      else
        sptr = ptr + c_bytes;
    }
    // mark the end
    if (n != string::npos && (unsigned) s_cols >= pos + n)
    {
      // cut at the right column, include also the current character
      if ((unsigned) s_cols == pos + n)
        eptr = ptr + c_bytes;
      // current character cut into pieces, don't include it
      else
        eptr = ptr;
      break;
    }

    s_bytes -= c_bytes;
    ptr += c_bytes;
  }

  if (eptr == NULL)
    eptr = ptr;

  if (eptr == sptr)
    return string();
  return string(sptr, eptr - sptr);
}

// ---------------------------------------------------------------------------
