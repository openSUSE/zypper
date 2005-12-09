/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file zypp/url/UrlUtils.h
 */
#ifndef   ZYPP_URL_URLUTILS_H
#define   ZYPP_URL_URLUTILS_H

#include <string>
#include <vector>
#include <map>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace url
  { //////////////////////////////////////////////////////////////////

    // ---------------------------------------------------------------
    typedef std::vector < std::string >             ParamVec;
    typedef std::map < std::string, std::string >   ParamMap;

    typedef enum { E_ENCODED, E_DECODED }           EEncoding;

    /**
     * \brief Encodes a string using URL percent encoding.
     *
     * Already encoded characters are detected and will be not encoded a
     * second time. By default, all characters except of "a-zA-Z0-9_.-"
     * will be encoded. Additional characters can be specified in the
     * \p safe argument.
     *
     * The following function call will encode the "@" character as "%40",
     * but skip encoding of the "%" character:
     * @code
     *   zypp::url::encode("foo%bar@localhost", "%");
     * @endcode
     *
     * \param str      A string to encode (binary data).
     * \param safe     Characters safe to skip in encoding,
     *                 e.g. "/" for path names.
     * \return A percend encoded string.
     */
    std::string
    encode(const std::string &str, const std::string &safe = "");

    /**
     * @param str      A string to decode.
     * @param allowNUL If %00 (binary NUL) is allowed or not.
     * @return A decoded (binary data) string.
     * @throws A std::invalid_argument exception if allowNUL
     *         is false and a %00 was found.
     */
    std::string
    decode(const std::string &str, bool allowNUL = false);

    /**
     * @param c        A octet / character to encode.
     * @return A percent encoded representation of the octet,
     *         e.g. %20 for space.
     */
    std::string
    encode_octet(const unsigned char c);

    /**
     * @param hex     Pointer to two hex characters representing
     *                the octet value in percent-encoded strings.
     * @return The octet value 0-255 or -1 for invalid hex argument.
     */
    int
    decode_octet(const char *hex);


    /**
     * FIXME: split vec
     */
    void
    split(ParamVec          &pvec,
          const std::string &pstr,
          const std::string &psep);

    /**
     * FIXME: split map
     */
    void
    split(ParamMap          &pmap,
          const std::string &pstr,
          const std::string &psep,
          const std::string &vsep,
          EEncoding         eflag = E_ENCODED);

    /**
     * FIXME: join vec
     */
    std::string
    join(const ParamVec     &pvec,
         const std::string  &psep);

    /**
     * FIXME: join map
     */
    std::string
    join(const ParamMap     &pmap,
         const std::string  &psep,
         const std::string  &vsep,
         const std::string  &safe);


    //////////////////////////////////////////////////////////////////
  } // namespace url
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif /* ZYPP_URL_URLUTILS_H */
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
