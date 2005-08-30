-- todo, bail on any error

delete from phone_group;
delete from phone;

-- todo, translate superuser as well
delete from user_group;
delete from users where user_id != 1;

delete from group_storage;
delete from setting_value;
delete from value_storage;

-- U S E R   G R O U P S
create or replace function migrate_user_groups() returns integer as '
declare
  usrgrp record;
  next_id int;
begin

  for usrgrp in select * from dblink(''select id, name from user_groups'') as (id int, name text) loop

    raise debug ''importing user group %...'', usrgrp.name;
 
    insert into value_storage (value_storage_id) values (usrgrp.id);

    -- todo: weights ought to be the lower the depth, the higher to weight
    -- how do you find the depth though?

    insert into group_storage (group_id, name, weight, resource)
        values (usrgrp.id, usrgrp.name, nextval(''group_weight_seq''), ''user'');

    -- todo: user permissions
    
  end loop; 

  -- update value_storage_seq  
  next_id := max(value_storage_id) + 1 from value_storage;
  perform setval(''storage_seq'', next_id);

  return 1;
end;
' language plpgsql;

-- U S E R S
create or replace function migrate_users() returns integer as '
declare
  usr record;
  grp record;
  next_id int;
begin

  -- avoid SDS and superadmin users
  for usr in select * from dblink(''select id, first_name, password, last_name, display_id, extension, ug_id from users where ug_id is not null'') as (id int, first_name text, password text, last_name text, display_id text, extension text, ug_id int) loop

   raise debug ''importing user %...'', usr.display_id;

   insert into users (user_id, first_name, pintoken, last_name, user_name)
      values (usr.id, usr.first_name, usr.password, usr.last_name, usr.display_id);

   -- user group
   insert into user_group (user_id, group_id) values (usr.id, usr.ug_id);

   -- todo report user permissions that cannot be handled

   -- todo update user_seq

  end loop; 

  next_id := max(user_id) + 1 from users;
  perform setval(''user_seq'', next_id);

  return 1;
end;
' language plpgsql;

-- U S E R  G R O U P  H I E R A R C H Y
create or replace function migrate_phone_groups() returns integer as '
declare
begin
  -- add user to 1st parent group
  for grp in select * from dblink(''select u.id, g.parent_ug_id from users u, user_groups g where u.ug_id = g.id and g.parent_ug_id is not null'') as (user_id int, group_id int) loop
    insert into user_group (user_id, group_id) values (grp.user_id, grp.group_id);
  end loop;

  -- add user to 2nd parent group
  for grp in select * from dblink(''select u.id, g.parent_ug_id from users u, user_groups g, user_groups g1 where u.ug_id = g1.id and g1.parent_ug_id = g.id and g.parent_ug_id is not null'') as (user_id int, group_id int) loop
    insert into user_group (user_id, group_id) values (grp.user_id, grp.group_id);
  end loop;

  -- todo add 3rd and 4th level user groups
end;
' language plpgsql;


-- P H O N E  G R O U P S
create or replace function migrate_phone_groups() returns integer as '
declare
  phonegrp record;
  next_id int;
begin

  create temporary table phone_group_migration (pds_group_id int not null, group_id int not null);

  for phonegrp in select * from dblink(''select id, name from phone_groups'') as (id int, name text) loop

    next_id := nextval(''storage_seq'');

    insert into phone_group_migration (pds_group_id, group_id) values (phonegrp.id, next_id);

    insert into value_storage (value_storage_id) values (next_id);

    insert into group_storage (group_id, name, weight, resource)
        values (next_id, phonegrp.name, nextval(''group_weight_seq''), ''phone'');

  end loop; 

  return 1;
end;
' language plpgsql;

-- N O N   P O L Y C O M   P H O N E S
create or replace function migrate_non_polycom_phones() returns integer as '
declare
  phone record;
  my_group_id int;
  my_phone_type record;
begin

  -- translation table for phone types
  create temporary table phone_type_migration ( pds_model text, bean_id text, model_id text);
  insert into phone_type_migration values (''7960'', ''ciscoIp'', ''7960'');
  insert into phone_type_migration values (''7940'', ''ciscoIp'', ''7940'');
  insert into phone_type_migration values (''xpressa_strongarm_vxworks'', ''genericPhone'', null);
  insert into phone_type_migration values (''ixpressa_x86_win32'', ''genericPhone'', null);

  for phone in select * from dblink(''select p.id, p.serial_number, p.pg_id, pt.model from logical_phones p, phone_types pt where p.pt_id = pt.id'') as (id int, serial_number text, pg_id int, pt_model text) loop

    select into my_phone_type * from phone_type_migration where pds_model = phone.pt_model; 

    raise debug ''importing phone %, %, %...'', phone.serial_number, my_phone_type.bean_id;

    insert into phone (phone_id, serial_number, bean_id, model_id) values (phone.id, phone.serial_number, my_phone_type.bean_id, my_phone_type.model_id);

    select into my_group_id group_id from phone_group_migration where pds_group_id = phone.pg_id;

    insert into phone_group (phone_id, group_id) values (phone.id, my_group_id);

    -- todo, phone settings, only cisco? very difficult

  end loop; 

  return 1;
end;
' language plpgsql;


-- P O L Y C O M   P H O N E S 
create or replace function migrate_polycom_phones() returns integer as '
declare
  phone record;
  my_group_id int;
  my_phone_type record;
begin

  for phone in select * from dblink(''select p.phone_id, p.serial_number, p.name, p.factory_id, p.storage_id, p.folder_id from phone p'') as (phone_id int, serial_number text, name text, factory_id text, storage_id int, folder_id int) loop

    -- phone settings
    
  end loop; 

  return 1;
end;
' language plpgsql;


load 'dblink';

select dblink_connect('dbname=PDS');

select migrate_user_groups();
select migrate_users();
select migrate_phone_groups();
select migrate_non_polycom_phones();
select migrate_polycom_phones();


