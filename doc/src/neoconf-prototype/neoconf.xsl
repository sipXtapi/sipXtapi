<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet 
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"
  xmlns:x="http://www.w3.org/1999/xhtml"
  >

<xsl:output method="html" encoding="iso-8859-1" doctype-system="http://www.w3c.org/TR/2000/REC-xhtml1-20000126/DTD/xhtml1-transitional" doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"/>

<xsl:param name="pushed" select="'dummy'"/>

<xsl:param name="wwwroot" select="''"/>

<xsl:param name="navigation" select="''"/>

<xsl:template match="x:html">

<xsl:param name="pushed" select="x:body/attribute::id"/>

<html lang="en" xmlns="http://www.w3.org/1999/xhtml">

  <head>
      <title><xsl:apply-templates select="x:head/x:title/text()"/></title>
      <meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1"/>
	  <script language="Javascript" src="functions.js" type="text/javascript">
		<![CDATA[ /* DLH:hack for IE, needs some body */]]>
	  </script>
      <link type="text/css" rel="stylesheet">
          <xsl:attribute name="href"><xsl:value-of select="$wwwroot"/>sipxconfig.css</xsl:attribute>
      </link> 
      <xsl:apply-templates select="x:head/x:style"/>
  </head>

  <body>

  <div id='banner'>
<h1>sipXconfig</h1>
  </div>

  <div id="navigation">
      <xsl:apply-templates select="document($navigation)/menu">
        <xsl:with-param name="pushed" select="$pushed"/>
      </xsl:apply-templates>
  </div>

  <div id='content'>
    <xsl:apply-templates select='x:body'/>
  </div>

  <div id='footer'>
Copyright<xsl:text disable-output-escaping="yes"><![CDATA[&nbsp;&copy;&nbsp;]]></xsl:text>2004 SIPfoundry, Inc. <xsl:text disable-output-escaping="yes"><![CDATA[&quot;]]></xsl:text>SIPfoundry<xsl:text disable-output-escaping="yes"><![CDATA[&quot;]]></xsl:text> is a trademark and service mark of SIPfoundry, Inc..
  </div>

 </body>

</html>

</xsl:template>


<!--
  - T O P  N A V I G A T I O N
  -->
<xsl:template match="menu">

  <xsl:param name="pushed" select="$pushed"/>

  <ul>
    <xsl:apply-templates>
      <xsl:with-param name="pushed" select="$pushed"/>
    </xsl:apply-templates>
  </ul>

</xsl:template>



<!--
  - N A V I G A T I O N  L I N K S
  -->
<xsl:template match="item">

  <xsl:param name="pushed" select="$pushed"/>
  <li>
    <xsl:choose>
      <xsl:when test="$pushed=@name">
        <xsl:attribute name="id">nav_on</xsl:attribute>
        <xsl:value-of select="."/>
      </xsl:when>

      <xsl:otherwise>
        <a>
          <xsl:attribute name="href"><xsl:value-of select="$wwwroot"/><xsl:value-of select="@href"/></xsl:attribute>
          <xsl:attribute name="title"><xsl:value-of select="@title"/></xsl:attribute>
          <xsl:value-of select="."/>
        </a>
      </xsl:otherwise>
    </xsl:choose>
  </li>
</xsl:template>


<!-- 
  - B O D Y
  -->
<xsl:template match='x:body'>
  <xsl:apply-templates mode="content"/>
</xsl:template>   


<!-- 
  - B O D Y  C O N T E N T: Default rule is to copy elements, attributes and content
  -  for all tags.
  -->
<xsl:template match='text()|*' mode="content">

  <xsl:copy>
    <xsl:copy-of select="@*"/>
    <xsl:apply-templates  mode="content"/>
  </xsl:copy>

</xsl:template>

</xsl:stylesheet>

