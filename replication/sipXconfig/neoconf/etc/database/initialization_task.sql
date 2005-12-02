create table initialization_task (
  name varchar(255) not null primary key
); 

/* retro-add initialization items that used legacy interface */
insert into initialization_task (name) values ('dial-plans');
insert into initialization_task (name) values ('default-phone-group');
