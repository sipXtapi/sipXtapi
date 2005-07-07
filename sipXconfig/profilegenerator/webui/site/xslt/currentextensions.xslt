<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html"/>

  <!-- begin processing -->
    <xsl:template match="/">
        <xsl:choose>
            <xsl:when test="/extensionpool/extensions != ''">
                <xsl:apply-templates select="/extensionpool/extensions"/>
            </xsl:when>
            <xsl:otherwise>
                <p class="formtext">No extensions have been defined for this extension pool.</p>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <!-- match template "attributes", this is used for the General Tab -->
    <xsl:template match="extensions">
        <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="280" align="center">
            <tr>
                <th colspan="2">Current Ranges</th>
            </tr>
            <xsl:for-each select="extensionrange">
                <tr>
                    <td width="140" align="center"><xsl:value-of select="min"/></td>
                    <td align="center"><xsl:value-of select="max"/></td>
                </tr>
            </xsl:for-each>
        </table>
    </xsl:template>
</xsl:stylesheet>
