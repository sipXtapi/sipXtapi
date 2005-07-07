<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method = "text" />
<xsl:param name="includedContent"/>

	<xsl:template match="/">		 		
		<xsl:value-of select="$includedContent"/>
		<xsl:call-template name="newline"/>		
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="PROFILE">			
		<xsl:apply-templates select = "USER_LINE" />	
		<xsl:apply-templates select = "*[name()!='USER_LINE']" />
	</xsl:template>


	<xsl:template match="PERMISSIONS">
	</xsl:template>
	
	<xsl:template match="PHONESET_DIGITMAP/PHONESET_DIGITMAP|PHONESET_HTTP_AUTH_DB/PHONESET_HTTP_AUTH_DB">
		<xsl:value-of select="name()"/>
		<xsl:text>.</xsl:text>
		
		<xsl:choose>
			<xsl:when test="name()='PHONESET_DIGITMAP'">
				<xsl:value-of select="NUMBER"/>
				<xsl:text> : </xsl:text>
				<xsl:value-of select="TARGET"/>
			</xsl:when>
			<xsl:when test="name()='PHONESET_HTTP_AUTH_DB'">									
				<xsl:value-of select="USERID"/>
				<xsl:text> : </xsl:text>
				<xsl:value-of select="PASSTOKEN"/>
			</xsl:when>
		  
			<xsl:otherwise>
			 
			</xsl:otherwise>
		</xsl:choose>
		<xsl:call-template name="newline"/>
	</xsl:template>
	
	<xsl:template match="SIP_AUTHENTICATE_DB/SIP_AUTHENTICATE_DB">
		<xsl:value-of select="name()"/>
		<xsl:text>.</xsl:text>			
			
		<xsl:value-of select="USER"/>
		<xsl:text> : </xsl:text>
		<xsl:value-of select="PASSWORD"/>
			
		<xsl:call-template name="newline"/>
	</xsl:template>
		
	
	<xsl:template match="ADDITIONAL_USER_PARAMETERS/ADDITIONAL_USER_PARAMETERS|ADDITIONAL_DEVICE_PARAMETERS/ADDITIONAL_DEVICE_PARAMETERS">
		<xsl:value-of select="text()"/>
		<xsl:call-template name="newline"/>
	</xsl:template>
	
	
	<xsl:template match="PHONESET_SPEEDDIAL">
			
			<xsl:if test="PHONESET_SPEEDDIAL/ID">
				<xsl:text>PHONESET_SPEEDDIAL.</xsl:text>
				<xsl:value-of select="PHONESET_SPEEDDIAL/ID"/><xsl:text>.</xsl:text>
				<xsl:text>ID : </xsl:text>
				<xsl:value-of select="PHONESET_SPEEDDIAL/ID"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
			<xsl:if test="PHONESET_SPEEDDIAL/LABEL">
				<xsl:text>PHONESET_SPEEDDIAL.</xsl:text>
				<xsl:value-of select="PHONESET_SPEEDDIAL/ID"/><xsl:text>.</xsl:text>
				<xsl:text>LABEL : </xsl:text>
				<xsl:value-of select="PHONESET_SPEEDDIAL/LABEL"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
			<xsl:if test="PHONESET_SPEEDDIAL/TYPE">
				<xsl:text>PHONESET_SPEEDDIAL.</xsl:text>
				<xsl:value-of select="PHONESET_SPEEDDIAL/ID"/><xsl:text>.</xsl:text>
				<xsl:text>TYPE : </xsl:text>
				<xsl:value-of select="PHONESET_SPEEDDIAL/TYPE"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>	
			<xsl:if test="PHONESET_SPEEDDIAL/ADDRESS">
				<xsl:text>PHONESET_SPEEDDIAL.</xsl:text>
				<xsl:value-of select="PHONESET_SPEEDDIAL/ID"/><xsl:text>.</xsl:text>
				<xsl:text>ADDRESS : </xsl:text>
				<xsl:value-of select="PHONESET_SPEEDDIAL/ADDRESS"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
	
	</xsl:template>

	<xsl:template match="USER_DEFAULT_OUTBOUND_LINE/USER_DEFAULT_OUTBOUND_LINE">	
	</xsl:template>
	

	
	<xsl:template match="USER_LINE">
		<xsl:variable name="line_number" select="position()+1"/>		
		<xsl:apply-templates select="USER_LINE">
			<xsl:with-param name="line_number" select="$line_number"/>
		</xsl:apply-templates>	
	</xsl:template>
	
	<xsl:template match="USER_LINE/USER_LINE">
		<xsl:param name="line_number"/>	
		
		<xsl:variable name="default_line" select = "/PROFILE/USER_DEFAULT_OUTBOUND_LINE/USER_DEFAULT_OUTBOUND_LINE"/>		
		<xsl:if test="URL = $default_line">
			<xsl:text>USER_DEFAULT_OUTBOUND_LINE : USER_LINE.</xsl:text>
			<xsl:value-of select="$line_number"/>
			<xsl:call-template name="newline"/>			
		</xsl:if>	
		
		<xsl:if test="ALLOW_FORWARDING">
			<xsl:call-template name="simple_line">
				<xsl:with-param name="line_type" select="'USER_LINE'"/>
				<xsl:with-param name="line_number" select="$line_number"/>
				<xsl:with-param name="line_property" select="ALLOW_FORWARDING"/>
				<xsl:with-param name="line_property_name" select="'ALLOW_FORWARDING'"/>
			</xsl:call-template>		
		</xsl:if>
		
		<xsl:if test="REGISTRATION">
			<xsl:call-template name="simple_line">
				<xsl:with-param name="line_type" select="'USER_LINE'"/>
				<xsl:with-param name="line_number" select="$line_number"/>
				<xsl:with-param name="line_property" select="REGISTRATION"/>
				<xsl:with-param name="line_property_name" select="'REGISTRATION'"/>
			</xsl:call-template>		
		</xsl:if>
		
		<xsl:if test="URL">
			<xsl:call-template name="simple_line">
				<xsl:with-param name="line_type" select="'USER_LINE'"/>
				<xsl:with-param name="line_number" select="$line_number"/>
				<xsl:with-param name="line_property" select="URL"/>
				<xsl:with-param name="line_property_name" select="'URL'"/>
			</xsl:call-template>		
		</xsl:if>
		
		<xsl:for-each select="CREDENTIAL">
			
			<xsl:if test="USERID">
				<xsl:text>USER_LINE.</xsl:text>
				<xsl:value-of select="$line_number"/>
				<xsl:text>.CREDENTIAL.</xsl:text>
				<xsl:value-of select="position()"/><xsl:text>.</xsl:text>
				<xsl:text>USERID : </xsl:text>
				<xsl:value-of select="USERID"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
			<xsl:if test="PASSTOKEN">
				<xsl:text>USER_LINE.</xsl:text>
				<xsl:value-of select="$line_number"/>
				<xsl:text>.CREDENTIAL.</xsl:text>
				<xsl:value-of select="position()"/><xsl:text>.</xsl:text>
				<xsl:text>PASSTOKEN : </xsl:text>
				<xsl:value-of select="PASSTOKEN"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
			<xsl:if test="REALM">
				<xsl:text>USER_LINE.</xsl:text>
				<xsl:value-of select="$line_number"/>
				<xsl:text>.CREDENTIAL.</xsl:text>
				<xsl:value-of select="position()"/><xsl:text>.</xsl:text>
				<xsl:text>REALM : </xsl:text>
				<xsl:value-of select="REALM"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
		
		</xsl:for-each>
	
	</xsl:template>
	
	
	<xsl:template match="PRIMARY_LINE/PRIMARY_LINE">		
		
		<xsl:variable name="default_line" select = "/PROFILE/USER_DEFAULT_OUTBOUND_LINE/USER_DEFAULT_OUTBOUND_LINE"/>
				
		<xsl:if test="URL = $default_line">
			<xsl:text>USER_DEFAULT_OUTBOUND_LINE : USER_LINE.1</xsl:text>			
			<xsl:call-template name="newline"/>			
		</xsl:if>	
		
		<xsl:if test="ALLOW_FORWARDING">
			<xsl:call-template name="simple_line">
				<xsl:with-param name="line_type" select="'USER_LINE'"/>
				<xsl:with-param name="line_number" select="1"/>
				<xsl:with-param name="line_property" select="ALLOW_FORWARDING"/>
				<xsl:with-param name="line_property_name" select="'ALLOW_FORWARDING'"/>
			</xsl:call-template>		
		</xsl:if>
		
		<xsl:if test="REGISTRATION">
			<xsl:call-template name="simple_line">
				<xsl:with-param name="line_type" select="'USER_LINE'"/>
				<xsl:with-param name="line_number" select="1"/>
				<xsl:with-param name="line_property" select="REGISTRATION"/>
				<xsl:with-param name="line_property_name" select="'REGISTRATION'"/>
			</xsl:call-template>		
		</xsl:if>
		
		<xsl:if test="URL">
			<xsl:call-template name="simple_line">
				<xsl:with-param name="line_type" select="'USER_LINE'"/>
				<xsl:with-param name="line_number" select="1"/>
				<xsl:with-param name="line_property" select="URL"/>
				<xsl:with-param name="line_property_name" select="'URL'"/>
			</xsl:call-template>		
		</xsl:if>
		
		<xsl:for-each select="CREDENTIAL">
			
			<xsl:if test="USERID">
				<xsl:text>USER_LINE.</xsl:text>
				<xsl:value-of select="1"/>
				<xsl:text>.CREDENTIAL.</xsl:text>
				<xsl:value-of select="position()"/><xsl:text>.</xsl:text>
				<xsl:text>USERID : </xsl:text>
				<xsl:value-of select="USERID"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
			<xsl:if test="PASSTOKEN">
				<xsl:text>USER_LINE.</xsl:text>
				<xsl:value-of select="1"/>
				<xsl:text>.CREDENTIAL.</xsl:text>
				<xsl:value-of select="position()"/><xsl:text>.</xsl:text>
				<xsl:text>PASSTOKEN : </xsl:text>
				<xsl:value-of select="PASSTOKEN"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
			<xsl:if test="REALM">
				<xsl:text>USER_LINE.</xsl:text>
				<xsl:value-of select="1"/>
				<xsl:text>.CREDENTIAL.</xsl:text>
				<xsl:value-of select="position()"/><xsl:text>.</xsl:text>
				<xsl:text>REALM : </xsl:text>
				<xsl:value-of select="REALM"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
		
		</xsl:for-each>
	
	</xsl:template>
	
	
	<xsl:template match="PHONESET_LINE">			
		<xsl:apply-templates select="PHONESET_LINE"/>			
	</xsl:template>
	
	<xsl:template match="PHONESET_LINE/PHONESET_LINE">
		
		
		<xsl:if test="ALLOW_FORWARDING">
			<xsl:call-template name="simple_line">
				<xsl:with-param name="line_type" select="'PHONESET_LINE'"/>
				
				<xsl:with-param name="line_property" select="ALLOW_FORWARDING"/>
				<xsl:with-param name="line_property_name" select="'ALLOW_FORWARDING'"/>
			</xsl:call-template>		
		</xsl:if>
		
		<xsl:if test="REGISTRATION">
			<xsl:call-template name="simple_line">
				<xsl:with-param name="line_type" select="'PHONESET_LINE'"/>
				
				<xsl:with-param name="line_property" select="REGISTRATION"/>
				<xsl:with-param name="line_property_name" select="'REGISTRATION'"/>
			</xsl:call-template>		
		</xsl:if>
		
		<xsl:if test="URL">
			<xsl:call-template name="simple_line">
				<xsl:with-param name="line_type" select="'PHONESET_LINE'"/>
				
				<xsl:with-param name="line_property" select="URL"/>
				<xsl:with-param name="line_property_name" select="'URL'"/>
			</xsl:call-template>		
		</xsl:if>
		
		
		
		<xsl:for-each select="CREDENTIAL">			
			
			<xsl:if test="USERID">
				<xsl:text>PHONESET_LINE.CREDENTIAL.</xsl:text>
				<xsl:value-of select="position()"/><xsl:text>.</xsl:text>
				<xsl:text>USERID : </xsl:text>
				<xsl:value-of select="USERID"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
			<xsl:if test="PASSTOKEN">
				<xsl:text>PHONESET_LINE.CREDENTIAL.</xsl:text>
				<xsl:value-of select="position()"/><xsl:text>.</xsl:text>
				<xsl:text>PASSTOKEN : </xsl:text>
				<xsl:value-of select="PASSTOKEN"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
			<xsl:if test="REALM">
				<xsl:text>PHONESET_LINE.CREDENTIAL.</xsl:text>
				<xsl:value-of select="position()"/><xsl:text>.</xsl:text>
				<xsl:text>REALM : </xsl:text>
				<xsl:value-of select="REALM"/>
				<xsl:call-template name="newline"/>	
			</xsl:if>
		
		</xsl:for-each>
	
	</xsl:template>
	
	<xsl:template name="simple_line">
		<xsl:param name="line_type"/>
		<xsl:param name="line_number"/>
		<xsl:param name="line_property"/>
		<xsl:param name="line_property_name"/>
	
		<xsl:value-of select="$line_type"/><xsl:text>.</xsl:text>
		<xsl:choose>
			<xsl:when test="$line_type = 'USER_LINE'">		
				<xsl:value-of select="$line_number"/>
				<xsl:text>.</xsl:text>
			</xsl:when>		
		</xsl:choose>
		
		<xsl:value-of select="$line_property_name"/><xsl:text> :</xsl:text>
		<xsl:text> </xsl:text>
		<xsl:value-of select="$line_property"/>
		
		<xsl:call-template name="newline"/>	
	</xsl:template>	
	
		
	<xsl:template match="*">					
		
		<xsl:if test = "count(./*) = 0"> 		
			<xsl:for-each select = "ancestor-or-self::*">
				<xsl:if test="name() != 'PROFILE'">
					<xsl:choose>				
						<xsl:when test="../../PROFILE">	
						</xsl:when>
						<xsl:otherwise>
							<xsl:value-of select="name()" />				
							
							<xsl:if test = "count(./*) != 0">
								<xsl:text>.</xsl:text>
							</xsl:if>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:if>
			</xsl:for-each>			
			
			<xsl:if test = "@id">
				<xsl:text>.</xsl:text>
				<xsl:value-of select="@id" />
			</xsl:if>
			<xsl:text> : </xsl:text>
			<xsl:value-of select="node()" />
			<xsl:call-template name="newline"/>	
		</xsl:if>
		
		<xsl:apply-templates select = "*" />
	</xsl:template>
	
	
	<xsl:template name="newline">
		<xsl:text>&#xa;</xsl:text>
		<!--<xsl:text>&#xD;&#xa;</xsl:text>-->
	</xsl:template>
		
</xsl:stylesheet>