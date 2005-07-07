<?xml version="1.0" encoding="UTF-8"?>
    <xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <xsl:param name="idtest"/>
    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:apply-templates select="groups/group">
            <xsl:sort select="name"/>
        </xsl:apply-templates>

    </xsl:template>

    <xsl:template match="group">
        <xsl:apply-templates select="." mode="getTree"/>
        <xsl:apply-templates select="group">
            <xsl:sort select="name" data-type="text"/>
        </xsl:apply-templates>
    </xsl:template>


    <xsl:template match="group" mode="getTree">
        <xsl:variable name="currentNode" select="name"/>
        <option value="{id}">
            <xsl:for-each select="ancestor-or-self::*/name">
            <xsl:value-of select="."/>
            <xsl:if test=". != $currentNode">
                <xsl:text>  -&gt;  </xsl:text>
            </xsl:if>
       </xsl:for-each>
        </option>
    </xsl:template>
</xsl:stylesheet>
