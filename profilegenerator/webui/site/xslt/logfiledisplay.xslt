<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html"/>

  <!-- begin processing -->
  <xsl:template match="/">
    <table border="0" cellpadding="4" cellspacing="1" >
        <tr>
            <th>Date</th>
            <th>Server</th>
            <th>Severity</th>
            <!--<th>Host</th>-->
            <th>Message</th>
        </tr>
        <xsl:apply-templates select="/root/event"/>
    </table>
  </xsl:template>

    <!-- match template "attributes", this is used for the General Tab -->
    <xsl:template match="event">

        <tr>
            <td><xsl:value-of select="@datetime"/></td>
            <td><xsl:value-of select="@facility"/></td>
            <td><xsl:value-of select="@priority"/></td>
            <!--<td><xsl:value-of select="@hostname"/></td>-->
            <td><xsl:value-of select="message"/></td>
        </tr>


    </xsl:template>
</xsl:stylesheet>
