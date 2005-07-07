<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <!-- begin processing -->
    <xsl:template match="/">
        <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="1">
            <xsl:apply-templates select="destinations/dialplans"/>
        </table>
    </xsl:template>

    <!--  path to take for normal Gateways -->
    <xsl:template match="dialplans">
        <tr>
            <th colspan="2">Dial Plans</th>
        </tr>
        <xsl:apply-templates select="dialplan"/>
    </xsl:template>

    <xsl:template match="dialplan">
        <xsl:for-each select=".">
            <tr>
                <td>
                    <xsl:value-of select="description"/>
                </td>
                <td width="259">
                    <input
                        type="text"
                        name="{name}"
                        value="{value}"
                        onchange="setFlag()"/>
                </td>
            </tr>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>