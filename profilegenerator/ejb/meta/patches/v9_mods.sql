/** 
 *   W A R N I N G  :   
 *   exact copy maintained in profilegen_create.sql
 */

create table user_ring (
   user_ring_id int4 not null,
   position int4,
   expiration int4,
   ring_type varchar(255),
   call_group_id int4 not null,
   user_id int4 not null,
   primary key (user_ring_id)
);

create table call_group (
   call_group_id int4 not null,
   enabled bool,
   name varchar(255),
   extension varchar(40),
   lineName varchar(255),
   primary key (call_group_id)
);

alter table user_ring add constraint user_ring_fk_call_group foreign key (call_group_id) references call_group;

--- problems with users table
--- alter table user_ring add constraint user_ring_fk_users foreign key (user_id) references users;

create sequence call_group_seq;
