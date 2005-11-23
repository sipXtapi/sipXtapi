create table firmware(
  firmware_id int4 not null primary key,
  name varchar(255) not null unique,
  delivery_id varchar(32) not null,
  manufacturer_id varchar(32) not null,
  model_id varchar(32),
  version_id varchar(32),
  value_storage_id int4,
  description varchar(255)
);

create sequence firmware_seq;

alter table firmware add constraint firmware_value_storage foreign key (value_storage_id) references value_storage;
