<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>

    <xsl:template match="/definitions">
        <form name="inputform" action="globaldefinitions_save.jsp" method='"post"'>
            <!-- Display the page title and the help link -->
            <table border="0" width="625">
                <xsl:choose>
                    <xsl:when test="/definitions/error/@critical = 'yes'">
                        <tr>
                            <td class="errortext_light" colspan="2">
                                <xsl:value-of select="/definitions/error" disable-output-escaping="yes" />
                            </td>
                        </tr>
                    </xsl:when>
                    <xsl:otherwise>
                        <tr>
                            <td width="50%" align="left">
                                <h1 class="list">
                                    Manage Global Definitions
                                </h1>
                            </td>
                            <td align="right" width="50%">
                                <a class="formtext" href="javascript:void 0" onclick="MM_openBrWindow('help/commserver/WebHelp/global_definitions.htm','popup','scrollbars,menubar,location=no,resizable,width=750,height=500');">Help</a>
                            </td>
                        </tr>
                        <tr>
                            <td colspan="2">
                                <hr class="dms"/>
                            </td>
                        </tr>
                        <tr>
                            <xsl:choose>
                                <xsl:when test="/definitions/error/@iserror = 'yes'">
                                    <td class="errortext_light" colspan="2">
                                        <xsl:variable name="error" select="/definitions/error"/>
                                        <xsl:choose>
                                            <xsl:when test="$error">
                                                <xsl:value-of select="$error" disable-output-escaping="yes" />
                                            </xsl:when>
                                            <xsl:otherwise>
                                                <img src="../image/spacer.gif" width="1" height="15" />
                                            </xsl:otherwise>
                                        </xsl:choose>
                                    </td>
                                </xsl:when>
                                    <xsl:otherwise>
                                        <td class="msgtext" colspan="2">
                                            <xsl:variable name="error" select="/definitions/error"/>
                                            <xsl:choose>
                                                <xsl:when test="$error">
                                                    <xsl:value-of select="$error" disable-output-escaping="yes" />
                                                </xsl:when>
                                                <xsl:otherwise>
                                                    <img src="../image/spacer.gif" width="1" height="15" />
                                                </xsl:otherwise>
                                            </xsl:choose>
                                        </td>
                                    </xsl:otherwise>
                            </xsl:choose>
                        </tr>
                        <tr>
                            <td colspan="2">
                                <!-- Wrap the table listing the definitions using a <div> tag.
                                        This is useful in adding and deleting speeddials on the fly
                                -->
                                <table border="0" class="bglist" cellpadding="4" cellspacing="1" width="625">
                                    <tr>
                                        <th>Setting</th>
                                        <th width="300">Value</th>
                                    </tr>
                                </table>
                                <div id="globaldefinitionslist">
                                    <xsl:call-template name="displaySettings" />
                                </div>
                            </td>
                        </tr>
                        <tr>
                            <td colspan="2">
                                <br/>
                            </td>
                        </tr>
                    </xsl:otherwise>
                </xsl:choose>
            </table>
        </form>
    </xsl:template>

    <!-- Iterate through all the properties and display standard properties. -->
    <xsl:template name="displaySettings">

        <xsl:for-each select="/definitions/definition">
            <xsl:variable name="configname" select="name"/>
            <xsl:variable name="configvalue" select="value"/>
            <xsl:variable name="id" select="normalize-space(id)" />
            <div id="{$id}">
                <table border="0" class="bglistmargins" cellpadding="4" cellspacing="0" width="625">
                    <tr>
                    <td>
                        <xsl:value-of select="$configname"/>
                    </td>
                    <td width="300" nowrap="yes">
                        <input type="text" size="43" value="{normalize-space($configvalue)}" name="{normalize-space($configname)}" onchange="setFlag()"/>
                        <img src="../image/spacer.gif" width="5" />
                        <a href="#">
                            <img src="../image/del.gif" border="0" onclick="del_definition('{$id}')" alt="Delete"/>
                        </a>
                    </td>
                </tr>
                </table>
            </div>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>

