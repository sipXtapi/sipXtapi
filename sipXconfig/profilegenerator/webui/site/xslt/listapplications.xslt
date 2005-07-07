<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <!-- begin processing -->
    <xsl:template match="/">
        <table border="0" cellpadding="4" width="600" cellspacing="1" class="bglist">
            <tr>
                <th width="200">Name</th>
                <th>Description</th>
                <th>Delete</th>
            </tr>

            <!-- select top level group -->
            <xsl:apply-templates select="items/item">
                <xsl:sort select="name"/>
            </xsl:apply-templates>
        </table>
    </xsl:template>

    <xsl:template match="item">
        <tr>
            <td>
                <a href="application_details.jsp?applicationid={id}">
                    <xsl:value-of select="name"/>
                </a>
            </td>
            <td>
                <xsl:choose>
                    <xsl:when test="description != ''">
                        <xsl:value-of select="description"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:text>&#160;</xsl:text>
                    </xsl:otherwise>
                </xsl:choose>
            </td>
            <!-- Delete Device -->
            <td align="center">
                <a href="#" onMouseOut="MM_swapImgRestore()" onclick="deleteApplication('{id}')" onMouseOver="MM_swapImage('sm_btn_del{id}','','../buttons/sm_btn_f2.gif',1)" onFocus="if(this.blur)this.blur()" >
                    <img name="sm_btn_del{id}" src="../buttons/sm_btn.gif" width="23" height="17" border="0"></img>
                </a>
            </td>
        </tr>
    </xsl:template>
</xsl:stylesheet>