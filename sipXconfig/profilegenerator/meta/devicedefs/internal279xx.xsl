<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method = "text" />

	<xsl:template match="PROFILE">
		<xsl:apply-templates select = "*" />
	</xsl:template>


	<xsl:template match="*">
		<xsl:if test="count(*) = 0">
			<xsl:value-of select="name()"/>
			<xsl:text> : </xsl:text>
			<xsl:value-of select="."/>
			<xsl:call-template name="newline" />
		</xsl:if>
		<xsl:apply-templates select = "*" />
	</xsl:template>



    <xsl:template match="cnf_join_enable/cnf_join_enable">

			<xsl:call-template name="lefthandside">
				<xsl:with-param name="name" select="."/>
			</xsl:call-template>

            <xsl:choose>
                <xsl:when test="normalize-space(.) = '0 (Do not join leaf nodes)'">
                    <xsl:text>0</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>1</xsl:text>
                </xsl:otherwise>
            </xsl:choose>

			<xsl:call-template name="newline" />

	</xsl:template>


	<xsl:template match="dtmf_db_level/dtmf_db_level">

			<xsl:call-template name="lefthandside">
				<xsl:with-param name="name" select="."/>
			</xsl:call-template>

            <xsl:choose>
                <xsl:when test="normalize-space(.) = '1 (6 db below nominal)'">
                    <xsl:text>1</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '2 (3 db below nominal)'">
                    <xsl:text>2</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '3 (Nominal)'">
                    <xsl:text>3</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '4 (3 db above nominal)'">
                    <xsl:text>4</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>5</xsl:text>
                </xsl:otherwise>
            </xsl:choose>

			<xsl:call-template name="newline" />

	</xsl:template>



    <xsl:template match="dtmf_inband/dtmf_inband">

			<xsl:call-template name="lefthandside">
				<xsl:with-param name="name" select="."/>
			</xsl:call-template>

            <xsl:choose>
                <xsl:when test="normalize-space(.) = '0 (Do not generate digits)'">
                    <xsl:text>0</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>1</xsl:text>
                </xsl:otherwise>
            </xsl:choose>

			<xsl:call-template name="newline" />

	</xsl:template>



    <xsl:template match="enable_vad/enable_vad|nat_enable/nat_enable|
        nat_received_processing/nat_received_processing|proxy_register/proxy_register|
        semi_attended_transfer/semi_attended_transfer|autocomplete/autocomplete|
        remote_party_id/remote_party_id|dst_auto_adjust/dst_auto_adjust">

			<xsl:call-template name="lefthandside">
				<xsl:with-param name="name" select="."/>
			</xsl:call-template>

            <xsl:call-template name="enable_disable_to_binary">
				<xsl:with-param name="value" select="."/>
			</xsl:call-template>

			<xsl:call-template name="newline" />

	</xsl:template>



    <xsl:template match="telnet_level/telnet_level">

			<xsl:call-template name="lefthandside">
				<xsl:with-param name="name" select="."/>
			</xsl:call-template>

            <xsl:choose>
                <xsl:when test="normalize-space(.) = '0 (Disabled)'">
                    <xsl:text>0</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '1 (Enabled, no privileged commands)'">
                    <xsl:text>1</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '2 (Enabled, privileged commands)'">
                    <xsl:text>2</xsl:text>
                </xsl:when>
            </xsl:choose>

			<xsl:call-template name="newline" />

	</xsl:template>



    <xsl:template match="time_format_24hr/time_format_24hr">

			<xsl:call-template name="lefthandside">
				<xsl:with-param name="name" select="."/>
			</xsl:call-template>

            <xsl:choose>
                <xsl:when test="normalize-space(.) = '0 (12-hour, can override on phone)'">
                    <xsl:text>0</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '1 (24-hour, can override on phone)'">
                    <xsl:text>1</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '2 (12-hour)'">
                    <xsl:text>2</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '3 (24-hour)'">
                    <xsl:text>3</xsl:text>
                </xsl:when>
            </xsl:choose>

			<xsl:call-template name="newline" />

	</xsl:template>



    <xsl:template match="tos_media/tos_media">

			<xsl:call-template name="lefthandside">
				<xsl:with-param name="name" select="."/>
			</xsl:call-template>

            <xsl:choose>
                <xsl:when test="normalize-space(.) = '0 (IP_ROUTINE)'">
                    <xsl:text>0</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '1 (IP_PRIORITY)'">
                    <xsl:text>1</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '2 (IP_IMMEDIATE)'">
                    <xsl:text>2</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '3 (IP_FLASH)'">
                    <xsl:text>3</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '4 (IP_OVERIDE)'">
                    <xsl:text>4</xsl:text>
                </xsl:when>
                <xsl:when test="normalize-space(.) = '5 (IP_CRITIC)'">
                    <xsl:text>5</xsl:text>
                </xsl:when>
            </xsl:choose>

			<xsl:call-template name="newline" />

	</xsl:template>



    <xsl:template match="call_waiting/call_waiting|callerid_blocking/callerid_blocking|
    	anonymous_call_block/anonymous_call_block|dnd_control/dnd_control">

			<xsl:call-template name="lefthandside">
				<xsl:with-param name="name" select="."/>
			</xsl:call-template>

            <xsl:call-template name="four_way_enable_disable_to_binary">
				<xsl:with-param name="value" select="."/>
			</xsl:call-template>


			<xsl:call-template name="newline" />

	</xsl:template>


    <xsl:template match="additional_user_settings/additional_user_settings|
        additional_device_settings/additional_device_settings">

        <!-- note: we only want the value of these properties, not the name -->
        <xsl:value-of select="."/>
        <xsl:call-template name="newline" />

    </xsl:template>



	<!--
		Standard templates used by the other templates to carry out routine
		taks.
	-->
	<xsl:template name="lefthandside">
		<xsl:param name="name"/>

		<xsl:value-of select="name()"/>
		<xsl:text> : </xsl:text>

	</xsl:template>

    <xsl:template name="enable_disable_to_binary">
		<xsl:param name="value"/>

		<xsl:choose>
            <xsl:when test="normalize-space($value) = '0 (Disabled)'">
                <xsl:text>0</xsl:text>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>1</xsl:text>
            </xsl:otherwise>
        </xsl:choose>

	</xsl:template>


    <xsl:template name="four_way_enable_disable_to_binary">
		<xsl:param name="value"/>

		<xsl:choose>
			<xsl:when test="normalize-space($value) = '0 (Disabled, can override on phone)'">
	            <xsl:text>0</xsl:text>
	        </xsl:when>
	        <xsl:when test="normalize-space($value) = '1 (Enabled, can override on phone)'">
	            <xsl:text>1</xsl:text>
	        </xsl:when>
	        <xsl:when test="normalize-space($value) = '2 (Disabled)'">
	            <xsl:text>2</xsl:text>
	        </xsl:when>
	        <xsl:when test="normalize-space($value) = '3 (Enabled)'">
	            <xsl:text>3</xsl:text>
	        </xsl:when>
        </xsl:choose>

	</xsl:template>


	<xsl:template name="newline">
		<xsl:text>&#xa;</xsl:text>
	</xsl:template>


</xsl:stylesheet>
