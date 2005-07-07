<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html"/>

  <!-- begin processing -->
  <xsl:template match="/">
      <xsl:choose>
          <xsl:when test="/extensionpool/attributes/name = 'reserved'">
              <div class="formtext" id="reserved">Extension Pool:  <xsl:value-of select="/extensionpool/attributes/name"/></div>
          </xsl:when>
          <xsl:otherwise>
              <div class="formtext">Extension Pool:  <xsl:value-of select="/extensionpool/attributes/name"/></div>
          </xsl:otherwise>
      </xsl:choose>
      <hr align="left" class="dms" width="580"/>
      <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="580">
            <tr>
                <th>Attribute</th>
                <th>Value</th>
            </tr>
            <tr>
                <td width="100">Name</td>
                <xsl:choose>
                    <xsl:when test="/extensionpool/attributes/name != 'reserved'">
                        <td width="480">
                        <form action="edit_extpool.jsp" method="post" name="frm1">
                            <input type="text" name="name" value="{/extensionpool/attributes/name}" size="60" onblur="setFlag()"/>
                            <input type="hidden" name="extensionpoolid" value="{/extensionpool/attributes/id}"/>
                        </form>
                        </td>
                    </xsl:when>
                    <xsl:otherwise>
                        <td class="readonly" width="480">
                        <xsl:value-of select="/extensionpool/attributes/name"/>
                        </td>
                    </xsl:otherwise>
                </xsl:choose>
            </tr>
        </table>
        <br/>
      <div align="left" class="formtext">
            Extensions
        </div>
        <hr align="left" class="dms" width="580"/>
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
        <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="480">
            <tr>
                <th width="240">Min</th>
                <th>Max</th>
            </tr>
            <xsl:for-each select="extensionrange">
                <tr>
                    <td width="240" align="center"><xsl:value-of select="min"/></td>
                    <td align="center"><xsl:value-of select="max"/></td>
                </tr>
            </xsl:for-each>
        </table>
    </xsl:template>
</xsl:stylesheet>
