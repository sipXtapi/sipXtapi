
create table line(
  id int4 not null primary key, 
  user_id int4 not null,
  name varchar(256) not null
);

create sequence line_seq;