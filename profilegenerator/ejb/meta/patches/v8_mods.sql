create table endpoint(
  endpoint_id int4 not null primary key, 
  serial_number varchar(256) not null,
  description varchar(256),
  phone_id varchar(256) not null  
);

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
