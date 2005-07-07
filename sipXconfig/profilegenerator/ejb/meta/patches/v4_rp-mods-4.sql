UPDATE	REF_PROPERTIES
SET		CONTENT = '<definition name="PHONESET_NO_ANSWER_TIMEOUT" mobility="false" cardinality="0..1" description="Forward on no answer within (seconds)"><value name="PHONESET_NO_ANSWER_TIMEOUT" type="integer" cardinality="1" visibility="write" description="Forward on no answer within (seconds)"><default_value>24</default_value><max_value>30</max_value><min_value>6</min_value></value></definition>'
WHERE		CODE = 'xp_2007';


