<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>

    <!-- begin processing -->
    <xsl:template match="/">
        <p>
            <xsl:apply-templates select="items/applicationset">
                <xsl:sort select="name"/>
            </xsl:apply-templates>
        </p>
     </xsl:template>

    <xsl:template match="applicationset">
        <div style="font-weight: bold">
            <u><xsl:value-of select="name"/></u>
        </div>
        <xsl:for-each select="application">
        <div style="text-indent: 2em">
            <xsl:value-of select="url"/><br/>
        </div>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>
