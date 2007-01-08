/*
 * Add blf column and set default to false in a way supported by Postgres 7.4
 */
 
alter table speeddial_button add column blf boolean;

update speeddial_button set blf = false;

alter table speeddial_button alter column blf set NOT NULL;
    