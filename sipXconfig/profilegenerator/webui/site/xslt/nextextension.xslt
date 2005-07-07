<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
   <xsl:output method="html"/>
   <!-- begin processing -->
   <xsl:template match="/">
      <select name="nextext" onchange="nextExt(this.value)" disabled="true">
          <option selected="true" value="">--Extension Pools --</option>
         <xsl:apply-templates select="extensionpools/extensionpool">
            <xsl:sort select="name"/>
         </xsl:apply-templates>
      </select>
   </xsl:template>

   <!-- output information for a group and recursively select
   all children -->
    <xsl:template match="extensionpool">
        <xsl:if test="nextavailable != ''">
            <xsl:if test="name != 'reserved'">
               <option value="{nextavailable}"><xsl:value-of select="name"/></option>
            </xsl:if>
        </xsl:if>
    </xsl:template>
</xsl:stylesheet>

