-- no need for external number any more: it's kept in settings
alter table users drop column external_number;

-- additional gateway columns
alter table gateway 
	add column anonymous boolean not null,
	add column ignore_user_info boolean not null,
	add column transform_user_extension boolean not null,
	add column add_prefix varchar(255),
	add column keep_digits integer not null;
	
alter table gateway
	alter column anonymous set default false,
	alter column ignore_user_info set default false,
	alter column transform_user_extension set default false,
	alter column keep_digits set default 0;	

-- initialize default values
update gateway set
	anonymous=false,
	ignore_user_info=false,
	transform_user_extension=false,
	keep_digits=0;






