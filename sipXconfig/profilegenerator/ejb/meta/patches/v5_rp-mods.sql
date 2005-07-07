UPDATE	REF_PROPERTIES
SET CONTENT = '<definition name="PERMISSIONS" mobility="false" cardinality="0..1" description="User permissions">
   <value name="PERMISSIONS" type="container" cardinality="1" visibility="write">
      <value name="_900Dialing" type="enumeration" cardinality="1" visibility="write" description="User can dial 900 numbers">
         <default_value>ENABLE</default_value>

         <enum_value>DISABLE</enum_value>

         <enum_value>ENABLE</enum_value>
      </value>

      <value name="AutoAttendant" type="enumeration" cardinality="1" visibility="write" description="List user in Auto Attendant">
         <default_value>ENABLE</default_value>

         <enum_value>DISABLE</enum_value>

         <enum_value>ENABLE</enum_value>
      </value>

      <value name="InternationalDialing" type="enumeration" cardinality="1" visibility="write" description="User can dial international numbers">
         <default_value>ENABLE</default_value>

         <enum_value>DISABLE</enum_value>

         <enum_value>ENABLE</enum_value>
      </value>

      <value name="LocalDialing" type="enumeration" cardinality="1" visibility="write" description="User can dial local numbers">
         <default_value>ENABLE</default_value>

         <enum_value>DISABLE</enum_value>

         <enum_value>ENABLE</enum_value>
      </value>

      <value name="LongDistanceDialing" type="enumeration" cardinality="1" visibility="write" description="User can dial long distance numbers">
         <default_value>ENABLE</default_value>

         <enum_value>DISABLE</enum_value>

         <enum_value>ENABLE</enum_value>
      </value>

      <value name="Voicemail" type="enumeration" cardinality="1" visibility="write" description="User has voicemail inbox">
         <default_value>ENABLE</default_value>

         <enum_value>DISABLE</enum_value>

         <enum_value>ENABLE</enum_value>
      </value>

      <value name="ForwardCallsExternal" type="enumeration" cardinality="1" visibility="write" description="User can forward calls to external numbers">
         <default_value>DISABLE</default_value>

         <enum_value>DISABLE</enum_value>

         <enum_value>ENABLE</enum_value>
      </value>

      <value name="RecordSystemPrompts" type="enumeration" cardinality="1" visibility="write" description="User can record system prompts">
         <default_value>DISABLE</default_value>

         <enum_value>DISABLE</enum_value>

         <enum_value>ENABLE</enum_value>
      </value>
   </value>
</definition>'
WHERE		CODE = 'xp_10000';


UPDATE	REF_PROPERTIES
SET CONTENT = 
'<definition name="ALLOW_CONSOLE_OUTPUT" mobility="false" cardinality="0..1" description="Activate console logging">
      <value name="ALLOW_CONSOLE_OUTPUT" type="enumeration" cardinality="1" visibility="write" description="Activate console logging">
         <default_value>DISABLE</default_value>
         <enum_value>DISABLE</enum_value>
         <enum_value>ENABLE</enum_value>
      </value>
   </definition>'
WHERE		CODE = 'xp_1028';


INSERT INTO	REF_PROPERTIES (ID, CONTENT, PROFILE_TYPE, NAME, CODE )
VALUES		(1500000,
		'<definition name="PHONESET_MUSIC_ON_HOLD" mobility="false" cardinality="0..1" description="Music on hold">
       			<value name="PHONESET_MUSIC_ON_HOLD" type="enumeration" cardinality="1" visibility="write" description="Music on hold">
            			<default_value>DISABLE</default_value>
            			<enum_value>DISABLE</enum_value>
            			<enum_value>ENABLE</enum_value>
       			</value>
    		</definition>',
		2,
		'PHONESET_MUSIC_ON_HOLD',
		'xp_2035');

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500000, 1, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500000, 2, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500000, 3, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500000, 4, 0, 0 );


INSERT INTO	REF_PROPERTIES (ID, CONTENT, PROFILE_TYPE, NAME, CODE )
VALUES		(1500001,
		'<definition name="PHONESET_MUSIC_ON_HOLD_SOURCE" mobility="false" cardinality="0..1" description="Music on hold source">
		       <value name="PHONESET_MUSIC_ON_HOLD_SOURCE" type="string" cardinality="1" visibility="write" description="Music on hold source" />
		</definition>',
		2,
		'PHONESET_MUSIC_ON_HOLD_SOURCE',
		'xp_2036');

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500001, 1, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500001, 2, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500001, 3, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500001, 4, 0, 0 );


INSERT INTO	REF_PROPERTIES (ID, CONTENT, PROFILE_TYPE, NAME, CODE )
VALUES		(1500002,
		'<definition name="PHONSET_SPASH_SOUND" mobility="false" cardinality="0..1" description="Play Splash Sound">
       			<value name="PHONSET_SPASH_SOUND" type="enumeration" cardinality="1" visibility="write" description="Play Splash Sound">
            			<default_value>DISABLE</default_value>
            			<enum_value>DISABLE</enum_value>
            			<enum_value>ENABLE</enum_value>
       			</value>
    		</definition>',
		1,
		'PHONSET_SPASH_SOUND',
		'xp_1045');

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500002, 1, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500002, 2, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500002, 3, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500002, 4, 0, 0 );

INSERT INTO	REF_PROPERTIES (ID, CONTENT, PROFILE_TYPE, NAME, CODE )
VALUES		(1500003,
		'<definition name="PHONESET_PREFS_CALL_HANDLING" mobility="false" cardinality="0..1" description="Show Call Handling on Phone top">
       			<value name="PHONESET_PREFS_CALL_HANDLING" type="enumeration" cardinality="1" visibility="write" description="Show Call Handling on Phone top">
            			<default_value>DISABLE</default_value>
            			<enum_value>DISABLE</enum_value>
            			<enum_value>ENABLE</enum_value>
       			</value>
    		</definition>',
		1,
		'PHONESET_PREFS_CALL_HANDLING',
		'xp_10002');

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500003, 1, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500003, 2, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500003, 3, 0, 0 );

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL )
VALUES ( 1500003, 4, 0, 0 );