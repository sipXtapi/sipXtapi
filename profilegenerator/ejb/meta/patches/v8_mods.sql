/** 
 *   W A R N I N G  :   
 *   exact copy maintained in profilegen_create.sql
 */

/**
 * F O L D E R
 */
create table folder(
  folder_id int4 not null primary key,
  resource varchar(256) not null,
  label varchar(256),
  parent_id int4
);

create sequence folder_seq;
create unique index idx_folder_resource_label on folder (resource, label);


/**
 * F O L D E R   S E T T I N G
 */
create table folder_setting(
  folder_id int4 not null,
  path varchar(256) not null,
  value varchar(256),
  hidden bool,
  primary key (folder_id, path)
);

/** 
 * S E T T I N G   S T O R A G E
 */
create table storage(
  storage_id int4 not null primary key
);

create sequence storage_seq;

/**
 * S E T T I N G
 */
create table setting(
  storage_id int4 not null,
  path varchar(256) not null,
  value varchar(256) not null,
  primary key (storage_id, path)
);

create sequence setting_seq;

/**
 * E N D P O I N T
 */
create table endpoint(
  endpoint_id int4 not null primary key, 
  serial_number varchar(256) not null,
  name varchar(256),
  phone_id varchar(256) not null,
  storage_id int4,
  folder_id int4 not null
);

/* 
 * would like to add FK contraint to setting table
 * but setting_id can be null and I don't think thats
 * possible w/postgres
alter table endpoint
add constraint fk_endpoint_1
foreign key (storage_id) references storage (storage_id);
 */
 
create sequence endpoint_seq;
create unique index idx_endpoint_sernum on endpoint (serial_number);

alter table line
add constraint fk_endpoint_2
foreign key (folder_id) references folder (folder_id);

/* 
 * L I N E 
 */
create table line(
  line_id int4 not null primary key,
  position int4 not null,
  user_id int4 not null,
  storage_id int4,
  folder_id int4 not null,
  endpoint_id int4 not null
);
create sequence line_seq;
/*
 * relax this, otherwise adjusting positions gets messy
create unique index idx_line_pos_endpt on line (endpoint_id, position);
 */

alter table line
add constraint fk_line_1 
foreign key (endpoint_id) references endpoint (endpoint_id);

alter table line
add constraint fk_line_2
foreign key (folder_id) references folder (folder_id);

/* 
 * would like to add FK contraint to setting table
 * but setting_id can be null and I don't think thats
 * possible w/postgres
alter table line
add constraint fk_line_2
foreign key (storage_id) references storage (storage_id);
*/

/** may relax if user null ok */
alter table line
add constraint fk_line_3
foreign key (user_id) references users (id);

