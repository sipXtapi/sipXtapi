/** 
 *   W A R N I N G  :   
 *   exact copy maintained in profilegen_create.sql
 */

/** 
 * R I N G
 */
create table ring(
  ring_id int4 not null,
  user_id int4 not null,
  number varchar(256),
  expiration int4,
  position int4 not null,
  ring_type varchar(40),
  primary key (ring_id)
);

create unique index idx_ring_user_ring on ring (user_id, ring_id);
create index idx_ring_user on ring (user_id);

create sequence ring_seq;

alter table ring
add constraint fk_user_1
foreign key (user_id) references users (id);

/**
 * G A T E W A Y
 */
 
create table gateway (
  gateway_id int4 not null,
  name varchar(256),
  address varchar(256),
  description varchar(256),
  primary key (gateway_id)
);

create sequence gateway_seq;


 



