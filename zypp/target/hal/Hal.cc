/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/hal/Hal.cc
 *
*/
#include <iostream>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "HAL"
#include "zypp/base/Logger.h"

#include "zypp/target/hal/Hal.h"

#ifndef FAKE_HAL

#include <hal/libhal.h>

#endif

using std::endl;
using std::string;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace hal
    { /////////////////////////////////////////////////////////////////

#ifndef FAKE_HAL
///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Hal::Impl
//
/** Hal implementation. */
struct Hal::Impl
{
    /**
     * pointer to dbus connection
     */
    DBusConnection *_connection;

    /**
     * pointer to complete hal context
     */
    LibHalContext *_context;

    /**
     * report HAL error and reset the error condition
     */
    void
    report_error (const std::string & reason, DBusError & error) const
    {
	ERR << reason << ": " << string (error.name) << ": " << string(error.message);
	dbus_error_init(&error);
	return;
    }

    /** Ctor. */
    Impl()
    {
	DBusError error;
	dbus_error_init (&error);

	_connection = dbus_bus_get (DBUS_BUS_SYSTEM, &error);			// get shared connection to DBUS 'socket'
	//_connection = dbus_bus_get_private (DBUS_BUS_SYSTEM, &error);		// get private connection DBUS 'socket'
	if (_connection) {
	    _context = libhal_ctx_new ();					// create empty HAL context
	    if (_context) {
		if (libhal_ctx_set_dbus_connection (_context, _connection)) {	// connect to HAL daemon via DBUS
		    if (libhal_ctx_init (_context, &error)) {			// fill HAL context
			return;
		    } else {
			report_error ("libhal_ctx_init", error);
		    }
		} else {
		    report_error ("libhal_ctx_set_dbus_connection", error);
		}
		libhal_ctx_free (_context);					// clean up
		_context = NULL;
	    } else {
		report_error ("libhal_ctx_new: Can't create libhal context", error);
	    }
	    // dbus_connection_close (_connection);				// call only if dbus_bus_get_private was used
	    dbus_connection_unref (_connection);
	    _connection = NULL;
	} else {
	    report_error ("dbus_bus_get", error);
	}
    }


    /** Dtor. */
    ~Impl()
    {
	if (_context) {
	    libhal_ctx_free (_context);
	}
	if (_connection) {
	    // dbus_connection_close (_connection);				// call only if dbus_bus_get_private was used
	    dbus_connection_unref (_connection);
	}
    }

    /**
     * query for HAL capability present
     */

    bool query( const std::string & cap_r ) const
    { return query( cap_r, Rel::ANY, std::string() ); }

    /**
     * query for HAL capability having a specific value
     */
    bool  query( const std::string & cap_r,
	       Rel op_r,
	       const std::string & val_r ) const
    {
	DBusError error;
	dbus_error_init (&error);

	// ask HAL which devices provide the needed capability

	bool result = false;

	int device_count;
	char **device_names = libhal_find_device_by_capability (_context, cap_r.c_str(), &device_count, &error);

	if (device_names == NULL) {
	    report_error ("libhal_find_device_by_capability", error);
	    result = false;
	}
	else if (device_count > 0) {

#if 0		// once we get a capabilities value from HAL, we can compare it ...
	    if (value) {
		string lhs (value);
		int cmp = (lhs != rhs) ? ((lhs < rhs) ? -1 : 1) : 0;

		switch ( relation.inSwitch() )
		{
		    case Rel::EQ_e:
			res = (cmp == 0);
			break;
		    case Rel::NE_e:
			res = (cmp != 0);
			break;
		    case Rel::LT_e:
			res = (cmp == -1);
			break;
		    case Rel::LE_e:
			res = (cmp != 1);
			break;
		    case Rel::GT_e:
			res = (cmp == 1);
			break;
		    case Rel::GE_e:
			res = (cmp != -1);
			break;
		    case Rel::ANY_e:
			res = true;
			break;
		    case Rel::NONE_e:
			res = false;
			break;
		    default:
			// We shouldn't get here.
			INT << "Unknown relational opertor '" << relation << "' treated as  'NONE'" << endl;
			break;
		}
	    }
#endif

	    result = true;

	}

	if (device_names != NULL)
	    libhal_free_string_array (device_names);

	return result;
    }

  public:
     /** Offer default Impl. */
    static shared_ptr<Impl> nullimpl()
    {
	static shared_ptr<Impl> _nullimpl( new Impl );
	return _nullimpl;
    }

};  // struct Hal::Impl


#else // FAKE_HAL
      struct Hal::Impl
      {
        bool query( const std::string & cap_r ) const
        { return query( cap_r, Rel::ANY, std::string() ); }
        bool  query( const std::string & cap_r,
                     Rel op_r,
                     const std::string & val_r ) const
        { return false; }
        /** Offer default Impl. */
        static shared_ptr<Impl> nullimpl()
        {
          static shared_ptr<Impl> _nullimpl( new Impl );
          return _nullimpl;
        }
      };
#endif

///////////////////////////////////////////////////////////////////

/** \relates Hal::Impl Stream output
     * And maybe std::ostream & operator<< Hal::Impl below too.
     * return libhal version or something like that.
 */
inline std::ostream & operator<<( std::ostream & str, const Hal::Impl & obj )
{
  return str << "Hal::Impl";
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Hal
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : Hal::Hal
//	METHOD TYPE : Ctor
//
Hal::Hal()
: _pimpl( Impl::nullimpl() )
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : Hal::~Hal
//	METHOD TYPE : Dtor
//
Hal::~Hal()
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : Hal::instance
//	METHOD TYPE : Hal &
//
Hal & Hal::instance()
{
  static Hal _singleton;
  return _singleton;
}

///////////////////////////////////////////////////////////////////
// Foreward to implenemtation
///////////////////////////////////////////////////////////////////

bool Hal::query( const std::string & cap_r ) const
{ return _pimpl->query( cap_r ); }

bool Hal::query( const std::string & cap_r,
		 Rel op_r,
		 const std::string & val_r ) const
{ return _pimpl->query( cap_r, op_r, val_r ); }

/******************************************************************
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
*/
std::ostream & operator<<( std::ostream & str, const Hal & obj )
{
  return str << *obj._pimpl;
}

/////////////////////////////////////////////////////////////////
    } // namespace hal
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
