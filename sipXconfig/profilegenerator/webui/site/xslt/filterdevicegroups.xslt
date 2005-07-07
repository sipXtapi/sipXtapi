<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html"/>
  <!-- begin processing -->
    <xsl:template match="/">
    <select name="filterSel">
        <option value="0">All Groups</option>
        <xsl:apply-templates select="groups/group">
            <xsl:sort select="name"/>
        </xsl:apply-templates>
        </select>
        <a href="#" onMouseOut="MM_swapImgRestore()" onclick="filterGroups()" onMouseOver="MM_swapImage('sm_btn_ass{id}','','../buttons/go_btn2_f2.gif',1)" onFocus="if(this.blur)this.blur()" >
            <img name="sm_btn_ass{id}" src="../buttons/go_btn2.gif" width="23" height="17" border="0"></img>
        </a>
    </xsl:template>

    <xsl:template match="group">
        <!-- <xsl:if test="child::member"> -->
            <option value="{id}">
                <xsl:apply-templates select="." mode="getTree"/>
            </option>
        <!-- </xsl:if> -->
        <xsl:apply-templates select="group"/>
    </xsl:template>

    <xsl:template match="group" mode="getTree">
        <xsl:for-each select="ancestor-or-self::*/name">
            <xsl:choose>
                <xsl:when test="position()!=last()">
                    <xsl:value-of select="."/>
                    <xsl:text>  -&gt;  </xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="."/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>
