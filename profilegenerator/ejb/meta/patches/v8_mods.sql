create table setting(
  setting_id int4 not null primary key,
  parent_id int4,
  name varchar(256) not null,
  string_value varchar(256),
  type int4
);

create sequence setting_seq;

create table endpoint(
  endpoint_id int4 not null primary key, 
  serial_number varchar(256) not null,
  description varchar(256),
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

create table endpoint_assignment(
  assignment_id int4 not null primary key, 
  label varchar(256) not null,
  user_id int4 not null,
  endpoint_id int4 not null
);

alter table endpoint_assignment
add constraint fk_endpoint_1 
foreign key (endpoint_id) references endpoint (endpoint_id)
;

alter table endpoint_assignment
add constraint fk_endpoint_2 
foreign key (user_id) references users (id)
;

create sequence assignment_seq;
