/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file zypp/url/UrlException.h
 */
#ifndef   ZYPP_URL_URLEXCEPTION_H
#define   ZYPP_URL_URLEXCEPTION_H

#include "zypp/base/Exception.h"


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace url
  { //////////////////////////////////////////////////////////////////


    // ---------------------------------------------------------------
    /**
     * Base class for all URL exceptions.
     */
    class UrlException: public zypp::Exception
    {
    public:
      UrlException()
        : zypp::Exception("Url exception")
      {}

      UrlException(const std::string &msg)
        : zypp::Exception(msg)
      {}

      virtual ~UrlException() throw() {};
    };

    // ---------------------------------------------------------------
    /**
     * Thrown if the encoded string contains a NUL byte (%00).
     */
    class UrlDecodingException: public UrlException
    {
    public:
      UrlDecodingException()
        : UrlException("Url NUL decoding exception")
      {}

      UrlDecodingException(const std::string &msg)
        : UrlException(msg)
      {}

      virtual ~UrlDecodingException() throw() {};
    };

    // ---------------------------------------------------------------
    /**
     * Thrown if the url or a component can't be parsed at all.
     */
    class UrlParsingException: public UrlException
    {
    public:
      UrlParsingException()
        : UrlException("Url parsing failure exception")
      {}

      UrlParsingException(const std::string &msg)
        : UrlException(msg)
      {}

      virtual ~UrlParsingException() throw() {};
    };

    // ---------------------------------------------------------------
    /**
     * Thrown if a url component is invalid.
     */
    class UrlBadComponentException: public UrlException
    {
    public:
      UrlBadComponentException()
        : UrlException("Url bad component exception")
      {}

      UrlBadComponentException(const std::string &msg)
        : UrlException(msg)
      {}

      virtual ~UrlBadComponentException() throw() {};
    };


    // ---------------------------------------------------------------
    /**
     * Thrown if scheme does not allow a component.
     */
    class UrlNotAllowedException: public UrlException
    {
    public:
      UrlNotAllowedException()
        : UrlException("Url not allowed component exception")
      {}

      UrlNotAllowedException(const std::string &msg)
        : UrlException(msg)
      {}

      virtual ~UrlNotAllowedException() throw() {};
    };


    // ---------------------------------------------------------------
    /**
     * Thrown if a feature e.g. parsing of a component
     * is not supported for the url/scheme.
     */
    class UrlNotSupportedException: public UrlException
    {
    public:
      UrlNotSupportedException()
        : UrlException("Url parsing unsupported exception")
      {}

      UrlNotSupportedException(const std::string &msg)
        : UrlException(msg)
      {}

      virtual ~UrlNotSupportedException() throw() {};
    };


    //////////////////////////////////////////////////////////////////
  } // namespace url
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////

#endif /* ZYPP_URL_URLEXCEPTION_H */
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
