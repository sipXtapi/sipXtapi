/* v8_mods */
drop table line cascade;
drop sequence line_seq;
drop table phone cascade;
drop table setting cascade;
drop sequence setting_seq;
drop table storage;
drop sequence storage_seq;
drop table meta_storage;
drop sequence meta_storage_seq;
drop table setting_meta;
drop table folder;
drop sequence folder_seq;
drop table folder_setting;

/* v8_mods-2 */
drop table ring cascade;
drop sequence ring_seq;
drop table gateway cascade;
drop sequence gateway_seq;

drop table dialing_rule cascade;
drop table dial_pattern cascade;
drop table international_dialing_rule;
drop table local_dialing_rule;
drop table custom_dialing_rule cascade;
drop table custom_dialing_rule_permission;
drop table dialing_rule_gateway;
drop table emergency_dialing_rule;
drop table internal_dialing_rule;
drop table long_distance_dialing_rule;
drop sequence dialing_rule_seq;

