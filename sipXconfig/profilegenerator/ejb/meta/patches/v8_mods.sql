/** 
 *   W A R N I N G  :   
 *   exact copy maintained in profilegen_create.sql
 */

create table folder(
  folder_id int4 not null primary key,
  resource varchar(256) not null,
  label varchar(256),
  parent_id int4
);

create sequence folder_seq;
create unique index idx_folder_resource_label on folder (resource, label);


create table folder_setting(
  folder_id int4 not null,
  path varchar(256) not null,
  value varchar(256),
  hidden bool,
  primary key (folder_id, path)
);

create table storage(
  storage_id int4 not null primary key
);

create sequence storage_seq;

create table setting(
  storage_id int4 not null,
  path varchar(256) not null,
  value varchar(256) not null,
  primary key (storage_id, path)
);

create sequence setting_seq;

create table phone(
  phone_id int4 not null primary key, 
  serial_number varchar(256) not null,
  name varchar(256),
  factory_id varchar(64) not null,
  storage_id int4,
  folder_id int4 not null
);

create unique index idx_phone_sernum on phone (serial_number);

alter table phone
add constraint fk_phone_2
foreign key (folder_id) references folder (folder_id);

create table line(
  line_id int4 not null primary key,
  position int4 not null,
  user_id int4 not null,
  storage_id int4,
  folder_id int4 not null,
  phone_id int4 not null
);
create sequence line_seq;

alter table line
add constraint fk_line_1 
foreign key (phone_id) references phone (phone_id);

alter table line
add constraint fk_line_2
foreign key (folder_id) references folder (folder_id);

alter table line
add constraint fk_line_3
foreign key (user_id) references users (id);
