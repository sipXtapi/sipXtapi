create table attendant_dialing_rule (
   attendant_dialing_rule_id int4 not null,
   attendant_aliases varchar(255),
   extension varchar(255),
   after_hours_attendant_id int4 not null,
   after_hours_attendant_enabled bool,
   
   holiday_attendant_id int4 not null,
   holiday_attendant_enabled bool,
   
   working_time_attendant_id int4 not null,
   working_time_attendant_enabled bool,
   
   primary key (attendant_dialing_rule_id)
);

create table attendant_working_hours (
   attendant_dialing_rule_id int4 not null,
   index int4 not null,
   enabled bool,
   day varchar(255),
   start timestamp,
   stop timestamp,
   primary key (attendant_dialing_rule_id, index)
);

create table holiday_dates (
   attendant_dialing_rule_id int4 not null,
   position int4 not null,
   date timestamp,
   primary key (attendant_dialing_rule_id, position)
);

alter table attendant_dialing_rule 
  add constraint fk_attendant_dialing_rule_dialing_rule 
  foreign key (attendant_dialing_rule_id) 
  references dialing_rule;

alter table attendant_working_hours 
  add constraint fk_attendant_working_hours_attendant_dialing_rule 
  foreign key (attendant_dialing_rule_id) 
  references attendant_dialing_rule;

alter table holiday_dates 
  add constraint fk_holiday_dates_attendant_dialing_rule 
  foreign key (attendant_dialing_rule_id) 
  references attendant_dialing_rule;
