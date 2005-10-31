<?xml version="1.0" encoding="UTF-8"?>
<!--
  - Add hooks for loading spring beans.
  -->
<xsl:stylesheet 
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns="http://xml.apache.org/axis/wsdd/" 
	xmlns:java="http://xml.apache.org/axis/wsdd/providers/java"
	version="1.0">
<xsl:output method="xml" />

<xsl:apply-templates/>
	
<xsl:template match="deployment">
  <!-- put namespaces back-in, axis will complain -->
  <deployment xmlns="http://xml.apache.org/axis/wsdd/" xmlns:java="http://xml.apache.org/axis/wsdd/providers/java">
	<xsl:apply-templates/>	
  </deployment>		
</xsl:template>
				
<!-- Insert Spring Hooks -->
<xsl:template match="service[@style='document']">
 <service provider="Handler" style="document" use="literal">	
 <xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute>
 <xsl:apply-templates/>
 <parameter name="handlerClass" value="com.workingmouse.webservice.axis.SpringBeanRPCProvider"/>
 <parameter name="springBean">
	 <xsl:attribute name="value">
		 <!-- Must name bean same as service -->
		 <xsl:value-of select="@name"/>
	 </xsl:attribute>
 </parameter>
 </service>	 
</xsl:template>	
	
<xsl:template match='text()|*'>
  <xsl:copy>
    <xsl:copy-of select="@*"/>
    <xsl:apply-templates/>
  </xsl:copy>
</xsl:template>

</xsl:stylesheet>
