<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>
    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:apply-templates select="applications_list/categories"/>
    </xsl:template>

    <xsl:template match="categories">
        <select name="categories" onblur="setCat(this.value)">
            <xsl:for-each select="category">
                <option value="cat_{name}"><xsl:value-of select="name"/></option>
            </xsl:for-each>
        </select>
        <a href="#" onclick="showCat()">
            <img name="sm_btn_go" src="../buttons/go_btn2.gif" width="23" height="17" border="0" ></img>
        </a>
    </xsl:template>
</xsl:stylesheet>