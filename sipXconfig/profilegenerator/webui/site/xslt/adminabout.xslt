<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html"/>

  <!-- begin processing -->
  <xsl:template match="/">

      <table border="0" cellpadding="4" cellspacing="0" class="bglist" width="600">
        <tr>
            <th>Version</th>
        </tr>
        <tr>
            <td align="center"><xsl:value-of select="/dbversioninfo/currentversion"/></td>
        </tr>
      </table>
      <xsl:apply-templates select="/dbversioninfo/appliedpatches"/>
  </xsl:template>

    <!-- match template "attributes", this is used for the General Tab -->
    <xsl:template match="appliedpatches">
        <br/>
        <table border="0" cellpadding="6" cellspacing="0" class="bglist" width="600">
            <tr>
                <th colspan="3">Applied Patches</th>
            </tr>
            <tr>
                <th align="left">Patch</th>
                <th>Description</th>
                <th align="right">Status</th>
            </tr>
        </table>
        <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="600">
            <xsl:for-each select="patch">
            <tr>
                <td width="75" align="center"><xsl:value-of select="number"/></td>
                <td width="450"><xsl:value-of select="description"/></td>
                <td width="75" align="center"><xsl:value-of select="status"/></td>
            </tr>
            </xsl:for-each>
        </table>
        <p class="formtext">Status Key: A = applied, S = successful, F = failed, N = not run</p>

   </xsl:template>
</xsl:stylesheet>
