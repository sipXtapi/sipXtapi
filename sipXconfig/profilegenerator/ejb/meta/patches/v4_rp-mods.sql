INSERT INTO	REF_PROPERTIES (	ID, CONTENT, PROFILE_TYPE, NAME, CODE )
VALUES		(1000001,
			'<definition name="PHONESET_HTTPS_PORT" mobility="false" cardinality="0..1" description="Secure HTTPS port for embedded web server"><value name="PHONESET_HTTPS_PORT" type="integer" cardinality="1" visibility="write" description="Secure HTTPS port for embedded web server"><default_value>443</default_value><max_value>65535</max_value><min_value>0</min_value></value></definition>',
			1,
			'PRIMARY_LINE',
			'xp_1042');

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1000001, 1, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1000001, 2, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1000001, 3, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1000001, 4, 0, 0 );