/*
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 */

/* 
 * This file contains SQL for the "call direction" feature needed by a specific customer.
 */


---------------------------------- Tables ----------------------------------

/*
 * For cleanliness we could put call direction in its own table, separate 
 * from the cdrs table.  But that's overkill just for a single char(1) column.
 * So this column is in the cdrs table, see schema.sql.
 *
 * Call direction is encoded in char(1) as follows: 
 *
 *   Incoming (I): for calls that come in from a PSTN gateway
 *   Outgoing (O): for calls that go out to a PSTN gateway
 *   Intranetwork (A): for calls that are pure SIP and don't go through a gateway
 */

---------------------------------- Views ----------------------------------

/*
 * Simplify the presentation of CDRs by showing all CDR data in a single view so that the
 * user doesn't have to be confronted with normalization into different tables.
 * Add call_directions to the standard view.
 */

create view view_cdrs_cleveland as
  select view_cdr.id, 
         view_cdr.caller_aor, view_cdr.callee_aor,
         view_cdr.start_time, view_cdr.connect_time, view_cdr.end_time,
         view_cdr.termination, view_cdr.failure_status, view_cdr.failure_reason,
         cdr.call_direction
  from view_cdrs view_cdr, cdrs cdr
  where view_cdr.id = cdr.id
