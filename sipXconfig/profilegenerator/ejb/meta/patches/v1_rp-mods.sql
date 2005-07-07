UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="PHONESET_DIGITMAP" mobility="false" cardinality="0..N" blank_entries="1" description="Digitmaps"> <value name="PHONESET_DIGITMAP" type="container" cardinality="1" visibility="write" > <value name="NUMBER" type="integer" cardinality="1"  visibility="write" index="true" sort="alphanum-casesensitive-asc" description="Dial Plan"><max_value>9999999999</max_value><min_value>1</min_value></value><value name="TARGET" type="string" cardinality="1" visibility="write" description="SIP Address"></value></value></definition>'
WHERE		CODE = 'xp_1003';


UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="PHONESET_HTTP_AUTH_DB" mobility="false" cardinality="0..N" blank_entries="1" description="Authorized Phone Users"><value name="PHONESET_HTTP_AUTH_DB" type="container" cardinality="1" visibility="write"><value name="USERID" type="string" cardinality="1" visibility="write" index="true" sort="alphanum-casesensitive-asc" description="User Name"><default_value>admin</default_value></value><value name="PASSTOKEN" type="digest" cardinality="1" visibility="write" description="Password"><default_value>1234</default_value></value><value name="REALM" type="string" cardinality="1" visibility="hidden" read_only_value="xpressa"/></value></definition>'
WHERE		CODE = 'xp_1005';

UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="SIP_AUTHENTICATE_DB" mobility="false" cardinality="0..N" blank_entries="1" description="Authorized Callers"><value name="SIP_AUTHENTICATE_DB" type="container" cardinality="1" visibility="write"><value name="USER" type="string" cardinality="1" visibility="write" index="true" sort="alphanum-casesensitive-asc" description="User Name"/><value name="PASSWORD" type="password" cardinality="1" visibility="write" description="Password"/></value></definition>'
WHERE		CODE = 'xp_1018';

UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="ADDITIONAL_DEVICE_PARAMETERS" mobility="false" cardinality="A" description=""><value name="ADDITIONAL_DEVICE_PARAMETERS" type="area" cardinality="1" visibility="write" index="true" description="Additional Settings"/></definition>'
WHERE		CODE = 'xp_1039';

UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="PHONESET_CHECK_SYNC" mobility="false" cardinality="0..1" description="Allow remote restarts"><value name="PHONESET_CHECK_SYNC" type="enumeration" cardinality="1" visibility="write" description="Allow remote restarts"><default_value>DISABLE</default_value><enum_value>DISABLE</enum_value><enum_value>ENABLE</enum_value></value></definition>'
WHERE		CODE = 'xp_1040';

UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="PHONESET_RTP_CODECS" mobility="false" cardinality="0..1" description="Codec priority list"><value name="PHONESET_RTP_CODECS" type="string" cardinality="1" visibility="write" description="Codec priority list"/></definition>'
WHERE		CODE = 'xp_1041';

UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="USER_LINE" mobility="false" cardinality="0..N" blank_entries="1" description="Identify an Individual Phone User by Defining a User Line"><value name="USER_LINE" type="container" cardinality="1" visibility="write"><value name="URL" type="string" cardinality="1" visibility="write" index="true" sort="alphanum-casesensitive-asc" description="SIP URL"><default_value>4444@</default_value></value><value name="REGISTRATION" type="enumeration" cardinality="0..1" visibility="write" description="Register with host or provision line as is"><default_value>PROVISION</default_value><enum_value>PROVISION</enum_value><enum_value>REGISTER</enum_value></value><value name="ALLOW_FORWARDING" type="enumeration" cardinality="0..1" visibility="write" description="Apply rules for call forwarding"><default_value>DISABLE</default_value><enum_value>DISABLE</enum_value><enum_value>ENABLE</enum_value></value><value name="CREDENTIAL" type="container" cardinality="0..N" visibility="write" blank_entries="1" description="Credentials for Outgoing Calls"><value name="REALM" type="string" cardinality="1" visibility="write" description="Realm"></value><value name="USERID" type="string" cardinality="1" visibility="write" description="User Name"></value><value name="PASSTOKEN" type="digest" cardinality="0..1" visibility="write" description="Password"></value></value></value></definition>'
WHERE		CODE = 'xp_2029';

UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="PHONESET_SPEEDDIAL" mobility="false" cardinality="0..N" blank_entries="3" description="Speed Dial Number with its Corresponding Address to Dial, Identifying Label, and Address Type"><value name="PHONESET_SPEEDDIAL" type="container" cardinality="1" visibility="write"><value name="ID" type="integer" cardinality="1" visibility="write" index="true" sort="numeric-asc" description="Speed dial number"></value><value name="LABEL" type="string" cardinality="1" visibility="write" description="Name"></value><value name="TYPE" type="enumeration" cardinality="1" visibility="write" description="Phone number or SIP URL?"><default_value>NUMBER</default_value><enum_value>URL</enum_value><enum_value>NUMBER</enum_value></value><value name="ADDRESS" type="string" cardinality="1" visibility="write" description="Address"></value></value></definition>'
WHERE		CODE = 'xp_2030';


INSERT INTO	REF_PROPERTIES (	ID, CONTENT, PROFILE_TYPE, NAME, CODE )
VALUES		(1000000,
			'<definition name="PRIMARY_LINE" mobility="false" cardinality="0..1" blank_entries="1" description="Identify an Individual Phone User by Defining a Users Primary Line"><value name="PRIMARY_LINE" type="container" cardinality="1" visibility="write"><value name="URL" type="string" cardinality="1" visibility="write" sort="alphanum-casesensitive-asc" description="SIP URL"><default_value>4444@</default_value></value><value name="REGISTRATION" type="enumeration" cardinality="0..1" visibility="write" description="Register with host or provision line as is"><default_value>PROVISION</default_value><enum_value>PROVISION</enum_value><enum_value>REGISTER</enum_value></value><value name="ALLOW_FORWARDING" type="enumeration" cardinality="0..1" visibility="write" description="Apply rules for call forwarding"><default_value>DISABLE</default_value><enum_value>DISABLE</enum_value><enum_value>ENABLE</enum_value></value><value name="CREDENTIAL" type="container" cardinality="0..N" visibility="write" blank_entries="1" description="Credentials for Outgoing Calls"><value name="REALM" type="string" cardinality="1" visibility="write" description="Realm"></value><value name="USERID" type="string" cardinality="1" visibility="write" description="User Name"></value><value name="PASSTOKEN" type="digest" cardinality="0..1" visibility="write" description="Password"></value></value></value></definition>',
			2,
			'PRIMARY_LINE',
			'xp_10001');

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1000000, 1, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1000000, 2, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1000000, 3, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1000000, 4, 0, 0 );


