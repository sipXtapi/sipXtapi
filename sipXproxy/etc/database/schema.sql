/*
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 */

/* 
 * Schema for Call State Events (CSE) and Call Detail Records (CDR)
 */

---------------------------------- CSE Tables ----------------------------------

/*
 * The call_state_events holds events that describe the lifecycle of a call:
 * call request, call setup, call end or call failure.
 */
create table call_state_events (
   id              serial8 not null,
   observer        text not null,
   event_seq       int8 not null,
   event_time      timestamp not null,
   event_type      char(1) not null,
   callid          text not null,
   from_tag        text,
   to_tag          text,
   from_url        text not null,
   to_url          text not null,
   contact         text,
   refer           text,
   failure_status  int2,
   failure_reason  text,
   primary key     (id)
);


/*
 * The call_state_observer_events holds events relating to the event observer
 * state, for example, that the observer has restarted.  The forking proxy
 * and auth proxy are event observers.
 */
create table call_state_observer_events (
   id              serial8 not null,
   observer        text not null,
   event_seq       int8 not null,
   event_time      timestamp not null,
   event_type      char(1) not null,
   status          int2 not null,
   msg             text,
   uri             text,
   primary key     (id)
);


---------------------------------- CDR Tables ----------------------------------

/* 
 * The aors table holds SIP addresses of record (AORs) appearing in From or To
 * headers, not including tags.  Example: "Bob <sip:bob@biloxi.com>".
 */
create table aors (
  id serial8 not null,                  /* Row ID */
  value text not null unique,           /* SIP AOR */
  primary key (id) 
);


/*
 * The contacts table holds SIP contact URLs.  Example: "<sip:bob@192.0.2.4>".
 */
create table contacts (
  id serial8 not null,                  /* Row ID */
  value text not null unique,           /* SIP contact */
  primary key (id) 
);


/*
 * The dialogs table holds the SIP call ID, from tag, and to tag, which together
 * uniquely identify a SIP call.  This info is not relevant for billing, but it
 * is useful if one wants to do post-processing on the CDRs, or to link a CDR
 * back to the call state events (CSE) from which the CDR was created.
 */
create table dialogs (
  id serial8 not null,                  /* Row ID */
  call_id text not null,                /* SIP call ID */
  from_tag text not null,               /* SIP from tag */
  to_tag text not null,                 /* SIP to tag */
  unique (call_id, from_tag, to_tag),
  primary key (id) 
);


/*
 * The cdrs table records info about SIP calls.
 *
 * Start, connect, and end times are nullable to allow for partial CDRs where the
 * call cannot be completely analyzed.  For example, if we can't find a BYE or 
 * any other end event, then the end_time is indeterminate and will be set to NULL.
 * Note that NULL time values work fine with the check constraints declared below,
 * because comparisons return NULL for NULL operands, and a check constraint is
 * satisfied by a NULL value.
 *
 * The termination code column holds an single char value indicating why the call
 * was terminated, for example, "N" to indicate normal completion.  Enum values TBD.
 * The IPDR defines a standard set of three-letter termination codes
 * (see http://www.ipdr.org/public/Service_Specifications/3.X/VoIP3.5-A.0.1.pdf).
 * Our codes will have similar semantics, and there will be a mapping between them,
 * but we won't use the IPDR codes directly, to save space.
 */
create table cdrs (
  id serial8 not null,              /* Row ID */
  dialog_id int8 not null,          /* Dialog ID: foreign key to dialogs table */
  from_id int8 not null,            /* From AOR: foreign key to aors table */
  from_contact_id int8 not null,    /* From contact: foreign key to contact table */
  to_id int8 not null,              /* To AOR: foreign key to aors table */
  to_contact_id int8 not null,      /* To contact: foreign key to contacts table */
  start_time timestamp,             /* Start time in GMT: initial INVITE received */
  connect_time timestamp,           /* Connect time in GMT: ACK received for 200 OK */
  end_time timestamp,               /* End time in GMT: BYE received, or other ending */
  check (connect_time > start_time),
  check (end_time > connect_time),
  termination char(1),              /* Why the call was terminated */
  failure int2,                     /* SIP error code if the call failed, e.g., 4xx */
  primary key (id)
);


/* Add foreign key constraints */

alter table cdrs
  add constraint fk_cdrs_dialogs
  foreign key (dialog_id)
  references dialogs;

alter table cdrs
  add constraint fk_cdrs_aors_from
  foreign key (from_id)
  references aors;

alter table cdrs
  add constraint fk_cdrs_contacts_from
  foreign key (from_contact_id)
  references contacts;

alter table cdrs
  add constraint fk_cdrs_aors_to
  foreign key (to_id)
  references aors;

alter table cdrs
  add constraint fk_cdrs_contacts_to
  foreign key (to_contact_id)
  references contacts;


---------------------------------- Functions ----------------------------------

/*
 * Return the host part of a SIP contact URI.  For example, given the input
 * "sip:bob@192.0.2.4:999", return "192.0.2.4".
 * The host part of the URI starts after the "@".  If there is a port number, 
 * then the host substring ends before ":".  Otherwise it goes to the end of
 * the string.
 */
CREATE FUNCTION contact_host(text) RETURNS text AS $$
  SELECT substring($1 from '@([^\:]*)');
$$ LANGUAGE SQL;
