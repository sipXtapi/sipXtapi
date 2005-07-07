/* Update gateway table to keep value storage/setting references */

alter table gateway add column serial_number varchar(255);
alter table gateway add column factory_id varchar(255);
alter table gateway add column storage_id int4;

update gateway set factory_id='gwGeneric';

alter table gateway add constraint fk_gateway_storage foreign key (storage_id) references storage;
