UPDATE	REF_PROPERTIES
SET CONTENT = '<definition name="PHONESET_RINGDOWN" mobility="false" cardinality="0..1" description="Enable ring down">
      			<value name="PHONESET_RINGDOWN" type="enumeration" cardinality="1" visibility="write" description="Enable ring down">
        			<default_value>DISABLE</default_value>
        			<enum_value>DISABLE</enum_value>  
        			<enum_value>IMMEDIATE</enum_value>
        			<enum_value>PROXY</enum_value>
      			</value>
   		</definition>'
WHERE CODE = 'xp_1044';


UPDATE REF_PROPERTIES
SET CONTENT = '<definition name="PHONESET_SPLASH_SOUND" mobility="false" cardinality="0..1" description="Play Splash Sound">
       <value name="PHONESET_SPLASH_SOUND" type="enumeration" cardinality="1" visibility="write" description="Play Splash Sound">
            <default_value>DISABLE</default_value>
            <enum_value>DISABLE</enum_value>
            <enum_value>ENABLE</enum_value>
       </value>
    </definition>'
WHERE CODE = 'xp_1045';


INSERT INTO REF_PROPERTIES (ID, CONTENT, PROFILE_TYPE, NAME, CODE )
VALUES		(1500004,
		'<definition name="PHONESET_RINGDOWN_ADDRESS" mobility="false" cardinality="0..1" description="Ring down address">
      			<value name="PHONESET_RINGDOWN_ADDRESS" type="string" cardinality="1" visibility="write" description="Ring down address" />
   		</definition>',
		1,
		'PHONESET_RINGDOWN_ADDRESS',                 
		'xp_1046');

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500004, 1, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500004, 2, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500004, 3, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500004, 4, 0, 0 );


