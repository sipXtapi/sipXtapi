<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>

    <!-- begin processing -->
    <xsl:template match="/">
        <p>
            <xsl:apply-templates select="items/application">
                <xsl:sort select="name"/>
            </xsl:apply-templates>
        </p>
     </xsl:template>

    <xsl:template match="application">
        <xsl:value-of select="name"/> - <xsl:value-of select="url"/><br/>
    </xsl:template>
</xsl:stylesheet>