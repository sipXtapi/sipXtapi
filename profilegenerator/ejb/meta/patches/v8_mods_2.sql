/** 
 *   W A R N I N G  :   
 *   exact copy maintained in profilegen_create.sql
 */

/** 
 * R I N G
 */
create table ring(
  ring_id int4 not null primary key,
  user_id int4 not null,
  number varchar(256),
  expiration int4,
  position int4 not null,
  ring_type varchar(40)
);

create unique index idx_ring_user_ring on ring (user_id, ring_id);
create index idx_ring_user on ring (user_id);

create sequence ring_seq;

alter table ring
add constraint fk_user_1
foreign key (user_id) references users (id);



