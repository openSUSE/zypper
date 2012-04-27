/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/**
 * \file zypp/url/UrlUtils.cc
 */
#include "zypp/base/Gettext.h"
#include "zypp/base/String.h"
#include "zypp/url/UrlUtils.h"

#include <stdlib.h>   // strtol
#include <cctype>     // isxdigit
#include <stdexcept>


//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
  namespace url
  { //////////////////////////////////////////////////////////////////


    // ---------------------------------------------------------------
    std::string
    encode(const std::string &str, const std::string &safe,
                                   EEncoding         eflag)
    {
      std::string skip("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                       "abcdefghijklmnopqrstuvwxyz"
                       "0123456789.~_-");
      std::string more(":/?#[]@!$&'()*+,;=");
      size_t      beg, pos, len;
      std::string out;

      for(size_t i=0; i<safe.size(); i++)
      {
        if( more.find(safe.at(i)) != std::string::npos)
          skip.append(1, safe.at(i));
      }

      len = str.length();
      beg = 0;
      while( beg < len)
      {
        pos = str.find_first_not_of(skip, beg);
        if(pos != std::string::npos)
        {
          if( pos > beg)
          {
            out.append(str, beg, pos - beg);
          }

          if( eflag == E_ENCODED &&
              pos + 2 < len      &&
              str.at(pos) == '%' &&
              std::isxdigit(str.at(pos + 1)) &&
              std::isxdigit(str.at(pos + 2)))
          {
            out.append(str, pos, 3);
            beg = pos + 3;
          }
          else
          {
            out.append( encode_octet( str.at(pos)));
            beg = pos + 1;
          }
        }
        else
        {
          out.append(str, beg, len - beg);
          beg = len;
        }
      }
      return out;
    }


    // ---------------------------------------------------------------
    std::string
    decode(const std::string &str, bool allowNUL)
    {
      size_t      pos, end, len;
      std::string out(str);

      len = out.length();
      pos = end = 0;
      while(pos < len)
      {
        out[end] = out[pos];
        if( pos + 2 < len && out.at(pos) == '%')
        {
          int c = decode_octet(out.c_str() + pos + 1);
          switch(c)
          {
            case -1:
              // not a hex noted octet...
            break;

            case 0:
              // is a %00 octet allowed ?
              if( !allowNUL)
              {
                ZYPP_THROW(UrlDecodingException(
                  _("Encoded string contains a NUL byte")
                ));
              }
            default:
              // other octets are fine...
              out[end] = c;
              pos += 2;
            break;
          }
        }
        pos++;
        end++;
      }
      if( end < pos)
        out.erase(end);
      return out;
    }


    // ---------------------------------------------------------------
    std::string
    encode_octet(const unsigned char c)
    {
      static const unsigned char tab[] = "0123456789ABCDEF";
      unsigned char      out[4];

      out[0] = '%';
      out[1] = tab[0x0f & (c >> 4)];
      out[2] = tab[0x0f & c];
      out[3] = '\0';

      //snprintf(out, sizeof(out), "%%%02X", c);
      return std::string((char *)out);
    }


    // ---------------------------------------------------------------
    int
    decode_octet(const char *hex)
    {
      if(hex && std::isxdigit(hex[0]) && std::isxdigit(hex[1]))
      {
        char x[3] = { hex[0], hex[1], '\0'};
        return 0xff & ::strtol(x, NULL, 16);
      }
      else
      {
        return -1;
      }
    }


    // ---------------------------------------------------------------
    void
    split(ParamVec          &pvec,
          const std::string &pstr,
	        const std::string &psep)
    {
      size_t beg, pos, len;
      if( psep.empty())
      {
        ZYPP_THROW(UrlNotSupportedException(
          _("Invalid parameter array split separator character")
        ));
      }

      len = pstr.length();
      beg = 0;

      while( beg < len)
      {
        pos = pstr.find(psep, beg);
        if(pos != std::string::npos)
        {
          pvec.push_back( pstr.substr(beg, pos - beg));
          beg = pos + 1;
        }
        else
        {
          pvec.push_back( pstr.substr(beg, len - beg));
          beg = len;
        }
      }
    }


    // ---------------------------------------------------------------
    void
    split(ParamMap          &pmap,
          const std::string &str,
          const std::string &psep,
          const std::string &vsep,
          EEncoding         eflag)
    {
      ParamVec                 pvec;
      ParamVec::const_iterator pitr;
      std::string              k, v;
      size_t                   pos;

      if( psep.empty() || vsep.empty())
      {
        ZYPP_THROW(UrlNotSupportedException(
          _("Invalid parameter map split separator character")
        ));
      }

      split(pvec, str, psep);

      for( pitr = pvec.begin(); pitr != pvec.end(); ++pitr)
      {
        pos = pitr->find(vsep);
        if(pos != std::string::npos)
        {
          if( eflag == E_DECODED)
          {
            k = url::decode(pitr->substr(0, pos));
            v = url::decode(pitr->substr(pos + 1));
            pmap[ k ] = v;
          }
          else
          {
            k = pitr->substr(0, pos);
            v = pitr->substr(pos + 1);
            pmap[ k ] = v;
          }
        }
        else
        {
          if( eflag == E_DECODED)
          {
            pmap[ url::decode(*pitr) ] = "";
          }
          else
          {
            pmap[ *pitr ] = "";
          }
        }
      }
    }


    // ---------------------------------------------------------------
    std::string
    join(const ParamVec    &pvec,
         const std::string &psep)
    {
      std::string    str;
      ParamVec::const_iterator i( pvec.begin());

      if( i != pvec.end())
      {
        str = *i;
        while( ++i != pvec.end())
        {
          str += psep + *i;
        }
      }

      return str;
    }


    // ---------------------------------------------------------------
    std::string
    join(const ParamMap    &pmap,
         const std::string &psep,
         const std::string &vsep,
         const std::string &safe)
    {
      if( psep.empty() || vsep.empty())
      {
        ZYPP_THROW(UrlNotSupportedException(
          _("Invalid parameter array join separator character")
        ));
      }

      std::string join_safe;
      for(std::string::size_type i=0; i<safe.size(); i++)
      {
        if( psep.find(safe[i]) == std::string::npos &&
            vsep.find(safe[i]) == std::string::npos)
        {
          join_safe.append(1, safe[i]);
        }
      }
      std::string              str;
      ParamMap::const_iterator i( pmap.begin());

      if( i != pmap.end())
      {
        str = encode(i->first, join_safe);
        if( !i->second.empty())
          str += vsep + encode(i->second, join_safe);

        while( ++i != pmap.end())
        {
          str += psep + encode(i->first, join_safe);
          if( !i->second.empty())
            str +=  vsep + encode(i->second, join_safe);
        }
      }

      return str;
    }


    //////////////////////////////////////////////////////////////////
  } // namespace url
  ////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/
