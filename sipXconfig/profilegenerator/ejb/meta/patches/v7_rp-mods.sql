INSERT INTO REF_PROPERTIES (ID, CONTENT, PROFILE_TYPE, NAME, CODE )
VALUES		(1700001,
		'<definition name="ENCRYPT_PROFILES" mobility="false" cardinality="0..1" description="Encrypt Phone Profiles">
       			<value name="ENCRYPT_PROFILES" type="enumeration" cardinality="1" visibility="write" description="Encrypt Phone Profiles">
            			<default_value>DISABLE</default_value>
            			<enum_value>DISABLE</enum_value>
            			<enum_value>ENABLE</enum_value>
       			</value>
    		</definition>',
		1,
		'ENCRYPT_PROFILES',                 
		'xp_1047');

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1700001, 1, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1700001, 3, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1700001, 4, 0, 0 );


