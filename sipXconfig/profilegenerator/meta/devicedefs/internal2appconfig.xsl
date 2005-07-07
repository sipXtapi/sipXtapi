<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method = "text" />

	<xsl:template match="PROFILE">		
		<xsl:apply-templates select = "*" />		
	</xsl:template>
	

	
		
	<xsl:template match="*">
	
		<xsl:if test="./URL">
			<xsl:value-of select="name()" />
		</xsl:if>
							
		
		<xsl:if test = "count(./*) = 0"> 
			
			<xsl:text>, </xsl:text>
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