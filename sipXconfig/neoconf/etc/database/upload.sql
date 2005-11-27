create table upload(
  upload_id int4 not null primary key,
  name varchar(255) not null unique,
  specification_id varchar(32) not null,
  value_storage_id int4,
  description varchar(255)
);

-- same sequence number as upload
create table upload_id(
  upload_id int4 not null primary key
);

create sequence upload_seq;

alter table upload add constraint upload_value_storage foreign key (value_storage_id) references value_storage;
