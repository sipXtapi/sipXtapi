<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>
    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:apply-templates select="applications_list/categories"/>
    </xsl:template>

    <xsl:template match="categories">
        <xsl:for-each select="category">
            <xsl:variable name="display">
                <xsl:if test="position() != 1">
                    <xsl:text>none</xsl:text>
                </xsl:if>
            </xsl:variable>
            <div id="cat_{name}" style="display:{$display}" align="center">
                <div align="center" class="formtitle">
                    <xsl:value-of select="name"/>
                </div>
                <table border="0" cellpadding="4" width="560" cellspacing="1" class="bglist">
                    <tr>
                        <th width="100">Name</th>
                        <th width="390">Description</th>
                        <th widht="70">Action</th>
                    </tr>
                    <xsl:apply-templates select="applications">
                        <xsl:with-param name="cat" select="concat('cat_', name)"/>
                    </xsl:apply-templates>
                </table>
            </div>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="applications">
        <xsl:param name="cat"/>
        <xsl:for-each select="application">
            <xsl:sort select="name"/>
            <tr>
                <td>
                    <xsl:value-of select="name"/>

                </td>
                <td>
                    <xsl:value-of select="shortdescription"/>
                    <xsl:if test="infourl != null">
                        <xsl:text>...</xsl:text><a href="{infourl}" target="_blank">[read me]</a>
                    </xsl:if>

                </td>
                <td align="center" width="50">
                    <form action="../application/download_application.jsp" method="post">
                    <!--<form action="../../test/test.jsp" method="post" target="_blank">-->
                        <input type="hidden" name="applicationdescription" value="{shortdescription}" id="appdesc"/>
                        <input type="hidden" name="applicationname" value="{name}" id="appname"/>
                        <input type="hidden" name="devicetypeid" id="did"/>
                        <input type="hidden" name="refpropertyid" id="rpid"/>
                        <input type="hidden" name="sourceurl" value="{url}" id="surl"/>
                        <input type="hidden" name="cat" id="cat" value="{$cat}"/>
                        <xsl:variable name="tFlag">
                            <xsl:apply-templates select="/applications_list/downloaded_applications">
                                <xsl:with-param name="jarname" select="filename"/>
                            </xsl:apply-templates>
                        </xsl:variable>
                        <xsl:choose>
                            <xsl:when test="$tFlag != ''">
                                <!-- <input type="submit" value="Reload" id="btn"/>-->
                                <input type="hidden" name="shouldcreate" id="screate" value="false"/>
                                <a href='#' onclick="submit()">Reload</a>
                            </xsl:when>
                            <xsl:otherwise>
                                <!-- <input type="submit" value="Download" id="btn"/> -->
                                <input type="hidden" name="shouldcreate" id="screate" value="true"/>
                                <a href='#' onclick="submit()">Download</a>
                            </xsl:otherwise>
                        </xsl:choose>
                    </form>
                </td>
            </tr>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="downloaded_applications">
        <xsl:param name="jarname"/>
        <xsl:for-each select="application">
            <xsl:if test=". = $jarname">
                <xsl:text>x</xsl:text>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>