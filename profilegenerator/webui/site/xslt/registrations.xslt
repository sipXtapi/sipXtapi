<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <!-- current time in seconds from epoch -->
    <xsl:param name="now"/>
    <!-- sort holds what element to sort by -->
    <xsl:param name="sort"/>
    <!-- index is the lower end of the view range -->
    <xsl:param name="index"/>
    <!-- max is the upper end of the view range -->
    <xsl:param name="max"/>
    <!-- count is the number of lines to view per page -->
    <xsl:param name="count"/>
    <!-- pages is the number of pages to view, used in nav elements -->
    <xsl:param name="pages">
        <xsl:value-of select="floor(count(/items/item) div $count)"/>
    </xsl:param>
    <!-- total is the total number of items available -->
    <xsl:param name="total" select="count(/items/item)"/>
    <!-- location of the server to view the information from -->
    <xsl:param name="location"/>

    <!-- begin processing -->
    <xsl:template match="/">
        <p class="formtext">Server: <xsl:value-of select="$location"/></p>
        <div class="nav"><xsl:call-template name="BuildNav"/></div>
        <table class="bglist" width="480" border="0" cellpadding="4" cellspacing="1">
            <tr>
                <th><a href="registration_details.jsp?location={$location}&amp;sort=uri&amp;index=1&amp;count={$count}&amp;max=50">URI</a></th>
                <th width="200"><a href="registration_details.jsp?location={$location}&amp;sort=contact&amp;index=1&amp;count={$count}&amp;max=50">Contact</a></th>
                <th><a href="registration_details.jsp?location={$location}&amp;sort=expires&amp;index=1&amp;count={$count}&amp;max=50">Expires (sec)</a></th>
            </tr>
            <xsl:choose>
                <xsl:when test="$sort = 'uri'">
                    <xsl:apply-templates select="items/item">
                        <xsl:sort select="uri" order="ascending" data-type="text"/>
                        <xsl:sort select="expires" order="descending" data-type="number"/>
                    </xsl:apply-templates>
                </xsl:when>
                <xsl:when test="$sort = 'contact'">
                    <xsl:apply-templates select="items/item">
                        <xsl:sort select="contact" order="ascending" data-type="text"/>
                        <xsl:sort select="expires" order="descending" data-type="number"/>
                    </xsl:apply-templates>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:apply-templates select="items/item">
                        <xsl:sort select="expires" order="descending" data-type="number"/>
                    </xsl:apply-templates>
                </xsl:otherwise>
            </xsl:choose>
        </table>
        <div class="nav"><xsl:call-template name="BuildNav"/></div>
    </xsl:template>

    <xsl:template match="item">
        <xsl:param name="i" select="1"/>
        <xsl:if test="($i &gt;= $index) and ($i &lt; $max)">
            <tr>
                <xsl:choose>
                    <xsl:when test="(expires - $now) &gt; 0">
                        <td valign="top"><xsl:value-of select="uri"/></td>
                        <td width="200" valign="top"><xsl:value-of select="contact"/></td>
                        <td valign="top"><xsl:value-of select="expires - $now"/></td>
                    </xsl:when>
                    <xsl:otherwise>
                        <td valign="top"><font color="red"><xsl:value-of select="uri"/></font></td>
                        <td width="200" valign="top"><font color="red"><xsl:value-of select="contact"/></font></td>
                        <td valign="top"><font color="red"><xsl:text>expired</xsl:text></font></td>
                    </xsl:otherwise>
                </xsl:choose>
            </tr>
        </xsl:if>
        <!-- <xsl:with-param name="i" select="$i + 1"/> -->
    </xsl:template>

    <xsl:template name="BuildNav">
        <xsl:param name="currentPage" select="1"/>
        <xsl:if test="$currentPage &gt;= 1 and $currentPage &lt;= $pages + 1">
            <xsl:variable name="offset">
                <xsl:value-of select="($currentPage * $count) - $count"/>
            </xsl:variable>
            <a href="registration_details.jsp?location={$location}&amp;sort={$sort}&amp;index={$offset + 1}&amp;max={$offset + $count}&amp;count={$count}" class="navItem">
                <xsl:value-of select="$currentPage"/></a>
        </xsl:if>
        <xsl:if test="$currentPage &lt;= $pages">
            <xsl:call-template name="BuildNav">
                <xsl:with-param name="currentPage" select="$currentPage + 1"/>
            </xsl:call-template>
        </xsl:if>
        <xsl:if test="$currentPage &gt; $pages">
            <a href="registration_details.jsp?location={$location}&amp;sort={$sort}&amp;index=1&amp;max={$total + 1}&amp;count={$count}" class="navItem">
                [All]</a>
        </xsl:if>
    </xsl:template>


</xsl:stylesheet>
