alter table users
  drop column extension;
  
create table user_alias (
  user_id int4 not null,
  alias varchar(255) not null unique,
  primary key (user_id, alias)
);

alter table user_alias 
  add constraint user_alias_fk1 
  foreign key (user_id) 
  references users;
