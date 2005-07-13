/*
 * Unmapped hibernate tables
 */
create table version_history(
  version int4 not null primary key,
  applied date not null
);
insert into version_history (version, applied) values (0, now());
create table patch(
  name varchar(32) not null primary key
);
/*
 * hibernate tables
 */
create table phone (
   phone_id int4 not null,
   name varchar(255),
   serial_number varchar(255),
   factory_id varchar(255),
   value_storage_id int4,
   primary key (phone_id)
);
create table dial_pattern (
   custom_dialing_rule_id int4 not null,
   element_prefix varchar(255),
   element_digits int4,
   index int4 not null,
   primary key (custom_dialing_rule_id, index)
);
create table international_dialing_rule (
   international_dialing_rule_id int4 not null,
   international_prefix varchar(255),
   primary key (international_dialing_rule_id)
);
create table setting_value (
   value_storage_id int4 not null,
   value varchar(255) not null,
   path varchar(255) not null,
   primary key (value_storage_id, path)
);
create table line_group (
   line_id int4 not null,
   group_id int4 not null,
   primary key (line_id, group_id)
);
create table dialing_rule (
   dialing_rule_id int4 not null,
   name varchar(255) not null,
   description varchar(255),
   enabled bool,
   position int4,
   dial_plan_id int4,
   primary key (dialing_rule_id)
);
create table auto_attendant (
   auto_attendant_id int4 not null,
   name varchar(255),
   extension varchar(255),
   prompt varchar(255),
   system_id varchar(255),
   description varchar(255),
   primary key (auto_attendant_id)
);
create table park_orbit (
   park_orbit_id int4 not null,
   orbit_type char(1) not null,
   music varchar(255),
   enabled bool,
   name varchar(255),
   extension varchar(255),
   description varchar(255),
   primary key (park_orbit_id)
);
create table group_storage (
   group_id int4 not null,
   name varchar(255),
   description varchar(255),
   resource varchar(255),
   weight int4,
   primary key (group_id)
);
create table user_ring (
   user_ring_id int4 not null,
   position int4,
   expiration int4,
   ring_type varchar(255),
   call_group_id int4 not null,
   user_id int4 not null,
   primary key (user_ring_id)
);
create table gateway (
   gateway_id int4 not null,
   factory_id varchar(255) not null,
   name varchar(255),
   address varchar(255),
   description varchar(255),
   serial_number varchar(255),
   value_storage_id int4,
   primary key (gateway_id)
);
create table daily_backup_schedule (
   daily_backup_schedule_id int4 not null,
   enabled bool,
   time_of_day timestamp,
   scheduled_day varchar(255),
   backup_plan_id int4,
   primary key (daily_backup_schedule_id)
);
create table job (
   job_id int4 not null,
   type int4,
   status char(1),
   start_time_string varchar(255),
   details varchar(255),
   progress varchar(255),
   exception_message varchar(255),
   primary key (job_id)
);
create table group_weight (
   weight int4 not null,
   primary key (weight)
);
create table line (
   line_id int4 not null,
   phone_id int4 not null,
   position int4,
   user_id int4,
   value_storage_id int4,
   primary key (line_id)
);
create table value_storage (
   value_storage_id int4 not null,
   primary key (value_storage_id)
);
create table local_dialing_rule (
   local_dialing_rule_id int4 not null,
   external_len int4,
   pstn_prefix varchar(255),
   primary key (local_dialing_rule_id)
);
create table attendant_menu_item (
   auto_attendant_id int4 not null,
   action varchar(255),
   parameter varchar(255),
   dialpad_key varchar(255) not null,
   primary key (auto_attendant_id, dialpad_key)
);
create table custom_dialing_rule (
   custom_dialing_rule_id int4 not null,
   call_pattern_digits varchar(255),
   call_pattern_prefix varchar(255),
   primary key (custom_dialing_rule_id)
);
create table call_group (
   call_group_id int4 not null,
   enabled bool,
   name varchar(255),
   extension varchar(255),
   description varchar(255),
   primary key (call_group_id)
);
create table phone_group (
   phone_id int4 not null,
   group_id int4 not null,
   primary key (phone_id, group_id)
);
create table dialing_rule_gateway (
   dialing_rule_id int4 not null,
   gateway_id int4 not null,
   index int4 not null,
   primary key (dialing_rule_id, index)
);
create table emergency_dialing_rule (
   emergency_dialing_rule_id int4 not null,
   optional_prefix varchar(255),
   emergency_number varchar(255),
   use_media_server bool,
   primary key (emergency_dialing_rule_id)
);
create table backup_plan (
   backup_plan_id int4 not null,
   limited_count int4,
   configs bool,
   voicemail bool,
   dbase bool,
   primary key (backup_plan_id)
);
create table dial_plan (
   dial_plan_id int4 not null,
   primary key (dial_plan_id)
);
create table ring (
   ring_id int4 not null,
   number varchar(255),
   position int4,
   expiration int4,
   ring_type varchar(255),
   user_id int4 not null,
   primary key (ring_id)
);
create table users (
   user_id int4 not null,
   first_name varchar(255),
   pintoken varchar(255),
   sip_password varchar(255),
   last_name varchar(255),
   display_id varchar(255),
   extension varchar(255),
   primary key (user_id)
);
create table custom_dialing_rule_permission (
   custom_dialing_rule_id int4 not null,
   permission varchar(255)
);
create table internal_dialing_rule (
   internal_dialing_rule_id int4 not null,
   auto_attendant_id int4,
   local_extension_len int4,
   voice_mail varchar(255),
   voice_mail_prefix varchar(255),
   aa_aliases varchar(255),
   primary key (internal_dialing_rule_id)
);
create table long_distance_dialing_rule (
   international_dialing_rule_id int4 not null,
   area_codes varchar(255),
   external_len int4,
   long_distance_prefix varchar(255),
   permission varchar(255),
   pstn_prefix varchar(255),
   primary key (international_dialing_rule_id)
);
alter table phone add constraint FK65B3D6ECB50FCED foreign key (value_storage_id) references value_storage;
alter table dial_pattern add constraint FK8D4D2DC1454433A3 foreign key (custom_dialing_rule_id) references custom_dialing_rule;
alter table international_dialing_rule add constraint FKE5D682BA7DD83CC0 foreign key (international_dialing_rule_id) references dialing_rule;
alter table setting_value add constraint FKBB1806C2CB50FCED foreign key (value_storage_id) references value_storage;
alter table line_group add constraint FK8A14274A8B4D46 foreign key (line_id) references line;
alter table line_group add constraint FK8A142741E2E76DB foreign key (group_id) references group_storage;
alter table dialing_rule add constraint FK3B60F0A99F03EC22 foreign key (dial_plan_id) references dial_plan;
alter table group_storage add constraint FK92BDF0BB1E2E76DB foreign key (group_id) references value_storage;
alter table user_ring add constraint FK143BDE242A05F79C foreign key (call_group_id) references call_group;
alter table user_ring add constraint FK143BDE24F73AEE0F foreign key (user_id) references users;
alter table gateway add constraint FKF4BA4644CB50FCED foreign key (value_storage_id) references value_storage;
alter table daily_backup_schedule add constraint FK5518A4EFE76E474 foreign key (backup_plan_id) references backup_plan;
alter table line add constraint FK32AFF4CB50FCED foreign key (value_storage_id) references value_storage;
alter table line add constraint FK32AFF4B3B3158C foreign key (phone_id) references phone;
alter table line add constraint FK32AFF4F73AEE0F foreign key (user_id) references users;
alter table local_dialing_rule add constraint FK365020FD76B0539D foreign key (local_dialing_rule_id) references dialing_rule;
alter table attendant_menu_item add constraint FKD3F02D415BEFFE1D foreign key (auto_attendant_id) references auto_attendant;
alter table custom_dialing_rule add constraint FKB189AEB7454433A3 foreign key (custom_dialing_rule_id) references dialing_rule;
alter table phone_group add constraint FKD5244C6EB3B3158C foreign key (phone_id) references phone;
alter table phone_group add constraint FKD5244C6E1E2E76DB foreign key (group_id) references group_storage;
alter table dialing_rule_gateway add constraint FK65E824AE38F854F6 foreign key (gateway_id) references gateway;
alter table dialing_rule_gateway add constraint FK65E824AEF6075471 foreign key (dialing_rule_id) references dialing_rule;
alter table emergency_dialing_rule add constraint FK7EAEE897444E2DC3 foreign key (emergency_dialing_rule_id) references dialing_rule;
alter table ring add constraint FK356A30F73AEE0F foreign key (user_id) references users;
alter table users add constraint FK6A68E08F73AEE0F foreign key (user_id) references users;
alter table custom_dialing_rule_permission add constraint FK8F3EE457454433A3 foreign key (custom_dialing_rule_id) references custom_dialing_rule;
alter table internal_dialing_rule add constraint FK5D102EEB5BEFFE1D foreign key (auto_attendant_id) references auto_attendant;
alter table internal_dialing_rule add constraint FK5D102EEBDE4556EF foreign key (internal_dialing_rule_id) references dialing_rule;
alter table long_distance_dialing_rule add constraint FKA10B67307DD83CC0 foreign key (international_dialing_rule_id) references dialing_rule;
create sequence group_weight_seq;
create sequence dialing_rule_seq;
create sequence ring_seq;
create sequence daily_backup_schedule_seq;
create sequence gateway_seq;
create sequence park_orbit_seq;
create sequence auto_attendant_seq;
create sequence storage_seq;
create sequence dial_plan_seq;
create sequence phone_seq;
create sequence user_seq;
create sequence line_seq;
create sequence job_seq;
create sequence call_group_seq;
create sequence backup_plan_seq;

insert into users (user_id, pintoken, display_id) 
  values (nextval('user_seq'), 'password', 'superadmin');
