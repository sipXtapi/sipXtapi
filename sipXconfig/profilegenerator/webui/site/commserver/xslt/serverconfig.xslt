<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <xsl:param name="servername"/>

    <xsl:template match="/details">
        <form name="inputform" action="serverconfig_save.jsp">
            <input type="hidden" name="servername" value="{$servername}" />
            <!-- Display the page title and the help link -->
            <table border="0" width="625">
                <xsl:choose>
                    <xsl:when test="/details/error/@critical = 'yes'">
                        <tr>
                            <td class="errortext_light" colspan="2">
                                <xsl:value-of select="/details/error" disable-output-escaping="yes" />
                            </td>
                        </tr>
                    </xsl:when>
                    <xsl:otherwise>
                        <tr>
                            <td width="50%" align="left">
                                <h1 class="list">
                                    <xsl:value-of select="title"/>
                                </h1>
                            </td>
                            <td align="right" width="50%">
                                <xsl:variable name="pagehelp" select="pagehelp"/>
                                <a class="formtext" href="javascript:void 0" onclick="MM_openBrWindow('{$pagehelp}','popup','scrollbars,menubar,location=no,resizable,width=750,height=500');">Help</a>
                            </td>
                        </tr>
                        <tr>
                            <td colspan="2">
                                <hr class="dms"/>
                            </td>
                        </tr>
                        <tr>
                            <xsl:choose>
                                <xsl:when test="/details/error/@iserror = 'yes'">
                                    <td class="errortext_light" colspan="2">
                                        <xsl:variable name="error" select="/details/error"/>
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
                                            <xsl:variable name="error" select="/details/error"/>
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
                                <!-- Display the standard settings -->
                                <table border="0" class="bglist" cellpadding="4" cellspacing="1" width="625">
                                    <tr>
                                        <th>Setting</th>
                                        <th width="315">Value</th>
                                    </tr>
                                    <xsl:call-template name="displayStandard" />
                                    <tr>
                                        <td>Additional Settings</td>
                                        <td width="315">
                                            <table width="100%" cellspacing="0" cellpadding="0" class="bglist">
                                                <tr>
                                                    <td width="298">
                                                        <textarea name="additionalsettings" rows="5" cols="32" onchange="setFlag()">
                                                            <xsl:call-template name="displayAdditionalSettings" />
                                                        </textarea>
                                                    </td>
                                                    <td valign="top">
                                                        <xsl:variable name="additionalsettingshelp" select="additionalsettingshelp"/>
                                                        <a href="#">
                                                            <img src="images/help.gif" border="0" align="top" onclick="MM_openBrWindow('{$additionalsettingshelp}','info','scrollbars=yes,width=600,height=400');"/>
                                                        </a>
                                                    </td>
                                                    </tr>
                                                </table>
                                        </td>
                                    </tr>
                                </table>
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
    <xsl:template name="displayStandard">
        <xsl:for-each select="/details/refproperties/ref_property">
            <xsl:variable name="refpropname" select="name" />
            <!-- Get the value associated with this property -->
            <xsl:variable name="value">
                <xsl:for-each select="/details/configurationset/configuration">
                    <xsl:if test="normalize-space(configname) = normalize-space($refpropname)">
                        <xsl:value-of select="normalize-space(configvalue)"/>
                    </xsl:if>
                </xsl:for-each>
            </xsl:variable>
            <xsl:call-template name="valuetemplate">
                <xsl:with-param name="refpropcode" select="@code"/>
                <xsl:with-param name="refpropname" select="$refpropname"/>
                <xsl:with-param name="value" select="$value"/>
                <xsl:with-param name="valuetype" select="value/@type"/>
            </xsl:call-template>
        </xsl:for-each>
    </xsl:template>


    <!-- Iterate through all the properties and display the additional settings-->
    <xsl:template name="displayAdditionalSettings">
        <xsl:for-each select="/details/configurationset/configuration">
            <xsl:variable name="configname" select="configname"/>
            <xsl:variable name="configvalue" select="configvalue"/>
            <!-- Check if this property is defined. If no, then it is an additional setting. -->
            <xsl:variable name="isstandard">
                <xsl:for-each select="/details/refproperties/ref_property">
                    <xsl:if test="normalize-space($configname) = normalize-space(name)">
                        <xsl:text>yes</xsl:text>
                    </xsl:if>
                </xsl:for-each>
            </xsl:variable>
            <xsl:if test="$isstandard != 'yes'">
                <xsl:value-of select="normalize-space($configname)"/>
                <xsl:text> : </xsl:text>
                <xsl:value-of select="normalize-space($configvalue)"/>
                <xsl:text>&#13;</xsl:text>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template name="valuetemplate">
        <xsl:param name="refpropcode"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:for-each select="/details/refproperties/ref_property">
            <xsl:if test="@code = $refpropcode">
                <xsl:choose>
                    <!-- Check the type of the value and decide the input field type accordingly -->
                    <xsl:when test="$valuetype = 'string'">
                        <xsl:call-template name="stringhandler">
                            <xsl:with-param name="refpropcode" select="$refpropcode"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="value" select="$value"/>
                        </xsl:call-template>
                    </xsl:when>
                    <xsl:otherwise>
                        <!-- Unknown type. Handle it as a string. -->
                        <xsl:call-template name="stringhandler">
                            <xsl:with-param name="refpropcode" select="$refpropcode"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="value" select="$value"/>
                        </xsl:call-template>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <!-- Handler for displaying configurations that take a 'string' as a value -->
    <xsl:template name="stringhandler">
        <xsl:param name="refpropcode"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:variable name="refpropertyid" select="$refpropcode"/>
        <xsl:for-each select="/details/refproperties/ref_property">
            <xsl:if test="name = $refpropname and @code = $refpropcode">
                <tr>
                    <td>
                        <xsl:value-of select="description"/>
                    </td>
                    <td width="315" nowrap="yes">
                        <table width="100%" cellspacing="0" cellpadding="0" class="bglist">
                            <tr>
                                <td width="298"><input type="text" size="45" value="{normalize-space($value)}" name="{$refpropname}" onchange="setFlag()"/></td>
                                <td><a href="#"><img src="images/help.gif" border="0" onclick="MM_openBrWindow('help/info/{$refpropcode}.htm','info','scrollbars=yes,width=600,height=400');"/></a></td>
                            </tr>
                        </table>
                    </td>
                </tr>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>

