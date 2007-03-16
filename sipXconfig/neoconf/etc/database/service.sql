create table service(
  service_id int4 not null primary key,
  name varchar(255) not null unique,
  bean_id varchar(32) not null,
  descriptor_id varchar(32),
  value_storage_id int4,
  description varchar(255)
);

create sequence service_seq;

alter table service add constraint service_value_storage foreign key (value_storage_id) references value_storage;
