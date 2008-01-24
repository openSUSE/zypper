#ifndef ZYPP_MEDIA_CURLRCONFIG_H_
#define ZYPP_MEDIA_CURLRCONFIG_H_

//#include "zypp/base/NonCopyable.h"
#include "zypp/base/String.h"

namespace zypp
{
  namespace media
  {


  /**
   * Structure holding values of curlrc options.
   */
  struct CurlConfig
  {
  public:
    /**
     * Parse a curlrc file and store the result in the \a config structure.
     * 
     * \param config   a CurlConfig structure
     * \param filename path to the curlrc file. If empty, ~/.curlrc is used.
     * \return         0 on success, 1 if problem occurs.
     */
    static int parseConfig(CurlConfig & config, const std::string & filename = "");

    /**
     * Stores the \a value of the \a option in the \a config structure or
     * logs an unknown option.
     * 
     * \return         0 on success, 1 if problem occurs.
     */
    static int setParameter(CurlConfig & config,
                            const std::string & option,
                            const std::string & value);

  public:
    std::string proxyuserpwd;
    // add more curl config data here as they become needed
  };


  } // namespace media
} // namespace zypp

#endif /*ZYPP_MEDIA_CURLRCONFIG_H_*/
