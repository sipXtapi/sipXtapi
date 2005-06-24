create table group_storage (
  storage_id int4,
  resource varchar(256) not null,
  name varchar(256) not null,
  weight int4 not null,
  description varchar(256),
  primary key (storage_id)
);

create unique index idx_group_storage_name on group_storage (name);

/* relate phones to tags */
create table phone_group (
  storage_id int4 not null,
  phone_id int4 not null,
  primary key (phone_id, storage_id)
);

create table line_group (
  storage_id int4 not null,
  line_id int4 not null,
  primary key (line_id, storage_id)
);

/* Could just use sequence, but wanted to do this in a DB */
/* neutral way and to leverage hibernate, thus avoiding SQL trickery */
create table group_weight (
  weight int4 primary key
);

create sequence group_weight_seq;
/* save zero for root */
select nextval('group_weight_seq');


/* TODO migrate folder storage */

alter table phone
  drop column folder_id;

alter table line
  drop column folder_id;  

  