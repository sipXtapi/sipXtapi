/*
 * Add uniqueness constraints to all name columns, except for dialing_rule,
 * which causes problems that we haven't dealt with yet.
 */
alter table patch add constraint uqc_patch_name unique (name);
alter table phone add constraint uqc_phone_name unique (name);
alter table auto_attendant add constraint uqc_auto_attendant_name unique (name);
alter table park_orbit add constraint uqc_park_orbit_name unique (name);
alter table group_storage add constraint uqc_group_storage_name unique (name);
alter table gateway add constraint uqc_gateway_name unique (name);
alter table call_group add constraint uqc_call_group_name unique (name);
