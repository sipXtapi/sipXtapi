create table park_orbit(
  park_orbit_id int4 not null,
  enabled bool,
  name varchar(256),
  extension varchar(32),
  description varchar(256),
  music_on_hold varchar(256) not null,
  primary key (park_orbit_id)
);

create sequence park_orbit_seq;
