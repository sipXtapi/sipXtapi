/** 
 *   W A R N I N G  :   
 *   exact copy maintained in profilegen_create.sql
 */

/** 
 * R I N G
 */
create table ring(
  ring_id int4 not null,
  user_id int4 not null,
  number varchar(256),
  expiration int4,
  position int4 not null,
  ring_type varchar(40),
  primary key (ring_id)
);

create unique index idx_ring_user_ring on ring (user_id, ring_id);
create index idx_ring_user on ring (user_id);

create sequence ring_seq;

alter table ring
add constraint fk_user_1
foreign key (user_id) references users (id);

/**
 * G A T E W A Y
 */
 
create table gateway (
  gateway_id int4 not null,
  name varchar(256),
  address varchar(256),
  description varchar(256),
  primary key (gateway_id)
);

create sequence gateway_seq;

/**
 * D I A L I N G   R U L E 
 */
 
create table dial_plan (
   dial_plan_id int4 not null,
   primary key (dial_plan_id)
);

create sequence dial_plan_seq;
 
create table dial_pattern (
   custom_dialing_rule_id int4 not null,
   prefix varchar(40),
   digits int4,
   index int4 not null,
   primary key (custom_dialing_rule_id, index)
);

create table dialing_rule (
   dialing_rule_id int4 not null,
   name varchar(256) not null,
   description varchar(256),
   enabled bool,
   position int4,
   dial_plan_id int4,
   primary key (dialing_rule_id)
);

alter table dialing_rule add constraint FK3B60F0A99F03EC22 foreign key (dial_plan_id) references dial_plan;

create table dialing_rule_gateway (
   dialing_rule_id int4 not null,
   gateway_id int4 not null,
   index int4 not null,
   primary key (dialing_rule_id, index)
);

create table custom_dialing_rule (
   custom_dialing_rule_id int4 not null,
   digits varchar(40),
   prefix varchar(40),
   primary key (custom_dialing_rule_id)
);

create table custom_dialing_rule_permission (
   custom_dialing_rule_id int4 not null,
   permission varchar(256)
);

create table international_dialing_rule (
   international_dialing_rule_id int4 not null,
   internationalPrefix varchar(40),
   primary key (international_dialing_rule_id)
);


create table local_dialing_rule (
   local_dialing_rule_id int4 not null,
   externalLen int4,
   pstnPrefix varchar(40),
   primary key (local_dialing_rule_id)
);

create table emergency_dialing_rule (
   emergency_dialing_rule_id int4 not null,
   optionalPrefix varchar(40),
   emergencyNumber varchar(40),
   useMediaServer bool,
   primary key (emergency_dialing_rule_id)
);

create table internal_dialing_rule (
   internal_dialing_rule_id int4 not null,
   autoAttendant varchar(40),
   localExtensionLen int4,
   voiceMail varchar(40),
   voiceMailPrefix varchar(40),
   primary key (internal_dialing_rule_id)
);

create table long_distance_dialing_rule (
   international_dialing_rule_id int4 not null,
   areaCodes varchar(256),
   externalLen int4,
   longDistancePrefix varchar(40),
   permission varchar(256),
   pstnPrefix varchar(40),
   primary key (international_dialing_rule_id)
);

alter table dial_pattern add constraint FK8D4D2DC1454433A3 foreign key (custom_dialing_rule_id) references custom_dialing_rule;
alter table international_dialing_rule add constraint FKE5D682BA7DD83CC0 foreign key (international_dialing_rule_id) references dialing_rule;
alter table local_dialing_rule add constraint FK365020FD76B0539D foreign key (local_dialing_rule_id) references dialing_rule;
alter table custom_dialing_rule add constraint FKB189AEB7454433A3 foreign key (custom_dialing_rule_id) references dialing_rule;
alter table custom_dialing_rule_permission add constraint FK8F3EE457454433A3 foreign key (custom_dialing_rule_id) references custom_dialing_rule;
alter table dialing_rule_gateway add constraint FK65E824AE38F854F6 foreign key (gateway_id) references gateway;
alter table dialing_rule_gateway add constraint FK65E824AEF6075471 foreign key (dialing_rule_id) references dialing_rule;
alter table emergency_dialing_rule add constraint FK7EAEE897444E2DC3 foreign key (emergency_dialing_rule_id) references dialing_rule;
alter table internal_dialing_rule add constraint FK5D102EEBDE4556EF foreign key (internal_dialing_rule_id) references dialing_rule;
alter table long_distance_dialing_rule add constraint FKA10B67307DD83CC0 foreign key (international_dialing_rule_id) references dialing_rule;

create sequence dialing_rule_seq;
