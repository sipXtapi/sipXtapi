/** 
 * S E T T I N G   S T O R A G E
 */
create table storage(
  storage_id int4 not null primary key
);

create sequence storage_seq;

/**
 * S E T T I N G
 */
create table setting(
  storage_id int4 not null,
  path varchar(256) not null,
  value varchar(256) not null,
  primary key (storage_id, path)
);

create sequence setting_seq;

/**
 * E N D P O I N T
 */
create table endpoint(
  endpoint_id int4 not null primary key, 
  serial_number varchar(256) not null,
  name varchar(256),
  phone_id varchar(256) not null,
  storage_id int4
);

/* 
 * would like to add FK contraint to setting table
 * but setting_id can be null and I don't think thats
 * possible w/postgres
alter table endpoint
add constraint fk_endpoint_1
foreign key (storage_id) references storage (storage_id);
 */
 
create sequence endpoint_seq;
create unique index idx_endpoint_sernum on endpoint (serial_number);

/**
 * C R E D E N T I A L
 */
create table credential(
  credential_id int4 not null primary key,
  auth_id varchar(256),
  password varchar(256),
  realm varchar(256)
);
create sequence credential_seq;

/* 
 * L I N E 
 */
create table line(
  line_id int4 not null primary key,
  credential_id int4 not null,
  user_id int4 not null,
  storage_id int4,
  endpoint_id int4 not null
);
create sequence line_seq;

alter table line
add constraint fk_line_1 
foreign key (endpoint_id) references endpoint (endpoint_id);

/* 
 * would like to add FK contraint to setting table
 * but setting_id can be null and I don't think thats
 * possible w/postgres
alter table line
add constraint fk_line_2
foreign key (storage_id) references storage (storage_id);
*/

/** may relax if user null ok */
alter table line
add constraint fk_line_3
foreign key (user_id) references users (id);

alter table line
add constraint fk_line_4
foreign key (credential_id) references credential (credential_id);

