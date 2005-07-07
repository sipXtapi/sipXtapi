
create table backup_plan(
  backup_plan_id int4 not null,
  configs boolean not null default true,
  voicemail boolean not null default true,
  dbase boolean not null default true,
  limited_count int4,
  primary key (backup_plan_id)
);

create sequence backup_plan_seq;

create table daily_backup_schedule(
  daily_backup_schedule_id int4 not null,
  backup_plan_id int4 not null,
  enabled boolean not null default false,
  time_of_day timestamp not null,
  scheduled_day varchar(16) not null,
  primary key (daily_backup_schedule_id)
);

create sequence daily_backup_schedule_seq;
