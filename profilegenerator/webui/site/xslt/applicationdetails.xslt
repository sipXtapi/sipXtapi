<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html"/>

  <!-- begin processing -->
  <xsl:template match="/">
  <div class="formtext">Application: <xsl:value-of select="/application/details/name"/></div>
    <hr align="left" class="dms" width="580"/>
    <xsl:apply-templates select="/application/details"/>
  </xsl:template>

    <!-- match template "attributes", this is used for the General Tab -->
    <xsl:template match="details">
        <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="580">
            <tr>
                <th>Attribute</th>
                <th>Value</th>
            </tr>
            <tr>
                <td>Name</td>
                <td>
                <input type="text" name="name" value="{name}" size="80" onblur="setFlag()">
                </input>
                </td>
            </tr>
            <tr>
                <td>URL</td>
                <td>
                <input type="text" name="url" value="{url}" size="80" onblur="setFlag()">
                </input>
                </td>
            </tr>
            <tr>
                <td>Description</td>
                <td>
                <textarea name="description" cols="60" rows="4" onblur="setFlag()">
                <xsl:value-of select="description"/>
                </textarea>
                </td>
            </tr>
        </table>
    </xsl:template>
</xsl:stylesheet>
