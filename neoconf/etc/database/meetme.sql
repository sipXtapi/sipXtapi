create table meetme_participant (
    meetme_participant_id int4 not null,
    enabled bool,
    value_storage_id int4,
    user_id int4 not null,
	meetme_conference_id int4 not null,    
    primary key (meetme_participant_id)
);

create table meetme_conference (
    meetme_conference_id int4 not null,
    enabled bool,    
    name varchar(255) not null,
    description varchar(255),
    extension varchar(255),
    value_storage_id int4,
    meetme_bridge_id int4 not null,
    primary key (meetme_conference_id)
);

create table meetme_bridge (
    meetme_bridge_id int4 not null,
    enabled bool,
    name varchar(255) not null,
    host varchar(255),
    port int4,
    description varchar(255),
    value_storage_id int4,
    primary key (meetme_bridge_id)
);

alter table meetme_conference
    add constraint fk_meetme_conference_bridge
    foreign key (meetme_bridge_id) 
    references meetme_bridge;
    
alter table meetme_participant
    add constraint fk_meetme_participant_conference
    foreign key (meetme_conference_id) 
    references meetme_conference;
    
alter table meetme_participant 
    add constraint fk_meetme_participant_user
    foreign key (user_id) 
    references users;

create sequence meetme_seq;
