<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>

    <!-- begin processing -->
    <xsl:template match="/">
        <select name="applicationid">
            <option>Select Application to Assign</option>
            <xsl:apply-templates select="items/application">
                <xsl:sort select="name"/>
            </xsl:apply-templates>
        </select>
    </xsl:template>

    <xsl:template match="application">
        <option value="{id}">
            <xsl:value-of select="name"/>
        </option>
    </xsl:template>
</xsl:stylesheet>