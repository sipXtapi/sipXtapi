create table phonebook (
   phonebook_id int4 not null,      
   external_users_filename varchar(255),
   primary key (phonebook_id)
);

create table phonebook_member (
   phonebook_id int4 not null,      
   group_id int4 not null,      
   primary key (phonebook_id, group_id)
);

create sequence phonebook_seq;
