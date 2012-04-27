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

#include "zypp/url/UrlException.h"

#include <string>
#include <vector>
#include <map>

/** Characters that are safe for URL without percent-encoding. */
#define URL_SAFE_CHARS ":/?#[]@!$&'()*+,;="

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  /** Url details namespace. */
  namespace url
  { //////////////////////////////////////////////////////////////////


    // ---------------------------------------------------------------
    /** A parameter vector container.
     * A string vector containing splited PathParam- or Query-String.
     * Each string in the vector is allways URL percent encoded and
     * usually contains a "key=value" pair.
     */
    typedef std::vector < std::string >             ParamVec;


    /** A parameter map container.
     * A map containing key and value pairs parsed from a PathParam-
     * or Query-String.
     */
    typedef std::map < std::string, std::string >   ParamMap;


    /** Encoding flags.
     */
    typedef enum {
        E_ENCODED, //!< Flag to request encoded string(s).
        E_DECODED  //!< Flag to request decoded string(s).
    } EEncoding;


    // ---------------------------------------------------------------
    /** Encodes a string using URL percent encoding.
     *
     * By default, all characters except of "a-zA-Z0-9_.-" will be encoded.
     * Additional characters from the set ":/?#[]@!$&'()*+,;=", that are
     * safe for a URL compoent without encoding, can be specified in the
     * \p safe argument.
     *
     * If the \p eflag parameter is set to E_ENCODED, then already encoded
     * substrings will be detected and not encoded a second time.
     *
     * The following function call will encode the "@" character as "%40",
     * but skip encoding of the "%" character, because the \p eflag is set
     * to E_ENCODED and "%ba" is detected as a valid encoded character.
     * \code
     *   zypp::url::encode("foo%bar@localhost", "", E_ENCODED);
     * \endcode
     * With \p eflag set to E_DECODED, the "%" character would be encoded
     * as well. The complete encoded string would be "foo%25bar%40localhost".
     *
     * \param str      A string to encode (binary data).
     * \param safe     Characters safe to skip in encoding,
     *                 e.g. "/" for path names.
     * \param eflag    If to detect and skip already encoded substrings.
     * \return A percent encoded string.
     */
    std::string
    encode(const std::string &str, const std::string &safe = "",
                                   EEncoding         eflag = E_DECODED);


    // ---------------------------------------------------------------
    /** Decodes a URL percent encoded string.
     * Replaces all occurences of \c "%<hex><hex>" in the \p str string
     * with the character encoded using the two hexadecimal digits that
     * follows the "%" character.
     *
     * For example, the encoded string "%40%3F%3D%26%25" will be decoded
     * to "@?=&%".
     *
     * \param str      A string to decode.
     * \param allowNUL A flag, if \c "%00" (encoded \c '\\0')
     *                 is allowed or not.
     * \return A decoded strig (may contain binary data).
     * \throws UrlDecodingException if \p allowNUL is false and
     *         a encoded NUL byte (\c "%00") was found in \p str.
     */
    std::string
    decode(const std::string &str, bool allowNUL = false);


    // ---------------------------------------------------------------
    /** Encode one character.
     *
     * Encode the specified character \p c into its \c "%<hex><hex>"
     * representation.
     *
     * \param c        A character to encode.
     * \return A percent encoded representation of the character,
     *         e.g. %20 for a ' ' (space).
     */
    std::string
    encode_octet(const unsigned char c);


    // ---------------------------------------------------------------
    /** Decode one character.
     *
     * Decode the \p hex parameter pointing to (at least) two hexadecimal
     * digits into its character value and return it.
     *
     * Example:
     * \code
     *   char *str = "%40";
     *   char *pct = strchr(str, '%');
     *   int   chr = pct ? decode_octet(pct+1) : -1;
     *      // chr is set to the '@' ASCII character now.
     * \endcode
     *
     * \param hex     Pointer to two hex characters representing
     *                the character value in percent-encoded strings.
     * \return The value (0-255) encoded in the \p hex characters or -1
     *         if \p hex does not point to two hexadecimal characters.
     */
    int
    decode_octet(const char *hex);


    // ---------------------------------------------------------------
    /** Split into a parameter vector.
     *
     * Splits a parameter string \p pstr into substrings using \p psep
     * as separator and appends the resulting substrings to \p pvec.
     *
     * Usual parameter separators are \c '&' for Query- and \c ',' for
     * PathParam-Strings.
     *
     * \param pvec    Reference to a result parameter vector.
     * \param pstr    Reference to the PathParam- or Query-String to split.
     * \param psep    Parameter separator character to split at.
     * \throws UrlNotSupportedException if \p psep separator is empty.
     */
    void
    split(ParamVec          &pvec,
          const std::string &pstr,
          const std::string &psep);


    // ---------------------------------------------------------------
    /** Split into a parameter map.
     *
     * Splits a parameter string \p pstr into substrings using \p psep as
     * separator and then, each substring into key and value pair using
     * \p vsep as separator between parameter key and value and adds them
     * to the parameter map \p pmap.
     *
     * If a parameter substring doesn't contain any value separator \p vsep,
     * the substring is used as a parameter key and value is set to an empty
     * string.
     *
     * Usual parameter separators are \c '&' for Query- and \c ',' for
     * PathParam-Strings. A usual parameter-value separator is \c '=' for
     * both, Query- and PathParam-Strings.
     *
     * If the encoding flag \p eflag is set to \p E_DECODED, then the key
     * and values are dedcoded before they are stored in the map.
     *
     * \param pmap    Reference to a result parameter map.
     * \param pstr    Reference to the PathParam- or Query-String to split.
     * \param psep    Separator character to split key-value pairs.
     * \param vsep    Separator character to split key and value.
     * \param eflag   Flag if the key and value strings should be URL percent
     *                decoded before they're stored in the map.
     * \throws UrlNotSupportedException if \p psep or \p vsep separator
     *         is empty.
     */
    void
    split(ParamMap          &pmap,
          const std::string &pstr,
          const std::string &psep,
          const std::string &vsep,
          EEncoding         eflag = E_ENCODED);


    // ---------------------------------------------------------------
    /** Join parameter vector to a string.
     *
     * Creates a string containing all substrings from the \p pvec separated
     * by \p psep separator character. The substrings in \p pvec should be
     * already URL percent encoded and should't contain \p psep characters.
     *
     * Usual parameter separators are \c '&' for Query- and \c ',' for
     * PathParam-Strings.
     *
     * \param pvec    Reference to encoded parameter vector.
     * \param psep    Parameter separator character to use.
     * \return A parameter string.
     */
    std::string
    join(const ParamVec     &pvec,
         const std::string  &psep);


    // ---------------------------------------------------------------
    /** Join parameter map to a string.
     *
     * Creates a string containing all parameter key-value pairs from the
     * parameter map \p pmap, that will be joined using the \p psep character
     * and the parameter key is separated from the parameter value using the
     * \p vsep character. Both, key and value will be automatically encoded.
     *
     * Usual parameter separators are \c '&' for Query- and \c ',' for
     * PathParam-Strings. A usual parameter-value separator is \c '=' for
     * both, Query- and PathParam-Strings.
     *
     * See encode() function from details about the \p safe characters.
     *
     * \param pmap    Reference to a parameter map.
     * \param psep    Separator character to use between key-value pairs.
     * \param vsep    Separator character to use between keys and values.
     * \param safe    List of characters to accept without encoding.
     * \return A URL percent-encoded parameter string.
     * \throws UrlNotSupportedException if \p psep or \p vsep separator
     *         is empty.
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
