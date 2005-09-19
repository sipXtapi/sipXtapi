create table user_name (
   user_name_id int4 not null,
   name varchar(255) unique not null,
   primary key (user_name_id)
);

alter table user_alias drop constraint user_alias_pkey;
alter table user_alias drop column alias cascade;
alter table user_alias add column user_name_id int4 not null;
alter table user_alias add primary key (user_id, user_name_id);
alter table user_alias add constraint user_alias_fk2 foreign key (user_name_id) references user_name;
create sequence user_name_seq;

alter table users drop column user_name cascade;
alter table users add column user_name_id int4 not null;
alter table users add constraint user_fk2 foreign key (user_name_id) references user_name;
