alter table users
  add column value_storage_id int4;
  
create table user_group(
   user_id int4 not null,
   group_id int4 not null,
   primary key (user_id, group_id)
);  

alter table users add constraint user_fk1 foreign key (value_storage_id) references value_storage;
alter table user_group add constraint user_group_fk1 foreign key (user_id) references users;
alter table user_group add constraint user_group_fk2 foreign key (group_id) references group_storage;

insert into initialization_task (name) values ('add_default_user_group');
