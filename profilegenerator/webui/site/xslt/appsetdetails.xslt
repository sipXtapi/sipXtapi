<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html"/>

  <!-- begin processing -->
  <xsl:template match="/">
  <div class="formtext">Application Set: <xsl:value-of select="/applicationset/name"/></div>
    <hr align="left" class="dms" width="580"/>
    <xsl:apply-templates select="/applicationset"/>
  </xsl:template>

    <!-- match template "attributes", this is used for the General Tab -->
    <xsl:template match="applicationset">
        <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="480">
            <tr>
                <th>Attribute</th>
                <th>Value</th>
            </tr>
            <tr>
                <td>Name</td>
                <td>
                <input type="text" name="name" value="{name}" size="60" onblur="setFlag()">
                </input>
                </td>
            </tr>
        </table>
        <br/>
        <div align="left" class="formtext">
            Members
        </div>
        <hr align="left" class="dms" width="580"/>
        <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="480">
            <tr>
                <th>Application</th>
                <th>URL</th>
            </tr>
            <xsl:apply-templates select="members"/>
        </table>
    </xsl:template>

    <xsl:template match="members">
        <xsl:for-each select="application">
            <tr>
                <td>
                    <xsl:value-of select="name"/>
                </td>
                <td>
                    <xsl:value-of select="url"/>
                </td>
            </tr>
        </xsl:for-each>
    </xsl:template>
</xsl:stylesheet>
