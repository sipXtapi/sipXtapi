/**
 * S E T T I N G
 */
create table setting(
  setting_id int4 not null primary key,
  parent_id int4,
  name varchar(256) not null,
  string_value varchar(256),
  type int4
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
  setting_id int4
);

/* 
 * would like to add FK contraint to setting table
 * but setting_id can be null and I don't think thats
 * possible w/postgres
 */
create sequence endpoint_seq;
create unique index idx_endpoint_sernum on endpoint (serial_number);

/* 
 * L I N E 
 */
create table line(
  line_id int4 not null primary key,
  auth_id varchar(256),
  user_id varchar(256) not null,
  server varchar(256),
  extension varchar(256) not null
);
create sequence line_seq;
create unique index idx_line_extension on line (extension);

/**
 * E N D P O I N T  A S S I G N M E N T
 */
create table endpoint_line(
  endpoint_line_id int4 not null primary key, 
  setting_id int4,
  line_id int4 not null,
  endpoint_id int4 not null
);

alter table endpoint_line
add constraint fk_endpoint_line_1 
foreign key (endpoint_id) references endpoint (endpoint_id)
;

alter table endpoint_line
add constraint fk_endpoint_line_2 
foreign key (line_id) references line (line_id)
;

create sequence endpoint_line_seq;
