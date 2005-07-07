INSERT INTO REF_PROPERTIES (ID, CONTENT, PROFILE_TYPE, NAME, CODE )
VALUES		(1500005,
		'<definition name="dial_template" mobility="false" cardinality="0..1" description="dial_template (Path to dialplan template)"><value name="dial_template" type="string" cardinality="1" visibility="write" description="dial_template (Path to dialplan template)" /></definition>',
		1,
		'dial_template',                 
		'cs_1115');

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500005, 1, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500005, 5, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500005, 6, 0, 0 );



