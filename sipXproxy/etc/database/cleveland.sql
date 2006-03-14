/*
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 */

/* 
 * Additional customer-specific Call Resolver tables.
 */

/*
 * Call direction is considered to be: 
 *
 *   Incoming (I): for calls that come in from a PSTN gateway
 *   Outgoing (O): for calls that go out to a PSTN gateway
 *   Intranetwork (A): for calls that are pure SIP and don't go through a gateway
 *
 * The direction column holds a single char value for call direction with the 
 * values indicated above.  Because call_directions is just an extension to the 
 * cdrs table, the id is both the primary key for this table and a foreign key
 * to the cdrs table.
 */
create table call_directions (
  id int8 not null,
  direction char(1) not null,
  primary key (id)
);


/* Add foreign key constraints */

alter table call_directions
  add constraint call_directions_fk_cdrs
  foreign key (id)
  references cdrs;


---------------------------------- Views ----------------------------------

/*
 * Simplify the presentation of CDRs by showing all CDR data in a single view so that the
 * user doesn't have to be confronted with normalization into different tables.
 * Add call_directions to the standard view.
 */

create view view_cdrs_cleveland as
  select cdr.id, 
         caller_aor, callee_aor,
         start_time, connect_time, end_time,
         termination, failure,
         direction
  from view_cdrs cdr, call_directions call_dir
  where cdr.id = call_dir.id
