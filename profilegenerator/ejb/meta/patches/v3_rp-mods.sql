UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="PHONESET_BUSY_BEHAVIOR" mobility="false" cardinality="0..1" description="When busy, forward calls or play busy signal"><value name="PHONESET_BUSY_BEHAVIOR" type="enumeration" cardinality="1" visibility="write" description="When busy, forward calls or play busy signal"><default_value>BUSY</default_value><enum_value>BUSY</enum_value><enum_value>FORWARD</enum_value></value></definition>'
WHERE		CODE = 'xp_2001';

UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="PHONESET_MSG_WAITING_SUBSCRIBE" mobility="false" cardinality="0..1" description="Subscribe for voicemail NOTIFY msgs from"><value name="PHONESET_MSG_WAITING_SUBSCRIBE" type="string" cardinality="1" visibility="write" description="Subscribe for voicemail NOTIFY msgs from" /></definition>'
WHERE		CODE = 'xp_2006';

UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="PHONESET_VOICEMAIL_RETRIEVE" mobility="false" cardinality="0..1" description="Retrieve voicemail messages from"><value name="PHONESET_VOICEMAIL_RETRIEVE" type="string" cardinality="1" visibility="write" description="Retrieve voicemail messages from" /></definition>'
WHERE		CODE = 'xp_2009';