create table auto_attendant(
  auto_attendant_id int4 not null primary key,
  name varchar(256) not null,
  system_id varchar(32),
  description varchar(256),
  prompt varchar(256) not null,
  extension varchar(32)
);

create sequence auto_attendant_seq;
create unique index idx_auto_attendant_name on auto_attendant (name);
create unique index idx_auto_attendant_system_id on auto_attendant (system_id);
 
create table attendant_menu_item(
  auto_attendant_id int4 not null,
  dialpad_key varchar(1) not null,
  action varchar(32) not null,
  parameter varchar(256),
  primary key (auto_attendant_id, dialpad_key)
);

alter table attendant_menu_item
 add constraint fk_attendant_menu_item_auto_attendant
 foreign key (auto_attendant_id) references auto_attendant (auto_attendant_id);


alter table internal_dialing_rule
 add column auto_attendant_id int4;

alter table internal_dialing_rule
 drop column autoAttendant;

/* must be followed up with patch to create operator, then point all */
/* existing internal dialing rules to that operator */ 
