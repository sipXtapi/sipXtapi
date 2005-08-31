create table extension_pool (
  extension_pool_id int4 not null,
  enabled boolean not null,
  name varchar(255) unique not null,
  first_extension int4,
  last_extension int4,
  next_extension int4,
  primary key (extension_pool_id)
);

create sequence extension_pool_seq;
