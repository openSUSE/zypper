/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PurgeKernels.h
 *
*/

#include <zypp/PoolItem.h>
#include <zypp/base/PtrTypes.h>

namespace zypp {

  namespace str {
  class regex;
  }

  /*!
   * Implements the logic of the "purge-kernels" command.
   *
   */
  class PurgeKernels
  {
  public:
    PurgeKernels();


    /*!
     * Marks all currently obsolete Kernels according to the keep spec.
     * \note This will not commit the changes
     */
    void markObsoleteKernels();

    /*!
     * Force a specific uname to be set, only used for testing,
     * in production the running kernel is detected.
     */
    void setUnameR( const std::string &val );
    std::string unameR() const;


    /*!
     * Force a specific kernel arch to be set, only used for testing,
     * in production the running kernel arch is detected.
     */
    void setKernelArch( const zypp::Arch &arch );
    Arch kernelArch() const;

    /*!
       * Overrides the keep spec, the default value is read from ZConfig.
       * The keep spec is a string of tokens seperated by ",".
       * It only supports 3 different tokens:
       *  - "running" matches only the currently running kernel of the system
       *  - "oldest"  matches the kernel version for each flavour/arch combination with the lowest edition
       *              can be modified with a positive number:  oldest+n
       *  - "latest"  matches the kernel version for each flavour/arch combination with the highest edition
       *              can be modified with a negative number:  latest-n
       */
    void setKeepSpec( const std::string &val );
    std::string keepSpec () const;

    struct Impl;
  private:
    RW_pointer<Impl> _pimpl;
  };

}

