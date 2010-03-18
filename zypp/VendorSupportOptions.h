/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/VendorSupportOptions.h
 *
*/
#ifndef ZYPP_VendorSupportOptions_H
#define ZYPP_VendorSupportOptions_H

#include <string>
#include "zypp/base/Flags.h"

namespace zypp
{

    enum VendorSupportOption
    {
      /**
       * The support for this package is unknown
       */
      VendorSupportUnknown     = 0,
      /**
       * The package is known to be unsupported by the vendor
       */
      VendorSupportUnsupported = (1<<0),
      /**
       * Additional Customer Contract necessary
       */
      VendorSupportACC         = (1<<1),
      /**
       * Problem determination, which means technical support
       * designed to provide compatibility information,
       * installation assistance, usage support, on-going maintenance
       * and basic troubleshooting. Level 1 Support is not intended to
       * correct product defect errors.
       *
       * May have different semantics for different organizations.
       */
      VendorSupportLevel1      = (1<<2),
      /**
       * Problem isolation, which means technical support designed
       * to duplicate customer problems, isolate problem area and provide
       * resolution for problems not resolved by Level 1 Support.
       *
       * May have different semantics for different organizations.
       */
      VendorSupportLevel2      = (1<<3),
      /**
       * Problem resolution, which means technical support designed
       * to resolve complex problems by engaging engineering in resolution
       * of product defects which have been identified by Level 2 Support.
       *
       * May have different semantics for different organizations.
       */
      VendorSupportLevel3      = (1<<4)
    };

    // Make a flag set for this
    ZYPP_DECLARE_FLAGS(VendorSupportOptions,VendorSupportOption);
    ZYPP_DECLARE_OPERATORS_FOR_FLAGS(VendorSupportOptions)

    /**
     * converts the support option to a name intended to be printed
     * to the user.
     *
     * Note the description is based in the way Novell defines the support
     * levels, and the semantics may be different for other vendors.
     */
    std::string asUserString( VendorSupportOption );

    /**
     * converts the support option to a description intended to be printed
     * to the user.
     *
     * Note the description is based in the way Novell defines the support
     * levels, and the semantics may be different for other vendors.
     */
    std::string asUserStringDescription( VendorSupportOption );

}

#endif
