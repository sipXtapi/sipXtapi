create table firmware(
  firmware_id int4 not null primary key,
  name varchar(255) not null unique,
  description varchar(255)
);

create sequence firmware_seq;

