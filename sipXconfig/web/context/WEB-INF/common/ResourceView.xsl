<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:template match="/">
  <xsl:apply-templates mode="content"/>	
</xsl:template>

<xsl:template match='text()|*' mode="content">
  <xsl:copy>
    <xsl:copy-of select="@*"/>
    <xsl:apply-templates  mode="content"/>
  </xsl:copy>
</xsl:template>

</xsl:stylesheet>
