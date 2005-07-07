<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <xsl:param name="mfgid"/>
    <xsl:param name="devicetypeid"/>
    <xsl:param name="usertype"/>
    <xsl:param name="detailstype"/>
    <xsl:param name="level"/>
    <xsl:param name="visiblefor"/>
    <xsl:param name="modelLabel"/>
    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:choose>
            <xsl:when test="$detailstype = 'user'">
                <table width="600">
                    <tr>
                        <td class="formtext" align="left" valign="top">
                            User Name: <xsl:value-of select="/details/attributes/displayid"/>
                        </td>
                        <td class="formtext" align="left" valign="top">
                            Settings For: 
							<xsl:choose>
								<xsl:when test="$devicetypeid != 'common'">
									<xsl:value-of select="$modelLabel"/>
							    </xsl:when>
								<xsl:otherwise>
    								<xsl:value-of select="$mfgid"/>
								</xsl:otherwise>
                            </xsl:choose>
                        </td>
                        <td class="formtext" align="right" valign="top">Settings For:</td>
                        <td>
                            <xsl:apply-templates select="/details/refpropertygroup_rp_mappings/property_group_link" mode="buildSelect">
                                <xsl:with-param name="did">
                                    <xsl:value-of select="/details/attributes/id"/>
                                </xsl:with-param>
                            </xsl:apply-templates>
                        </td>
                        <td class="formtext" align="right" valign="top">
                            <a style="color: #669999;" href="#" onclick="top.MM_openBrWindow('/pds/ui/help/ConfigSrvr/WebHelp/configsrvr.htm#list_users.htm#configure_user_settings','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')">Help</a>
                        </td>
                    </tr>
                </table>
            </xsl:when>
            <xsl:when test="$detailstype = 'usergroup'">
                <table width="600">
                    <tr>
                        <td class="formtext" width="200" align="left" valign="top">
                            User Group Name: <xsl:value-of select="/details/attributes/name"/>
                        </td>
                        <td class="formtext" align="left" valign="top">
                            Settings For: 
							<xsl:choose>
								<xsl:when test="$devicetypeid != 'common'">
									<xsl:value-of select="$modelLabel"/>
							    </xsl:when>
								<xsl:otherwise>
    								<xsl:value-of select="$mfgid"/>
								</xsl:otherwise>
                            </xsl:choose>
                        </td>
                        <td class="formtext" width="100" align="right" valign="top">Settings For:</td>
                        <td>
                            <xsl:apply-templates select="/details/refpropertygroup_rp_mappings/property_group_link" mode="buildSelect">
                                <xsl:with-param name="did">
                                    <xsl:value-of select="/details/attributes/id"/>
                                </xsl:with-param>
                            </xsl:apply-templates>
                        </td>
                        <td class="formtext" align="right" valign="top">
                            <a style="color: #669999;" href="#" onclick="top.MM_openBrWindow('/pds/ui/help/ConfigSrvr/WebHelp/configsrvr.htm#list_user_groups.htm#configure_ugroup_settings','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')">Help</a>
                        </td>
                    </tr>
                </table>
            </xsl:when>
            <xsl:when test="$detailstype = 'device'">
                <table width="600">
                    <tr>
                        <td class="formtext" width="200" align="left" valign="top">
                            Device Name: <xsl:value-of select="/details/attributes/shortname"/>
                        </td>
                        <td class="formtext" align="left" valign="top">
                            Device Type: 
							<xsl:choose>
								<xsl:when test="$devicetypeid != 'common'">
									<xsl:value-of select="$modelLabel"/>
							    </xsl:when>
								<xsl:otherwise>
    								<xsl:value-of select="$mfgid"/>
								</xsl:otherwise>
                            </xsl:choose>
                        </td>
                        <td class="formtext" align="right" valign="top">
                            <a style="color: #669999;" href="#" onclick="top.MM_openBrWindow('/pds/ui/help/ConfigSrvr/WebHelp/configsrvr.htm#list_devices.htm#configure_device_settings','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')">Help</a>
                        </td>
                        <!-- <td class="formtext" width="100" align="right" valign="top">Settings For:</td>
                        <td>
                            <xsl:apply-templates select="/details/refpropertygroup_rp_mappings/property_group_link" mode="buildSelect">
                                <xsl:with-param name="did">
                                    <xsl:value-of select="/details/attributes/id"/>
                                </xsl:with-param>
                            </xsl:apply-templates>
                        </td> -->
                    </tr>
                </table>
            </xsl:when>
            <xsl:when test="$detailstype = 'devicegroup'">
                <table width="600">
                    <tr>
                        <td class="formtext" width="200" align="left" valign="top">
                            Device Group Name: <xsl:value-of select="/details/attributes/name"/>
                        </td>
                        <td class="formtext" align="left" valign="top">
                            Settings For: 							
							<xsl:choose>
								<xsl:when test="$devicetypeid != 'common'">
									<xsl:value-of select="$modelLabel"/>
							    </xsl:when>
								<xsl:otherwise>
    								<xsl:value-of select="$mfgid"/>
								</xsl:otherwise>
                            </xsl:choose>
                        </td>
                        <td class="formtext" width="100" align="right" valign="top">Settings For:</td>
                        <td>
                            <xsl:apply-templates select="/details/refpropertygroup_rp_mappings/property_group_link" mode="buildSelect">
                                <xsl:with-param name="did">
                                    <xsl:value-of select="/details/attributes/id"/>
                                </xsl:with-param>
                            </xsl:apply-templates>
                        </td>
                        <td class="formtext" align="right" valign="top">
                            <a style="color: #669999;" href="#" onclick="top.MM_openBrWindow('/pds/ui/help/ConfigSrvr/WebHelp/configsrvr.htm#list_device_groups.htm#configure_dgroup_settings','popup','scrollbars,menubar,location=no,resizable,width=750,height=500')">Help</a>
                        </td>
                    </tr>
                </table>
            </xsl:when>
            <xsl:otherwise></xsl:otherwise>
        </xsl:choose>
        <xsl:apply-templates select="details"/>
    </xsl:template>

    <xsl:template match="details">
        <xsl:variable name="did" select="attributes/id"/>
        <!-- <form name="form1" method="post" action=""> -->
            <input type="button" name="Submit{id}" value="General"
            onMouseOver="pviiClassNew(this,'buttonover')"
            onMouseOut="pviiClassNew(this,'buttonup')"
            class="buttonup"
            onClick="parent.changeTab('tab_General')">
            </input>
            <xsl:apply-templates select="/details/refpropertygroup_rp_mappings/property_group_link/manufacturer" mode="chkForCode">
                <xsl:with-param name="did" select="$did"/>
            </xsl:apply-templates>
        <!-- </form> -->
    </xsl:template>

    <xsl:template match="manufacturer" mode="chkForCode">
        <xsl:param name="did"/>
        <xsl:if test="@name = translate($mfgid, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')">
            <xsl:for-each select="device_type">
                <xsl:if test="@model = $devicetypeid or (@model = 'common' and $detailstype = 'device')">
                    <xsl:apply-templates select="ref_property_group">
                        <xsl:with-param name="did" select="$did"/>
                    </xsl:apply-templates>
                </xsl:if>
            </xsl:for-each>
        </xsl:if>
    </xsl:template>

    <xsl:template match="manufacturer" mode="select">
        <xsl:param name="did"/>
        <xsl:variable name="mfg" select="@name"/>
        <xsl:for-each select="device_type">
            <xsl:variable name="model" select="@model"/>
            <xsl:variable name="tFlag">
                <xsl:for-each select="ref_property_group">
                    <xsl:if test="@usertype = $usertype">
                        <xsl:apply-templates select="ref_property"/>
                    </xsl:if>
                </xsl:for-each>
            </xsl:variable>
            <xsl:if test="$tFlag != ''">
                <xsl:variable name="str1" select="concat( $detailstype,'id=', $did)"/>
                <xsl:variable name="str2" select="concat('&amp;mfgid=', $mfg)"/>
                <xsl:variable name="str3" select="concat('&amp;devicetypeid=', $model)"/>
                <xsl:variable name="str4" select="concat('&amp;usertype=', $usertype)"/>
                <option value="{$str1}{$str2}{$str3}{$str4}">
                    <xsl:value-of select="$mfg"/>
                    <xsl:if test="$model != 'common'">
                        <xsl:text> - </xsl:text>
                        <xsl:value-of select="$model"/>
                    </xsl:if>
                </option>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="ref_property_group">
        <xsl:param name="did"/>
        <xsl:variable name="tFlag">
            <xsl:apply-templates select="ref_property"/>
        </xsl:variable>
        <xsl:if test="$tFlag != '' and @usertype = $usertype">
            <xsl:call-template name="buttonBuild">
                <xsl:with-param name="name" select="@name"/>
                <xsl:with-param name="did" select="$did"/>
                <xsl:with-param name="model" select="../@model"/>
            </xsl:call-template>
        </xsl:if>
    </xsl:template>

    <xsl:template match="ref_property">
        <xsl:if test="(@level = $level or @level = 'both') and @visible-for = $visiblefor">
            <xsl:variable name="code" select="@code"/>
            <xsl:for-each select="/details/refproperties/refproperty">
                <xsl:if test="$code = normalize-space(code)">
                    <xsl:text>x</xsl:text>
                </xsl:if>
            </xsl:for-each>
        </xsl:if>
    </xsl:template>

    <xsl:template name="buttonBuild">
        <xsl:param name="name"/>
        <xsl:param name="did"/>
        <xsl:param name="model"/>
        <xsl:choose>
            <xsl:when test="$name = 'Lines' and $detailstype = 'devicegroup'">
            </xsl:when>
            <xsl:when test="$name = 'SIP servers' and $model = '7960'">
            </xsl:when>
            <xsl:otherwise>
                <input type="button" name="Submit{id}" value="{$name}"
                onMouseOver="pviiClassNew(this,'buttonover')"
                onMouseOut="pviiClassNew(this,'buttonup')"
                class="buttonup"
                onClick="parent.changeTab('tab_{$name}')">
                </input>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="property_group_link" mode="buildSelect">
        <xsl:param name="did"/>
        <form name="frmSelect" method="post" action="">
            <select name="display">
                <xsl:apply-templates select="manufacturer" mode="select">
                    <xsl:with-param name="did" select="$did"/>
                </xsl:apply-templates>
            </select>

            <a href="#" onclick="reloadFrameSet()">
                <img name="sm_btn_go" src="../buttons/go_btn2.gif" width="23" height="17" border="0" ></img>
            </a>
        </form>
    </xsl:template>
</xsl:stylesheet>