create table tag(
  storage_id int4,
  resource varchar(256) not null,
  label varchar(256),
  primary key (storage_id)
);

create unique index idx_tag_label on tag (label);

/* relate phones to tags */
create table phone_tag (
  storage_id int4 not null,
  phone_id int4 not null,
  index int4 not null,
  primary key (phone_id, storage_id)
);

create table line_tag (
  storage_id int4 not null,
  line_id int4 not null,
  index int4 not null,
  primary key (line_id, storage_id)
);


/* TODO migrate folder storage */

alter table phone
  drop column folder_id;

alter table line
  drop column folder_id;  