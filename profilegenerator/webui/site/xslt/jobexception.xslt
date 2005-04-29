<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:apply-templates select="job"/>
    </xsl:template>

    <xsl:template match="job">
        <p class="formtext">
            <xsl:value-of select="exception"/>
        </p>
    </xsl:template>

</xsl:stylesheet>
