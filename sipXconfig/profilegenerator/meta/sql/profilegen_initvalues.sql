-- create the 3 standard security roles
INSERT INTO roles (name) VALUES('SUPER');
INSERT INTO roles (name) VALUES('ADMIN');
INSERT INTO roles (name) VALUES('END_USER');
INSERT INTO roles (name) VALUES('SDS');

-- IB 8/13/01 - changed ordering of the following two statements as they were causing
-- havoc w/ the FK constraints.

INSERT INTO organizations ( id, name, stereotype )
VALUES ( 1000000, 'install-only', 0);

-- create a test user first_name, last_name with a unique primary key identifier of 'userid'
INSERT INTO users ( first_name, last_name, ug_id, password, id, org_ID, display_id )
VALUES ( 'first_name', 'last_name', null, 'password', 'installer', 1000000 , 'installer');

-- Associate 3 Roles with the test 'userid'
INSERT INTO user_roles ( roles_name, usrs_id ) VALUES( 'SUPER',    'installer');
INSERT INTO user_roles ( roles_name, usrs_id ) VALUES( 'ADMIN',    'installer');
INSERT INTO user_roles ( roles_name, usrs_id ) VALUES( 'END_USER', 'installer');
-- assumes autocommit is on