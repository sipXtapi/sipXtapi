<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>

    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:choose>
            <xsl:when test="count(/items/application/name) &lt; 10">
                <select name="applicationid" size="{count(/items/application/name) + 1}">
                    <option>Select Application to Unassign</option>
                    <xsl:apply-templates select="items/application">
                        <xsl:sort select="name"/>
                    </xsl:apply-templates>
                </select>
            </xsl:when>
            <xsl:otherwise>
                <select name="applicationid" size="10">
                    <option>Select Application to Unassign</option>
                    <xsl:apply-templates select="items/application">
                        <xsl:sort select="name"/>
                    </xsl:apply-templates>
                </select>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="application">
        <option value="{id}">
            <xsl:value-of select="name"/>
        </option>
    </xsl:template>
</xsl:stylesheet>