create table auto_attendant(
  auto_attendant_id int4 not null primary key,
  name varchar(256) not null,
  description varchar(256),
  prompt varchar(256) not null,
  extension varchar(32)
);

create sequence auto_attendant_seq;
create unique index idx_auto_attendant_name on auto_attendant (name);
 
create table attendant_menu_item(
  auto_attendant_id int4 not null,
  dialpad_key varchar(1) not null,
  action varchar(32) not null,
  parameter varchar(256),
  primary key (auto_attendant_id, dialpad_key)
);