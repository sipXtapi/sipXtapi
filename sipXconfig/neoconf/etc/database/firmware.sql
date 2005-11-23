create table firmware(
  firmware_id int4 not null primary key,
  name varchar(255) not null unique,
  manufacturer_id varchar(32) not null,
  value_storage_id int4,
  description varchar(255)
);

-- same sequence number as firmware
create table upload_id(
  upload_id int4 not null primary key
);

create sequence firmware_seq;

alter table firmware add constraint firmware_value_storage foreign key (value_storage_id) references value_storage;
