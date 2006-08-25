alter table long_distance_dialing_rule add column pstn_prefix_optional bool default true;
alter table long_distance_dialing_rule add column long_distance_prefix_optional bool default false;
