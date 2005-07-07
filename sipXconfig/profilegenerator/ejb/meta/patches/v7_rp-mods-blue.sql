INSERT INTO REF_PROPERTIES (ID, CONTENT, PROFILE_TYPE, NAME, CODE )
VALUES		(1700002,
		'<definition name="PHONESET_EXPEDITED_IP_TOS" mobility="false" cardinality="0..1" description="Network Layer 3 QoS">
       <value name="PHONESET_EXPEDITED_IP_TOS" type="string" cardinality="1" visibility="write" description="Network Layer 3 QoS">
            <default_value>184</default_value>
       </value>
    </definition>',
		1,
		'PHONESET_EXPEDITED_IP_TOS',                 
		'xp_1048');

INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL ) VALUES ( 1700002, 1, 0, 0 );
INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL ) VALUES ( 1700002, 2, 0, 0 );
INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL ) VALUES ( 1700002, 3, 0, 0 );
INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL ) VALUES ( 1700002, 4, 0, 0 );

INSERT INTO REF_PROPERTIES (ID, CONTENT, PROFILE_TYPE, NAME, CODE ) VALUES          
                (1700003,
		'<definition name="PHONESET_LOCALE_COUNTRY" mobility="false" cardinality="0..1" description="Device Locale">
		   <value name="PHONESET_LOCALE_COUNTRY" type="string" cardinality="1" visibility="write" description="Device Locale">
		     <default_value>US</default_value>
		   </value>
		 </definition>',
		 1,
		 'PHONESET_LOCALE_COUNTRY',   
		 'xp_1049');
INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL ) VALUES ( 1700003, 1, 0, 0 );
INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL ) VALUES ( 1700003, 2, 0, 0 );
INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL ) VALUES ( 1700003, 3, 0, 0 );
INSERT INTO CS_PROPERTY_PERMISSIONS ( REF_PROP_ID, RCS_ID, IS_READ_ONLY, IS_FINAL ) VALUES ( 1700003, 4, 0, 0 );


													  
