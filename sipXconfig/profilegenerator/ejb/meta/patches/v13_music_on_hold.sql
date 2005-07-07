alter table park_orbit
add column orbit_type character;

/* by default all rows represent Park orbits */
update park_orbit set orbit_type='P';

create index park_orbit_type_idx on park_orbit(orbit_type);

