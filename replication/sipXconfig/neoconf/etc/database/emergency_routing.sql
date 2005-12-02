create table emergency_routing (
    emergency_routing_id int4 not null,
    gateway_id int4,
    external_number varchar(255),
    primary key (emergency_routing_id)
);

create table routing_exception (
    routing_exception_id int4 not null,
    gateway_id int4,
    external_number varchar(255),
    callers  varchar(255),
    emergency_routing_id int4,
    primary key (routing_exception_id)
);

alter table emergency_routing
    add constraint fk_emergency_routing_gateway
    foreign key (gateway_id) 
    references gateway;
    
alter table routing_exception
    add constraint fk_routing_exception_gateway
    foreign key (gateway_id) 
    references gateway;

alter table routing_exception
    add constraint fk_emergency_routing_routing_exception
    foreign key (emergency_routing_id)
    references emergency_routing;
