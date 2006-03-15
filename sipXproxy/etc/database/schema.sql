/*
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 */

/* 
 * Schema for Call Resolver data, including Call State Events (CSE) and
 * Call Detail Records (CDR).
 */


---------------------------------- Versioning ----------------------------------
/*
 * Follow the sipXconfig model
 */

create table version_history(
  version int4 not null primary key,
  applied date not null
);

/**
 * CHANGE VERSION HERE ----------------------------> X <------------------
 *
 * For sipX v??? (first release with Call Resolver), the database version is 1.
 * Activate this line of code when we ship the Call Resolver.
 */
-- insert into version_history (version, applied) values (1, now());

create table patch(
  name varchar(32) not null primary key
);


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
   call_id         text not null,
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
 * The parties table holds the SIP address of record (AOR) and contact URL for
 * a party in a SIP call, the caller or callee.
 * AOR example: "Bob <sip:bob@biloxi.com>".
 * Contact example: "<sip:bob@192.0.2.4>".
 */
create table parties (
  id serial8 not null,                  /* Row ID */
  aor text not null,                    /* SIP AOR */
  contact text not null,                /* SIP contact URL */
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
 * was terminated.  Codes are:
 * 
 *   R: call requested -- got a call_request event, but no other events.
 *   I: call in progress -- got both call_request and call_setup events.
 *   C: successful call completion -- call_request, call_setup, and call_end with
 *      no errors.
 *   F: call failed -- an error occurred.
 */
create table cdrs (
  id serial8 not null,              /* Row ID */
  dialog_id int8 not null           /* Dialog ID: foreign key to dialogs table */
    references dialogs (id),
  caller_id int8 not null           /* Caller info: foreign key to parties table */
    references parties (id),
  callee_id int8 not null           /* Callee info: foreign key to parties table */
    references parties (id),
  start_time timestamp,             /* Start time in GMT: initial INVITE received */
  connect_time timestamp,           /* Connect time in GMT: ACK received for 200 OK */
  end_time timestamp,               /* End time in GMT: BYE received, or other ending */
  check (connect_time > start_time),
  check (end_time > connect_time),
  termination char(1),              /* Why the call was terminated */
  failure_status int2,              /* SIP error code if the call failed, e.g., 4xx */
  failure_reason text,              /* Text describing the reason for a call failure */
  primary key (id)
);


---------------------------------- Views ----------------------------------

/*
 * Simplify the presentation of CDRs by showing commonly used CDR data in a single
 * view so that the user doesn't have to be confronted with normalization into
 * different tables.
 * Do not include SIP dialog info in the view since it is not of interest for billing,
 * it is only used to link the CDR back to raw CSE data, or for CDR post-processing.
 */

create view view_cdrs as
  select cdr.id, 
         caller.aor as caller_aor, callee.aor as callee_aor,
         start_time, connect_time, end_time,
         termination, failure_status, failure_reason
  from cdrs cdr, parties caller, parties callee
  where cdr.caller_id = caller.id
  and   cdr.callee_id = callee.id
