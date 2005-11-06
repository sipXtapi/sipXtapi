create table domain (
   domain_id int4 not null,
   description varchar(255),
   name varchar(255) not null unique,
   bean_id varchar(255) not null,
   value_storage_id int4,
   primary key (domain_id)
);
create sequence domain_seq;
