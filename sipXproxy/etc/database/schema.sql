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

create table call_state_event (
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


create table call_state_observer_event (
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
 * The fromto table holds SIP AORs appearing in From or To headers, not including tags.
 * Example: "Bob <sip:bob@biloxi.com>".
 */
create table fromto(
  id serial8 not null,                  /* Row ID */
  fromto_value text not null unique,    /* SIP AOR */
  primary key (id) 
);


/*
 * The contact table holds SIP contact URLs.  Example: "<sip:bob@192.0.2.4>".
 */
create table contact(
  id serial8 not null,                  /* Row ID */
  contact_value text not null unique,   /* SIP contact */
  primary key (id) 
);


/*
 * The dialog table holds the SIP call ID, from tag, and to tag, which together
 * uniquely identify a SIP call.  This info is not relevant for billing, but it
 * is useful if one wants to do post-processing on the CDRs, or to link a CDR
 * back to the call state events (CSE) from which the CDR was created.
 */
create table dialog(
  id serial8 not null,                  /* Row ID */
  call_id text not null,                /* SIP call ID */
  from_tag text not null,               /* SIP from tag */
  to_tag text not null,                 /* SIP to tag */
  unique (call_id, from_tag, to_tag),
  primary key (id) 
);


/*
 * Each row records info about a single SIP call.
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
create table cdr(
  id serial8 not null,              /* Row ID */
  dialog_id int8 not null,          /* Dialog ID: foreign key to dialog table */
  from_id int8 not null,            /* From AOR: foreign key to fromto table */
  from_contact_id int8 not null,    /* From contact: foreign key to contact table */
  to_id int8 not null,              /* To AOR: foreign key to fromto table */
  to_contact_id int8 not null,      /* To contact: foreign key to contact table */
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

alter table cdr
  add constraint fk_cdr_dialog
  foreign key (dialog_id)
  references dialog;

alter table cdr
  add constraint fk_cdr_fromto_from
  foreign key (from_id)
  references fromto;

alter table cdr
  add constraint fk_cdr_contact_from
  foreign key (from_contact_id)
  references contact;

alter table cdr
  add constraint fk_cdr_fromto_to
  foreign key (to_id)
  references fromto;

alter table cdr
  add constraint fk_cdr_contact_to
  foreign key (to_contact_id)
  references contact;
