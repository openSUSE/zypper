/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Pending.h
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _Pending_h
#define _Pending_h

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/detail/PendingPtr.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      class Pending;
      typedef std::list <Pending_Ptr> PendingList;
      typedef PendingList * PendingList_Ptr;

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : Pending

      class Pending : public base::ReferenceCounted, private base::NonCopyable {
          

      typedef enum {
          PENDING_STATUS_INVALID = 0,
          PENDING_STATUS_PRE_BEGIN,
          PENDING_STATUS_RUNNING,
          PENDING_STATUS_BLOCKING,
          PENDING_STATUS_ABORTED,
          PENDING_STATUS_FAILED,
          PENDING_STATUS_FINISHED
      } PendingStatus;

      const char *pendingStatusToString (PendingStatus status);

      #define INVALID_PENDING_ID 0


        private:

          char *_description;
          int _id;

          PendingStatus _status;

          double _percent_complete;

          size_t _completed_size;
          size_t _total_size;

          time_t _start_time;
          time_t _last_time;
          time_t _poll_time;

          int _retval;
          char *_error_msg;

          std::list<const char *> _messages;

          void (*_update) (Pending_Ptr);
          void (*_complete) (Pending_Ptr);
          void (*_message) (Pending_Ptr);

        public:

          Pending (const char *description);
          virtual ~Pending();

          // ---------------------------------- I/O

          static std::string toString (const Pending & section);

          virtual std::ostream & dumpOn(std::ostream & str ) const;

          friend std::ostream& operator<<(std::ostream&, const Pending & section);

          std::string asString (void) const;

          // ---------------------------------- accessors

          const char *description (void) const { return _description; }
          void setDescription (const char *description) { _description = strdup (description); }
          int id (void) const { return _id; }
          PendingStatus status (void) const { return _status; }
          double percentComplete (void) const { return _percent_complete; }
          size_t completedSize (void) const { return _completed_size; }
          size_t totalSize (void) const { return _total_size; }
          time_t startTime (void) const { return _start_time; }
          time_t lastTime (void) const { return _last_time; }
          time_t pollTime (void) const { return _poll_time; }

          int elapsedSecs (void) const { return 0; }
          int expectedSecs (void) const { return 0; }
          int remainingSecs (void) const { return 0; }

          std::list<const char *> messages (void) const { return _messages; }
          const char *latestMessage (void) const { return _error_msg; }

          // ---------------------------------- methods

          Pending_Ptr lookupById (int id);
          std::list<Pending_Ptr> getAllActiveIds (void);

          void begin (void);
          void update (double percent_complete);
          void updateBySize (size_t size, size_t total_size);

          void finished (int retval);
          void abort (int retval);
          void fail (int retval, const char *error_msg);

          bool isActive (void);

          const char *errorMsg (void);

          void addMessage (const char *message);

      };

      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

#endif // _Pending_h
