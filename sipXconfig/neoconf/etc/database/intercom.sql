/*
 * Auto-answer configuration:
 *   prefix - the prefix for placing an auto-answer call
 *   timeout (in milliseconds) - how long the phone rings before the call is automatically answered
 *   code - a string sent to the phone that identifies the auto-answer configuration
 */
create table auto_answer (
   auto_answer_id int4 not null,
   prefix varchar(255) not null,
   timeout int4 not null,
   code varchar(255) not null unique,
   primary key (auto_answer_id)
);

/*
 * auto_answer_phone_group is a join table that links an auto-answer configuration
 * to one or more phone groups
 */
create table auto_answer_phone_group (
   auto_answer_id int4 not null,
   group_id int4 not null,
   primary key (auto_answer_id, group_id)
);


/*
 * Foreign key constraints
 */
 
alter table auto_answer_phone_group
  add constraint auto_answer_phone_group_fk1
  foreign key (auto_answer_id)
  references auto_answer;
  
alter table auto_answer_phone_group
  add constraint auto_answer_phone_group_fk2
  foreign key (group_id)
  references group_storage;
