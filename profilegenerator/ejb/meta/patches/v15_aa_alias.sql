alter table internal_dialing_rule
add column auto_attendant_aliases varchar(256);

/* by default all rows represent Park orbits */
update internal_dialing_rule set auto_attendant_aliases='operator, 0';

