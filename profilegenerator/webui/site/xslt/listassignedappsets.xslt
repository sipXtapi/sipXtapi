<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>

    <!-- begin processing -->
    <xsl:template match="/">
       <xsl:apply-templates select="items/applicationset">
                <xsl:sort select="name"/>
        </xsl:apply-templates>
     </xsl:template>

    <xsl:template match="applicationset">
        <tr>
            <td class="list"><xsl:value-of select="name"/></td>
        </tr>
    </xsl:template>
</xsl:stylesheet>