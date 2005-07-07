<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <xsl:param name="refpropertygroupid"/>
    <xsl:param name="mfgid"/>
    <xsl:param name="devicetypeid"/>
    <xsl:param name="usertype"/>
    <xsl:param name="detailstype"/>
    <xsl:param name="level"/>
    <xsl:param name="visiblefor"/>


    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:if test="normalize-space($refpropertygroupid) = 'General'">
            <div id="tab_General" style="display:none">
                <xsl:if test="$usertype != 'user'">
                    <div class="formtext">
                        <xsl:text>General</xsl:text>
                    </div>
                    <hr align="left" class="dms" width="600"/>
                </xsl:if>
                <xsl:apply-templates select="/details/attributes"/>
            </div>
        </xsl:if>
        <xsl:apply-templates select="/details/refpropertygroup_rp_mappings/property_group_link/manufacturer"/>
    </xsl:template>


    <!-- match template "attributes", this is used for the General Tab -->
    <xsl:template match="attributes">
        <xsl:variable name="dnsdomain">
            <xsl:value-of select="organization/dnsdomain"/>
        </xsl:variable>
        <xsl:choose>
            <xsl:when test="$detailstype = 'user'">
                <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="600">
                    <tr>
                        <th valign="bottom" align="left" width="100">
                            Attribute
                        </th>
                        <th>
                            Value
                            <img src="../../image/1x1green.gif" width="320" height="0"/>
                            <a href="#">
                                <img src="../../image/info_link.gif" border="0"
                                    onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/general_user.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                            </a>
                        </th>
                    </tr>
                </table>
                <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                    <tr>
                        <th colspan="3">
                            <xsl:text>&#160;</xsl:text>
                        </th>
                        <th align="right">
                            <a href="#">
                                <img src="../../image/editicon.gif" alt="edit" width="14" height="14" border="0"
                                onclick="parent.edit_user()"/>
                            </a>
                        </th>
                    </tr>
                </table>

                <div id="edit_user">
                    <table border="0" class="bglist" cellspacing="1" cellpadding="4" width="600">
                        <tr>
                            <td width="100">
                                User ID
                            </td>
                            <td colspan="2" class="readonly">
                                <xsl:value-of select="displayid"/>
                            </td>
                        </tr>
                        <tr>
                            <td>First Name</td>
                            <td colspan="2" class="readonly">
                                <xsl:choose>
                                    <xsl:when test="firstname != ''">
                                       <xsl:value-of select="firstname"/>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <xsl:text>&#160;</xsl:text>
                                    </xsl:otherwise>
                                </xsl:choose>
                            </td>
                        </tr>
                        <tr>
                            <td>Last Name</td>
                            <td colspan="2" class="readonly">
                                <xsl:choose>
                                    <xsl:when test="lastname != ''">
                                       <xsl:value-of select="lastname"/>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <xsl:text>&#160;</xsl:text>
                                    </xsl:otherwise>
                                </xsl:choose>
                            </td>
                        </tr>
                    </table>
                    <input type="hidden" name="general.DisplayID" value="{displayid}" id="user_displayid"/>
                    <input type="hidden" name="general.FirstName" value="{firstname}" id="user_firstname"/>
                    <input type="hidden" name="general.LastName" value="{lastname}" id="user_lastname"/>
               </div>
                <table border="0" class="bglist" cellspacing="1" cellpadding="4" width="600">
                    <tr>
                        <td colspan="3">Extension</td>
                    </tr>
                    <tr>
                        <td width="100" align="right">Enter extension</td>
                        <td colspan="2">
                            <input type="radio" name="rb1" value="1" onclick="radioSelect(this)" checked="true"/>
                            <input type="text" name="general.Extension" value="{extension}" onchange="parent.validateExt(this.value)"/>
                        </td>
                    </tr>
                    <tr>
                        <td width="100" align="right">
                            Select from pool
                        </td>
                        <td colspan="2">
                            <div id="extpool">
                            </div>
                        </td>
                    </tr>
                    <tr>
                        <td width="100">User Group</td>
                        <td colspan="2">
                            <xsl:variable name="ugid" select="usergroupid"/>
                            <div id="groupid">
                            </div>
                            <input type="hidden" name="selectedgroupid" value="{$ugid}"/>
                        </td>
                    </tr>
                </table>
                <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">Aliases</th>
                            <th width="58" align="right">
                                <a href="#">
                                    <img border="0"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/popup/add_alias.jsp?dnsdomain={$dnsdomain}','popup','scrollbars=yes,width=600,height=200')"
                                        src="../../image/add.gif"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                <xsl:apply-templates select="aliases"/>
            </xsl:when>
            <xsl:when test="$detailstype = 'device'">
                <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="600">
                    <tr>
                        <th valign="bottom" align="left">
                            Attribute
                        </th>
                        <th>
                            Value
                            <img src="../../image/1x1green.gif" width="320" height="0"/>
                            <a href="#">
                                <img src="../../image/info_link.gif" border="0"
                                    onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/general_device.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                            </a>
                        </th>
                    </tr>
                    <tr>
                        <td>Name</td>
                        <td colspan="2">
                            <input type="text" name="general.ShortName" value="{shortname}"  onblur="parent.checkForNull(this,'{shortname}')"/>
                        </td>
                    </tr>
                    <tr>
                        <td>Serial Number</td>
                        <td colspan="2">
                            <input type="text"
                                name="general.SerialNumber"
                                value="{serialnumber}"
                                onblur="parent.validateSerialNumber(this.value,'{devicetypeid}')"/>
                        </td>
                    </tr>
                    <tr>
                        <td>Device Group</td>
                        <td colspan="2">
                            <xsl:variable name="dgid" select="devicegroupid"/>
                            <div id="groupid">
                            </div>
                            <input type="hidden" name="selectedgroupid" value="{$dgid}"/>
                       </td>
                    </tr>
                    <tr>
                        <td>Description</td>
                        <td colspan="2">
                            <textarea name="general.Description" cols="20" rows="4" onchange="parent.setSaveFlag()">
                                <xsl:value-of select="description"/>
                            </textarea>
                        </td>
                    </tr>
                </table>
            </xsl:when>
            <xsl:when test="$detailstype = 'devicegroup'">
                <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="600">
                    <tr>
                        <th valign="bottom" align="left">
                            Attribute
                        </th>
                        <th>
                            Value
                            <img src="../../image/1x1green.gif" width="320" height="0"/>
                            <a href="#">
                                <img src="../../image/info_link.gif" border="0"
                                    onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/general_devicegroup.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                            </a>
                        </th>
                    </tr>
                    <tr>
                        <td>Name</td>
                        <td colspan="2">
                            <input type="text" name="general.Name" value="{name}" onblur="parent.checkForNull(this,'{name}')"/>
                        </td>
                    </tr>
                    <tr>
                        <td width="100">Parent Device Group</td>
                        <td colspan="2">
                            <xsl:variable name="pid" select="parentid"/>
                            <div id="pid">
                            </div>
                            <input type="hidden" name="selectedpid" value="{$pid}"/>
                        </td>
                    </tr>
                </table>
            </xsl:when>
            <xsl:when test="$detailstype = 'usergroup'">
                <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="600">
                    <tr>
                        <th valign="bottom" align="left">
                            Attribute
                        </th>
                        <th>
                            Value
                            <img src="../../image/1x1green.gif" width="320" height="0"/>
                            <a href="#">
                                <img src="../../image/info_link.gif" border="0"
                                    onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/general_usergroup.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                            </a>
                        </th>
                    </tr>
                    <tr>
                        <td>Name</td>
                        <td colspan="2">
                            <input type="text" name="general.Name" value="{name}" onblur="parent.checkForNull(this,'{name}')"/>
                        </td>
                    </tr>
                    <tr>
                        <td width="100">Parent User Group</td>
                        <td colspan="2">
                            <xsl:variable name="pid" select="parentid"/>
                            <div id="pid">
                            </div>
                            <input type="hidden" name="selectedpid" value="{$pid}"/>
                        </td>
                    </tr>
                </table>
            </xsl:when>
            <xsl:otherwise></xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="aliases">
        <xsl:variable name="dnsdomain">
            <xsl:value-of select="/details/attributes/organization/dnsdomain"/>
        </xsl:variable>
        <div id="insert_alias">
        <xsl:for-each select="alias">
            <div id="{.}">
                <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                    <tr>
                        <th colspan="3">
                            <xsl:text>&#160;</xsl:text>
                        </th>
                        <th align="right">
                            <a href="#">
                                <img src="../../image/editicon.gif" alt="edit" width="14" height="14" border="0"
                                onclick="parent.edit_alias('{.}','{$dnsdomain}')"/>
                            </a>
                            <img src="../../image/1x1green.gif" width="10"/>
                            <a href="#">
                                <img src="../../image/del.gif" alt="delete" width="14" height="14" border="0"
                                onclick="parent.del_alias('{.}')"/>
                            </a>
                        </th>
                    </tr>
                </table>
                <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                    <tr>
                        <td colspan="3">
                        Alias
                        </td>
                        <td class="readonly" width="259">
                            <xsl:text>sip:</xsl:text><xsl:value-of select="."/><xsl:text>@</xsl:text><xsl:value-of select="$dnsdomain"/>
                        </td>
                    </tr>
                </table>
                <input type="hidden" id="alias.{.}" value="{.}"/>
            </div>
        </xsl:for-each>
        <xsl:variable name="aliasString">
            <xsl:for-each select="alias">
                <xsl:value-of select="."/>
                <xsl:if test="not(position()=last())">
                    <xsl:text>,</xsl:text>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
            <input type="hidden" name="general.Aliases" size="60" value="{$aliasString}"/>
        </div>
    </xsl:template>

    <xsl:template match="manufacturer">
        <xsl:param name="name"/>
        <xsl:if test="@name = translate($mfgid, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')">
            <xsl:for-each select="device_type">
                <xsl:if test="@model = $devicetypeid or (@model = 'common' and $detailstype = 'device')">
                    <xsl:apply-templates select="ref_property_group"/>
                </xsl:if>
            </xsl:for-each>
        </xsl:if>
    </xsl:template>

    <xsl:template match="ref_property_group">
      <xsl:if test="$usertype = @usertype">
        <div id="tab_{@name}" style="display:none">
            <xsl:if test="$usertype != 'user'">
                <div class="formtext">
                    <xsl:value-of select="@name"/>
                </div>
                <hr align="left" class="dms" width="600"/>
            </xsl:if>
            <xsl:apply-templates select="ref_property"/>
        </div>
        </xsl:if>
    </xsl:template>

    <xsl:template match="ref_property">
        <xsl:if test="(@level = $level or @level = 'both') and @visible-for = $visiblefor">
            <xsl:variable name="code" select="@code"/>
            <xsl:apply-templates select="/details/refproperties/refproperty" mode="getProperty">
                <xsl:with-param name="code" select="$code"/>
            </xsl:apply-templates>
        </xsl:if>
    </xsl:template>

    <xsl:template match="refproperty" mode="getProperty">
        <xsl:param name="code"/>
        <xsl:if test="normalize-space(code) = normalize-space($code)">
            <xsl:choose>
                <xsl:when test="code = 'xp_1003'">
                    <xsl:variable name="rpid">
                        <xsl:for-each select="/details/refproperties/refproperty">
                            <xsl:if test="code = 'xp_1003'">
                                <xsl:value-of select="id"/>
                            </xsl:if>
                        </xsl:for-each>
                    </xsl:variable>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">Digitmaps</th>
                            <th width="58" align="right">
                                <a href="#">
                                    <img border="0"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/popup/add_digitmap.html?rpid={$rpid}','popup','scrollbars=yes,width=400,height=300')"
                                        src="../../image/add.gif"/>
                                </a>
                                <img src="../../image/1x1green.gif" width="10"/>
                                <a href="#">
                                    <img border="0" src="../../image/info_link.gif"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/{$code}.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                   <xsl:call-template name="xp_1003"/>
                </xsl:when>
                <xsl:when test="code = 'xp_1005'">
                    <xsl:variable name="rpid">
                        <xsl:for-each select="/details/refproperties/refproperty">
                            <xsl:if test="code = 'xp_1005'">
                                <xsl:value-of select="id"/>
                            </xsl:if>
                        </xsl:for-each>
                    </xsl:variable>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">Authorized Phone Users</th>
                            <th width="80" align="right">
                                <a href="#">
                                    <img border="0"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/popup/add_authuser.html?rpid={$rpid}','popup','scrollbars=yes,width=400,height=300')"
                                        src="../../image/add.gif"/>
                                </a>
                                <xsl:if test="/details/setproperties/ref_property_id = id">
                            		<img src="../../image/1x1green.gif" border="0" width="10"/>
									<img src="../../image/uparrow.gif" border="0" alt="set above"/>
                        		</xsl:if>
								<img src="../../image/1x1green.gif" border="0" width="10"/>
                                <a href="#">
                                    <img border="0" src="../../image/info_link.gif"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/{$code}.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <xsl:call-template name="xp_1005"/>
                </xsl:when>
                <xsl:when test="code = 'xp_1018'">
                    <xsl:variable name="rpid">
                        <xsl:for-each select="/details/refproperties/refproperty">
                            <xsl:if test="code = 'xp_1018'">
                                <xsl:value-of select="id"/>
                            </xsl:if>
                        </xsl:for-each>
                    </xsl:variable>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">Authorized Callers</th>
                            <th width="58" align="right">
                                <a href="#">
                                    <img border="0"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/popup/add_authcaller.html?rpid={$rpid}','popup','scrollbars=yes,width=400,height=300')"
                                        src="../../image/add.gif"/>
                                </a>
                                <img src="../../image/1x1green.gif" width="10"/>
                                <a href="#">
                                    <img border="0" src="../../image/info_link.gif"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/{$code}.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <xsl:call-template name="xp_1018"/>
                </xsl:when>
                <xsl:when test="code = 'xp_1031'">
                    <xsl:variable name="rpid">
                        <xsl:for-each select="/details/refproperties/refproperty">
                            <xsl:if test="code = 'xp_1031'">
                                <xsl:value-of select="id"/>
                            </xsl:if>
                        </xsl:for-each>
                    </xsl:variable>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">Device Line</th>
                            <th width="58" align="right">
                                <a href="#">
                                    <img border="0" src="../../image/info_link.gif"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/{$code}.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <xsl:call-template name="xp_1031"/>
                </xsl:when>
                <xsl:when test="code = 'xp_2029'">
                    <xsl:variable name="rpid">
                        <xsl:for-each select="/details/refproperties/refproperty">
                            <xsl:if test="code = 'xp_2029'">
                                <xsl:value-of select="id"/>
                            </xsl:if>
                        </xsl:for-each>
                    </xsl:variable>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">Additional User Lines</th>
                            <th width="58" align="right">
                                <xsl:if test="/details/setproperties/ref_property_id = $rpid">
                                    <img src="../../image/uparrow.gif" border="0" alt="set above"/>
                                    <img src="../../image/1x1green.gif" border="0" width="10"/>
                                </xsl:if>
                                <a href="#">
                                    <img border="0"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/popup/add_userline.html?rpid={$rpid}','popup','scrollbars=yes,width=460,height=300')"
                                        src="../../image/add.gif"/>
                                </a>
                                <img src="../../image/1x1green.gif" width="10"/>
                                <a href="#">
                                    <img border="0" src="../../image/info_link.gif"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/{$code}.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                                </a>
                            </th>
                        </tr>
                        <tr>
                            <th colspan="4">
                                <xsl:text>&#160;</xsl:text>
                            </th>
                        </tr>
                    </table>
                    <xsl:call-template name="xp_2029"/>
                </xsl:when>
                <xsl:when test="code = 'xp_2030'">
                    <xsl:variable name="rpid">
                        <xsl:for-each select="/details/refproperties/refproperty">
                            <xsl:if test="code = 'xp_2030'">
                                <xsl:value-of select="id"/>
                            </xsl:if>
                        </xsl:for-each>
                    </xsl:variable>
                    <xsl:variable name="sdVis">
                       <xsl:text>display:''</xsl:text>
                    </xsl:variable>
                    <div id="{code}head" style="{$sdVis}">
                        <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                            <tr>
                                <th colspan="3">
                                    Speed Dial Numbers
                                </th>
                                <th width="78" align="right">
                                    <xsl:if test="/details/setproperties/ref_property_id = $rpid">
                                        <img src="../../image/uparrow.gif" border="0" alt="set above"/>
                                        <img src="../../image/1x1green.gif" border="0" width="10"/>
                                    </xsl:if>
                                    <a href="#">
                                        <img border="0"
                                            onclick="window.top.MM_openBrWindow('/pds/ui/popup/add_speeddial.html?rpid={$rpid}','popup','scrollbars=yes,width=400,height=300')"
                                            src="../../image/add.gif"/>
                                    </a>
                                    <img src="../../image/1x1green.gif" width="10"/>
                                    <a href="#">
                                        <img border="0" src="../../image/info_link.gif"
                                            onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/{$code}.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                                    </a>
                                </th>
                            </tr>
                        </table>
                    </div>
                    <xsl:call-template name="xp_2030">
                        <xsl:with-param name="sdVis" select="$sdVis"/>
                    </xsl:call-template>
                </xsl:when>
                <xsl:when test="code = 'xp_10001'">
                    <xsl:if test="$detailstype = 'user'">
                        <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                            <tr>
                                <th colspan="3">Primary User Line</th>
                                <th width="58" align="right">
                                    <a href="#">
                                        <img border="0" src="../../image/info_link.gif"
                                            onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/{$code}.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                                    </a>
                                </th>
                            </tr>
                        </table>
                        <xsl:call-template name="xp_10001"/>
                    </xsl:if>
                </xsl:when>
                <xsl:when test="code = 'cs_1020'">
                    <xsl:variable name="rpid">
                        <xsl:for-each select="/details/refproperties/refproperty">
                            <xsl:if test="code = 'cs_1020'">
                                <xsl:value-of select="id"/>
                            </xsl:if>
                        </xsl:for-each>
                    </xsl:variable>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">Phone line 1</th>
                            <th width="58" align="right">
                                <a href="#">
                                    <img border="0" src="../../image/info_link.gif"
                                        onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/cisco_79xx.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <xsl:call-template name="cs_1020"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:apply-templates select="." mode="buildForm">
                        <xsl:with-param name="refpropname" select="name"/>
                        <xsl:with-param name="code" select="$code"/>
                        <xsl:with-param name="cardinality" select="content/definition/value/@cardinality"/>
                        <xsl:with-param name="valuetype" select="content/definition/value/@type"/>
                        <xsl:with-param name="valuename" select="content/definition/value/@name"/>
                        <xsl:with-param name="refpropcode" select="normalize-space(code)"/>
                    </xsl:apply-templates>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:if>
    </xsl:template>

    <xsl:template name="xp_1003">
        <div id="insert_digitmap">
            <xsl:for-each select="/details/configurationset/PROFILE/PHONESET_DIGITMAP">
                <xsl:variable name="id" select="@id"/>
                <xsl:variable name="rpid" select="@ref_property_id"/>
                <div id="{$id}">
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">
                                <xsl:text>&#160;</xsl:text>
                            </th>
                            <th align="right">
                                <a href="#">
                                    <img src="../../image/editicon.gif" alt="edit" width="14" height="14" border="0"
                                    onclick="parent.edit_digitmap('{$id}','{$rpid}')"/>
                                </a>
                                <img src="../../image/1x1green.gif" width="10"/>
                                <a href="#">
                                    <img src="../../image/del.gif" alt="delete" width="14" height="14" border="0"
                                    onclick="parent.del_digitmap('{$id}')"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                        <tr>
                            <td colspan="3">
                            Dial plan
                            </td>
                            <td class="readonly" width="259">
                                <xsl:value-of select="PHONESET_DIGITMAP/NUMBER"/>
                            </td>
                        </tr>
                        <tr>
                            <td colspan="3">
                            SIP Address
                            </td>
                            <td class="readonly" width="259">
                                <xsl:value-of select="PHONESET_DIGITMAP/TARGET"/>
                            </td>
                        </tr>
                    </table>
                    <input type="hidden"
                                name="{@id}.PHONESET_DIGITMAP.{@ref_property_id}.PHONESET_DIGITMAP~EMPTY.NUMBER.i_true."
                                value="{PHONESET_DIGITMAP/NUMBER}" id="digitmap_number"/>
                    <input type="hidden"
                                name="{@id}.PHONESET_DIGITMAP.{@ref_property_id}.PHONESET_DIGITMAP~EMPTY.TARGET."
                                value="{PHONESET_DIGITMAP/TARGET}" id="digitmap_target"/>
                </div>
            </xsl:for-each>
        </div>
        <br/>
    </xsl:template>

    <xsl:template name="xp_1005">
        <div id="insert_authuser">
            <xsl:for-each select="/details/configurationset/PROFILE/PHONESET_HTTP_AUTH_DB">
                <xsl:variable name="id" select="@id"/>
                <xsl:variable name="rpid" select="@ref_property_id"/>
                <div id="{$id}">
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">
                                <xsl:text>&#160;</xsl:text>
                            </th>
                            <th align="right">
                                <a href="#">
                                    <img src="../../image/editicon.gif" alt="edit" width="14" height="14" border="0"
                                    onclick="parent.edit_authuser('{$id}','{$rpid}')"/>
                                </a>
                                <img src="../../image/1x1green.gif" width="10"/>
                                <a href="#">
                                    <img src="../../image/del.gif" alt="delete" width="14" height="14" border="0"
                                    onclick="parent.del_authuser('{$id}')"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                        <tr>
                            <td colspan="3">
                            User Name
                            </td>
                            <td class="readonly" width="259">
                                <xsl:value-of select="PHONESET_HTTP_AUTH_DB/USERID"/>
                            </td>
                        </tr>
                    </table>
                    <input type="hidden"
                                name="{@id}.PHONESET_HTTP_AUTH_DB.{@ref_property_id}.PHONESET_HTTP_AUTH_DB~EMPTY.USERID.i_true."
                                value="{PHONESET_HTTP_AUTH_DB/USERID}" id="authuser_userid"/>
                    <input type="hidden"
                                name="{@id}.PHONESET_HTTP_AUTH_DB.{@ref_property_id}.PHONESET_HTTP_AUTH_DB~EMPTY.PASSTOKEN."
                                value="{PHONESET_HTTP_AUTH_DB/PASSTOKEN}" id="authuser_passtoken"/>
                    <input type="hidden"
                                name="{@id}.PHONESET_HTTP_AUTH_DB.{@ref_property_id}.PHONESET_HTTP_AUTH_DB~EMPTY.REALM."
                                value="{PHONESET_HTTP_AUTH_DB/REALM}" id="authuser_realm"/>
                </div>
            </xsl:for-each>
        </div>
        <br/>
    </xsl:template>

    <xsl:template name="xp_1018">
        <div id="insert_authcaller">
            <xsl:for-each select="/details/configurationset/PROFILE/SIP_AUTHENTICATE_DB">
                <xsl:variable name="id" select="@id"/>
                <xsl:variable name="rpid" select="@ref_property_id"/>
                <div id="{$id}">
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">
                                <xsl:text>&#160;</xsl:text>
                            </th>
                            <th align="right">
                                <a href="#">
                                    <img src="../../image/editicon.gif" alt="edit" width="14" height="14" border="0"
                                    onclick="parent.edit_authcaller('{$id}','{$rpid}')"/>
                                </a>
                                <img src="../../image/1x1green.gif" width="10"/>
                                <a href="#">
                                    <img src="../../image/del.gif" alt="delete" width="14" height="14" border="0"
                                    onclick="parent.del_authcaller('{$id}')"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                        <tr>
                            <td colspan="3">
                            User Name
                            </td>
                            <td class="readonly" width="259">
                                <xsl:value-of select="SIP_AUTHENTICATE_DB/USER"/>
                            </td>
                        </tr>
                    </table>
                    <input type="hidden"
                                name="{@id}.SIP_AUTHENTICATE_DB.{@ref_property_id}.SIP_AUTHENTICATE_DB~EMPTY.USER.i_true."
                                value="{SIP_AUTHENTICATE_DB/USER}" id="authcaller_user"/>
                    <input type="hidden"
                                name="{@id}.SIP_AUTHENTICATE_DB.{@ref_property_id}.SIP_AUTHENTICATE_DB~EMPTY.PASSWORD."
                                value="{SIP_AUTHENTICATE_DB/PASSWORD}" id="authcaller_password"/>
                </div>
            </xsl:for-each>
        </div>
        <br/>
    </xsl:template>

    <xsl:template name="xp_1031">
        <xsl:for-each select="/details/configurationset/PROFILE/PHONESET_LINE">
            <xsl:variable name="id" select="@id"/>
            <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                <tr>
                    <td colspan="3">
                        SIP URL
                    </td>
                    <td class="readonly" width="259">
                        <xsl:value-of select="PHONESET_LINE/URL"/>
                    </td>
                </tr>
                <tr>
                    <td colspan="3">
                        Register with host or provision line as is
                    </td>
                    <td class="readonly" width="259">
                        <xsl:value-of select="PHONESET_LINE/REGISTRATION"/>
                    </td>
                </tr>
                <tr>
                    <td colspan="3">
                        Apply rules for call forwarding
                    </td>
                    <td class="readonly" width="259">
                        <xsl:value-of select="PHONESET_LINE/ALLOW_FORWARDING"/>
                    </td>
                </tr>
            </table>
            <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                <tr>
                    <th colspan="3">Credentials</th>
                </tr>
            </table>
            <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                <tr>
                    <td class="cell_fill_dark" width="19">
                        <xsl:text>&#160;</xsl:text>
                    </td>
                    <td colspan="2">User ID</td>
                    <td width="257" class="readonly">
                        <xsl:value-of select="PHONESET_LINE/CREDENTIAL/USERID"/>
                    </td>
                </tr>
                <tr>
                    <td class="cell_fill_dark" width="19">
                        <xsl:text>&#160;</xsl:text>
                    </td>
                    <td colspan="2">Realm</td>
                    <td width="257" class="readonly">
                        <xsl:value-of select="PHONESET_LINE/CREDENTIAL/REALM"/>
                    </td>

                </tr>
            </table>
        </xsl:for-each>
        <br/>
    </xsl:template>


    <xsl:template name="xp_2029">
        <div id="insert_userline">
            <xsl:for-each select="/details/configurationset/PROFILE/USER_LINE">
                <xsl:variable name="id" select="@id"/>
                <xsl:variable name="rpid" select="@ref_property_id"/>
                <div id="cont{$id}">
                    <div id="{$id}">
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3" align="left">
                                User Line
                            </th>
                            <th align="right">
                                <a href="#">
                                    <img src="../../image/editicon.gif" alt="edit" width="14" height="14" border="0"
                                    onclick="parent.edit_userline('{$id}','{$rpid}','cont{$id}')"/>
                                </a>
                                <img src="../../image/1x1green.gif" width="10"/>
                                <a href="#">
                                    <img src="../../image/del.gif" alt="delete" width="14" height="14" border="0"
                                    onclick="parent.del_userline('cont{$id}')"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                        <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                            <tr>
                                <td colspan="3">
                                SIP URL
                                </td>
                                <td class="readonly" width="259">
                                <xsl:value-of select="USER_LINE/URL"/>
                                </td>
                            </tr>
                            <tr>
                                <td colspan="3">
                                Register with host or provision line as is
                                </td>
                                <td class="readonly" width="259">
                                <xsl:value-of select="USER_LINE/REGISTRATION"/>
                                </td>
                            </tr>
                            <tr>
                                <td colspan="3">
                                Apply rules for call forwarding
                                </td>
                                <td class="readonly" width="259">
                                <xsl:value-of select="USER_LINE/ALLOW_FORWARDING"/>
                                </td>
                            </tr>
                        </table>
                        <input type="hidden"
                            name="{$id}.USER_LINE.{$rpid}.USER_LINE~EMPTY.URL.i_true."
                            value="{USER_LINE/URL}" id="uline_url"/>
                        <input type="hidden"
                            name="{$id}.USER_LINE.{$rpid}.USER_LINE~EMPTY.REGISTRATION."
                            value="{USER_LINE/REGISTRATION}" id="uline_registration"/>
                        <input type="hidden"
                            name="{$id}.USER_LINE.{$rpid}.USER_LINE~EMPTY.ALLOW_FORWARDING."
                            value="{USER_LINE/ALLOW_FORWARDING}" id="uline_allow_forwarding"/>
                    </div> <!-- id userline for edit only -->
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">Credentials</th>
                            <th width="58" align="right">
                                <a href="#">
                                <img border="0"
                                onclick="window.top.MM_openBrWindow('/pds/ui/popup/add_ulcredential.html?ulid={$id}&amp;rpid={$rpid}','popup','scrollbars=yes,width=400,height=300')"
                                src="../../image/add.gif"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <div id="insert_ulcredential{$id}">
                        <xsl:for-each select="USER_LINE/CREDENTIAL">
                            <xsl:variable name="cid">
                                <xsl:value-of select="position()"/>
                            </xsl:variable>
                            <div id="{$cid}">
                                <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                                    <tr>
                                        <th colspan="3">
                                            <xsl:text>&#160;</xsl:text>
                                        </th>
                                        <th align="right">
                                            <a href="#">
                                            <img src="../../image/editicon.gif" alt="edit" width="14" height="14" border="0"
                                            onclick="parent.edit_ulcredential('{$id}','{$cid}','{$rpid}')"/>
                                            </a>
                                            <img src="../../image/1x1green.gif" width="10"/>
                                            <a href="#">
                                            <img src="../../image/del.gif" alt="delete" width="14" height="14" border="0"
                                            onclick="parent.del_ulcredential('{$id}','{$cid}')"/>
                                            </a>
                                        </th>
                                    </tr>
                                </table>
                                <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                                    <tr>
                                        <td class="cell_fill_dark" width="19">
                                            <xsl:text>&#160;</xsl:text>
                                        </td>
                                        <td colspan="2">User ID</td>
                                        <td width="257" class="readonly">
                                            <xsl:value-of select="USERID"/>
                                        </td>
                                    </tr>
                                    <tr>
                                        <td class="cell_fill_dark" width="19">
                                            <xsl:text>&#160;</xsl:text>
                                        </td>
                                        <td colspan="2">Realm</td>
                                        <td width="257" class="readonly">
                                            <xsl:value-of select="REALM"/>
                                        </td>

                                    </tr>
                                </table>
                                <input type="hidden"
                                    name="{$id}.USER_LINE.{$rpid}.CREDENTIAL~USER_LINE~EMPTY.USERID.{$cid}"
                                    value="{USERID}" id="ulc_userid"/>
                                <input type="hidden"
                                    name="{$id}.USER_LINE.{$rpid}.CREDENTIAL~USER_LINE~EMPTY.PASSTOKEN.{$cid}"
                                    value="{PASSTOKEN}" id="ulc_passtoken"/>
                                <input type="hidden"
                                    name="{$id}.USER_LINE.{$rpid}.CREDENTIAL~USER_LINE~EMPTY.REALM.{$cid}"
                                    value="{REALM}" id="ulc_realm"/>
                            </div> <!-- cid -->
                        </xsl:for-each>
                    </div> <!-- insert credential -->
                    <xsl:if test="not(position()=last())">
                         <hr class="seperator" width="600" align="left"/>
                    </xsl:if>

                </div> <!-- cont id -->
            </xsl:for-each>
        </div> <!-- insert userline -->
    </xsl:template>

    <xsl:template name="xp_2030">
        <xsl:param name="sdVis"/>
        <div id="insert_speeddial" style="{$sdVis}">
            <xsl:for-each select="/details/configurationset/PROFILE/PHONESET_SPEEDDIAL">
                <xsl:variable name="id" select="@id"/>
                <xsl:variable name="rpid" select="@ref_property_id"/>
                <div id="{$id}">
                    <div id="display">
                    <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                        <tr>
                            <th colspan="3">
                                <xsl:text>&#160;</xsl:text>
                            </th>
                            <th align="right">
                                <a href="#">
                                    <img src="../../image/editicon.gif" alt="edit" width="14" height="14" border="0"
                        onclick="parent.edit_speeddial('{$id}','{$rpid}')"/>
                                </a>
                                <img src="../../image/1x1green.gif" width="10"/>
                                <a href="#">
                                    <img src="../../image/del.gif" alt="delete" width="14" height="14" border="0"
                        onclick="parent.del_speeddial('{$id}')"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                        <tr>
                            <td colspan="3">
                                Speed dial number
                            </td>
                            <td class="readonly" width="259">
                                <xsl:value-of select="PHONESET_SPEEDDIAL/ID"/>
                            </td>
                        </tr>
                        <tr>
                            <td colspan="3">
                                Name
                            </td>
                            <td class="readonly" width="259">
                                <xsl:value-of select="PHONESET_SPEEDDIAL/LABEL"/>
                            </td>
                        </tr>
                        <tr>
                            <td colspan="3">
                                Type
                            </td>
                            <td class="readonly" width="259">
                                <xsl:value-of select="PHONESET_SPEEDDIAL/TYPE"/>
                            </td>
                        </tr>
                        <tr>
                            <td colspan="3">
                                Number/URL
                            </td>
                            <td class="readonly" width="259">
                                <xsl:value-of select="PHONESET_SPEEDDIAL/ADDRESS"/>
                            </td>
                        </tr>
                    </table>
                    </div>
                    <input type="hidden"
                        name="{@id}.PHONESET_SPEEDDIAL.{$rpid}.PHONESET_SPEEDDIAL~EMPTY.ID.i_true."
                        value="{PHONESET_SPEEDDIAL/ID}" id="speeddial_sdid"/>
                    <input type="hidden"
                        name="{@id}.PHONESET_SPEEDDIAL.{$rpid}.PHONESET_SPEEDDIAL~EMPTY.LABEL."
                        value="{PHONESET_SPEEDDIAL/LABEL}" id="speeddial_label"/>
                    <input type="hidden"
                        name="{@id}.PHONESET_SPEEDDIAL.{$rpid}.PHONESET_SPEEDDIAL~EMPTY.TYPE."
                        value="{PHONESET_SPEEDDIAL/TYPE}" id="speeddial_type"/>
                    <input type="hidden"
                        name="{@id}.PHONESET_SPEEDDIAL.{$rpid}.PHONESET_SPEEDDIAL~EMPTY.ADDRESS."
                        value="{PHONESET_SPEEDDIAL/ADDRESS}" id="speeddial_address"/>
                </div>
            </xsl:for-each>
        </div>
        <br/>
    </xsl:template>

    <xsl:template name="xp_10001">
        <xsl:for-each select="/details/configurationset/PROFILE/PRIMARY_LINE">
            <xsl:variable name="rpid" select="@ref_property_id"/>
            <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                <tr>
                    <td colspan="3">
                SIP URL
                    </td>
                    <td class="readonly" width="259">
                        <xsl:value-of select="PRIMARY_LINE/URL"/>
                    </td>
                </tr>
                <tr>
                    <td colspan="3">
                Register with host or provision line as is
                    </td>
                    <td class="readonly" width="259">
                        <xsl:value-of select="PRIMARY_LINE/REGISTRATION"/>
                    </td>
                </tr>
                <tr>
                    <td colspan="3">
                Apply rules for call forwarding
                    </td>
                    <td class="readonly" width="259">
                        <xsl:value-of select="PRIMARY_LINE/ALLOW_FORWARDING"/>
                    </td>
                </tr>
            </table>
            <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                <tr>
                    <th colspan="3">Credentials</th>
                    <th width="58" align="right">
                        <a href="#">
                            <img border="0"
                        onclick="window.top.MM_openBrWindow('/pds/ui/popup/add_plcredential.html?rpid={$rpid}','popup','scrollbars=yes,width=400,height=300')"
                        src="../../image/add.gif"/>
                        </a>
                    </th>
                </tr>
            </table>
            <input type="hidden"
        name="null.PRIMARY_LINE.{$rpid}.PRIMARY_LINE~EMPTY.URL."
        value="{PRIMARY_LINE/URL}" id="pline_url"/>
            <input type="hidden"
        name="null.PRIMARY_LINE.{$rpid}.PRIMARY_LINE~EMPTY.REGISTRATION."
        value="{PRIMARY_LINE/REGISTRATION}" id="pline_registration"/>
            <input type="hidden"
        name="null.PRIMARY_LINE.{$rpid}.PRIMARY_LINE~EMPTY.ALLOW_FORWARDING."
        value="{PRIMARY_LINE/ALLOW_FORWARDING}" id="pline_allow_forwarding"/>


            <xsl:for-each select="PRIMARY_LINE/CREDENTIAL">
                <xsl:if test="REALM = /details/attributes/organization/dnsdomain">
                    <xsl:variable name="cid">
                        <xsl:value-of select="position()"/>
                    </xsl:variable>
                    <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                        <tr>
                            <td class="cell_fill_dark" width="19">
                                <xsl:text>&#160;</xsl:text>
                            </td>
                            <td colspan="2">Primary - User ID</td>
                            <td width="257" class="readonly">
                                <xsl:value-of select="USERID"/>
                            </td>
                        </tr>
                        <tr>
                            <td class="cell_fill_dark" width="19">
                                <xsl:text>&#160;</xsl:text>
                            </td>
                            <td colspan="2">Primary - Realm</td>
                            <td width="257" class="readonly">
                                <xsl:value-of select="REALM"/>
                            </td>

                        </tr>
                    </table>
                    <input type="hidden"
                            name="null.PRIMARY_LINE.{$rpid}.CREDENTIAL~PRIMARY_LINE~EMPTY.USERID.{$cid}"
                            value="{USERID}" id="plc_readonly_userid"/>
                    <input type="hidden"
                                    name="null.PRIMARY_LINE.{$rpid}.CREDENTIAL~PRIMARY_LINE~EMPTY.PASSTOKEN.{$cid}"
                                    value="{PASSTOKEN}" id="plc_readonly_passtoken"/>
                    <input type="hidden"
                                    name="null.PRIMARY_LINE.{$rpid}.CREDENTIAL~PRIMARY_LINE~EMPTY.REALM.{$cid}"
                                    value="{REALM}" id="plc_readonly_realm"/>
                </xsl:if>
            </xsl:for-each>
            <div id="insert_plcredential">
                <xsl:for-each select="PRIMARY_LINE/CREDENTIAL">
                    <xsl:if test="not(REALM = /details/attributes/organization/dnsdomain)">
                        <xsl:variable name="cid">
                            <xsl:value-of select="position()"/>
                        </xsl:variable>
                        <div id="{$cid}">
                            <table class="bglist" width="600" cellpadding="2" cellspacing="0" border="0">
                                <tr>
                                    <th colspan="3">
                                        <xsl:text>&#160;</xsl:text>
                                    </th>
                                    <th align="right">
                                        <a href="#">
                                            <img src="../../image/editicon.gif" alt="edit" width="14" height="14" border="0"
                                    onclick="parent.edit_plcredential('{$cid}','{$rpid}')"/>
                                        </a>
                                        <img src="../../image/1x1green.gif" width="10"/>
                                        <a href="#">
                                            <img src="../../image/del.gif" alt="delete" width="14" height="14" border="0"
                                    onclick="parent.del_plcredential('{$cid}')"/>
                                        </a>
                                    </th>
                                </tr>
                            </table>
                            <table class="bglist" width="600" cellpadding="2" cellspacing="1" border="0">
                                <tr>
                                    <td class="cell_fill_dark" width="19">
                                        <xsl:text>&#160;</xsl:text>
                                    </td>
                                    <td colspan="2">User ID</td>
                                    <td width="257" class="readonly">
                                        <xsl:value-of select="USERID"/>
                                    </td>
                                </tr>
                                <tr>
                                    <td class="cell_fill_dark" width="19">
                                        <xsl:text>&#160;</xsl:text>
                                    </td>
                                    <td colspan="2">Realm</td>
                                    <td width="257" class="readonly">
                                        <xsl:value-of select="REALM"/>
                                    </td>

                                </tr>
                            </table>
                            <input type="hidden"
                            name="null.PRIMARY_LINE.{$rpid}.CREDENTIAL~PRIMARY_LINE~EMPTY.USERID.{$cid}"
                            value="{USERID}" id="plc_userid"/>
                            <input type="hidden"
                                    name="null.PRIMARY_LINE.{$rpid}.CREDENTIAL~PRIMARY_LINE~EMPTY.PASSTOKEN.{$cid}"
                                    value="{PASSTOKEN}" id="plc_passtoken"/>
                            <input type="hidden"
                                    name="null.PRIMARY_LINE.{$rpid}.CREDENTIAL~PRIMARY_LINE~EMPTY.REALM.{$cid}"
                                    value="{REALM}" id="plc_realm"/>
                        </div>
                    </xsl:if>
                </xsl:for-each>
            </div>
        </xsl:for-each>
        <table class="bglist" width="600" cellpadding="2" cellspacing="1" boder="0">
            <tr>
                <td><hr class="dms"/></td>
            </tr>
        </table>
    </xsl:template>

    <xsl:template name="cs_1020">
        <xsl:for-each select="/details/configurationset/PROFILE/line1">
            <xsl:variable name="id" select="@id"/>
            <table class="bglist" width="600" cellpadding="4" cellspacing="1" border="0">
                <tr>
                    <td colspan="3">
                        line1_name (Name)
                    </td>
                    <td class="readonly" width="259">
                        <xsl:value-of select="container/line1_name"/>
                    </td>
                </tr>
                <tr>
                    <td colspan="3">
                        line1_shortname (LCD Display name)
                    </td>
                    <td class="readonly" width="259">
                        <xsl:value-of select="container/line1_shortname"/>
                    </td>
                </tr>
                <tr>
                    <td colspan="3">
                        line1_authname (Authentication name)
                    </td>
                    <td class="readonly" width="259">
                        <xsl:value-of select="container/line1_authname"/>
                    </td>
                </tr>
                <tr>
                    <td colspan="3">
                        line1_password (Authentication password)
                    </td>
                    <td class="readonly" width="259">
                        <xsl-text>********</xsl-text>
                    </td>
                </tr>
                <tr>
                    <td colspan="3">
                        line1_displayname (Caller ID name)
                    </td>
                    <td class="readonly" width="259">
                        <xsl:value-of select="container/line1_displayname"/>
                    </td>
                </tr>
            </table>
        </xsl:for-each>
        <br/>
    </xsl:template>

    <xsl:template match="refproperty" mode="buildForm">
        <xsl:param name="refpropname"/>
        <xsl:param name="code"/>
        <xsl:param name="cardinality"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="valuename"/>
        <xsl:param name="refpropcode"/>
        <xsl:variable name="displayname" select="content/definition/@description"/>
        <xsl:if test="$refpropcode = normalize-space($code)">
            <table border="0" cellpadding="2" cellspacing="0" class="bglist" width="600">
                <tr>
                    <xsl:choose>
                        <xsl:when test="content/definition/value/@type = 'container'">
                            <th colspan="3">
                                <xsl:value-of select="$displayname"/>
                            </th>
                        </xsl:when>
                        <xsl:otherwise>
                            <th colspan="2">
                                <xsl:text>&#160;</xsl:text>
                            </th>
                        </xsl:otherwise>
                    </xsl:choose>
                    <xsl:variable name="help">
                        <xsl:choose>
                            <xsl:when test="contains($code,'cs_')">
                                <xsl:text>cisco_79xx</xsl:text>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:value-of select="$code"/>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:variable>
                    <th align="right">
                        <xsl:if test="/details/setproperties/ref_property_id = id">
                            <img src="../../image/uparrow.gif" border="0" alt="set above"/>
                            <img src="../../image/1x1green.gif" border="0" width="10"/>
                        </xsl:if>
                        <a href="#">
                            <img src="../../image/info_link.gif" border="0"
                        onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/{$help}.htm','info','scrollbars,menubar,location=no,resizable,width=750,height=500')"/>
                        </a>
                    </th>
                </tr>
            </table>
            <table border="0" cellpadding="2" cellspacing="1" class="bglist" width="600">
                <xsl:choose>
                    <xsl:when test="/details/configurationset/PROFILE/child::*[name() = $refpropname]">
                        <xsl:apply-templates select="/details/configurationset/PROFILE" mode="getValues">
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="cardinality" select="$cardinality"/>
                            <xsl:with-param name="cuid">
                                <xsl:if test="$valuetype = 'container'">
                                    <xsl:choose>
                                        <xsl:when test="/details/configurationset/PROFILE/child::*[name() = $refpropname]/@id != ''">
                                            <xsl:value-of select="/details/configurationset/PROFILE/child::*[name() = $refpropname]/@id"/>
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <xsl:value-of select="generate-id()"/>
                                        </xsl:otherwise>
                                    </xsl:choose>
                                </xsl:if>
                            </xsl:with-param>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:apply-templates select="content/definition/value" mode="chkType">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="cuid">
                                <xsl:if test="$valuetype = 'container'">
                                    <xsl:value-of select="generate-id()"/>
                                </xsl:if>
                            </xsl:with-param>
                        </xsl:apply-templates>
                    </xsl:otherwise>
                </xsl:choose>
            </table>
        </xsl:if>
    </xsl:template>

    <xsl:template match="PROFILE" mode="getValues">
        <xsl:param name="refpropname"/>
        <xsl:param name="code"/>
        <xsl:param name="cardinality"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="cuid"/>
        <xsl:param name="valuename"/>
        <xsl:if test="child::*[name() = $refpropname]">
            <xsl:for-each select="child::*[name() = $refpropname]">
                <xsl:variable name="pid">
                    <xsl:if test="@id">
                        <xsl:value-of select="@id"/>
                        <xsl:variable name="cuid" select="@id"/>
                    </xsl:if>
                </xsl:variable>
                <xsl:choose>
                    <xsl:when test="$valuetype = 'container'">
                        <xsl:apply-templates select="/details/refproperties/refproperty" mode="getContainerValues">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="pid" select="$pid"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="contname" select="$valuename"/>
                            <xsl:with-param name="cuid" select="$cuid"/>
                            <xsl:with-param name="cardinality" select="$cardinality"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:apply-templates select="/details/refproperties/refproperty/content/definition/value" mode="chkType">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="value" select="descendant::*[name() = $valuename]"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                        </xsl:apply-templates>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:for-each>
        </xsl:if>
    </xsl:template>

    <xsl:template match="PROFILE" mode="getContainerValues">
        <xsl:param name="valuename"/>
        <xsl:param name="contname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="cardinality"/>
        <xsl:param name="pid"/>
        <xsl:param name="contpos"/>
        <xsl:choose>
            <xsl:when test="$pid != ''">
                <xsl:for-each select="child::*[name() = $refpropname and @id = $pid]">
                    <xsl:for-each select="descendant::*[name() = $contname]">
                        <xsl:if test="name() = $contname">
                            <xsl:for-each select=".">
                                <xsl:apply-templates select="/details/refproperties/refproperty/content/definition/value" mode="chkType">
                                    <xsl:with-param name="code" select="$code"/>
                                    <xsl:with-param name="cuid" select="$pid"/>
                                    <xsl:with-param name="refpropname" select="$refpropname"/>
                                    <xsl:with-param name="value" select="descendant::*[name() = $valuename]"/>
                                    <xsl:with-param name="valuetype" select="$valuetype"/>
                                    <xsl:with-param name="valuename" select="$valuename"/>
                                </xsl:apply-templates>
                            </xsl:for-each>
                        </xsl:if>
                    </xsl:for-each>
                </xsl:for-each>
            </xsl:when>
            <xsl:otherwise>
                <xsl:for-each select="child::*[name() = $refpropname]">
                    <xsl:for-each select="descendant::*[name() = $contname]">
                        <xsl:if test="name() = $contname">
                            <xsl:for-each select=".">
                                <xsl:apply-templates select="/details/refproperties/refproperty/content/definition/value" mode="chkType">
                                    <xsl:with-param name="code" select="$code"/>
                                    <xsl:with-param name="cuid" select="$cuid"/>
                                    <xsl:with-param name="refpropname" select="$refpropname"/>
                                    <xsl:with-param name="value" select="child::*[name() = $valuename]"/>
                                    <xsl:with-param name="valuetype" select="$valuetype"/>
                                    <xsl:with-param name="valuename" select="$valuename"/>
                                </xsl:apply-templates>
                            </xsl:for-each>
                        </xsl:if>
                    </xsl:for-each>
                </xsl:for-each>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <!-- Mode: chkType - used for building standard form elements -->
    <xsl:template match="value" mode="chkType">
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:param name="suffix"/>

        <xsl:for-each select=".">
            <xsl:if test="ancestor::refproperty/child::code = $code">
                <xsl:choose>
                    <xsl:when test="$valuetype = 'container'">
                        <xsl:apply-templates select="." mode="container">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="cuid" select="$cuid"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="$valuetype = 'enumeration'">
                        <xsl:apply-templates select="." mode="enumeration">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="cuid" select="$cuid"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                            <xsl:with-param name="suffix" select="$suffix"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="$valuetype = 'string'">
                        <xsl:apply-templates select="." mode="string">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="cuid" select="$cuid"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                            <xsl:with-param name="suffix" select="$suffix"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="$valuetype = 'integer'">
                        <xsl:apply-templates select="." mode="integer">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="cuid" select="$cuid"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                            <xsl:with-param name="suffix" select="$suffix"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="$valuetype = 'password'">
                        <xsl:apply-templates select="." mode="password">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="cuid" select="$cuid"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                            <xsl:with-param name="suffix" select="$suffix"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="$valuetype = 'digest'">
                        <xsl:apply-templates select="." mode="digest">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="cuid" select="$cuid"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                            <xsl:with-param name="suffix" select="$suffix"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="$valuetype = 'area'">
                        <xsl:apply-templates select="." mode="area">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="cuid" select="$cuid"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                            <xsl:with-param name="suffix" select="$suffix"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:otherwise>
                        <td colspan="2">Type Not Found</td>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="refproperty" mode="getContainerValues">
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="pid"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="contname"/>
        <xsl:param name="cardinality"/>
        <xsl:if test="normalize-space(code) = normalize-space($code)">
            <xsl:for-each select="name">
                <xsl:if test=". = $refpropname">
                    <tr>
                        <td colspan="4">
                            <hr class="dms"/>
                        </td>
                    </tr>
                    <xsl:for-each select="parent::*/content/definition/value">
                        <xsl:if test="@name = $contname">
                            <xsl:for-each select="value">
                                <xsl:choose>
                                    <xsl:when test="@type != 'container'">
                                        <xsl:apply-templates select="/details/configurationset/PROFILE" mode="getContainerValues">
                                            <xsl:with-param name="valuename" select="@name"/>
                                            <xsl:with-param name="contname" select="$contname"/>
                                            <xsl:with-param name="valuetype" select="@type"/>
                                            <xsl:with-param name="refpropname" select="$refpropname"/>
                                            <xsl:with-param name="code" select="$code"/>
                                            <xsl:with-param name="cuid" select="$cuid"/>
                                            <xsl:with-param name="pid" select="$pid"/>
                                        </xsl:apply-templates>
                                    </xsl:when>
                                    <xsl:otherwise>
                                    </xsl:otherwise>
                                </xsl:choose>
                            </xsl:for-each>
                        </xsl:if>
                    </xsl:for-each>
                </xsl:if>
            </xsl:for-each>
        </xsl:if>
    </xsl:template>


    <xsl:template match="value" mode="container">
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="valuename"/>
        <xsl:for-each select="value">
            <xsl:apply-templates select="." mode="chkType">
                <xsl:with-param name="code" select="$code"/>
                <xsl:with-param name="cuid" select="$cuid"/>
                <xsl:with-param name="refpropname" select="$refpropname"/>
                <xsl:with-param name="valuetype" select="@type"/>
                <xsl:with-param name="valuename" select="@name"/>
            </xsl:apply-templates>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="value" mode="enumeration">
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:param name="suffix"/>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
        <xsl:variable name="tFlag">
            <xsl:for-each select="/details/refproperties/refproperty">
                <xsl:if test="name = $refpropname">
                    <xsl:for-each select="descendant::value">
                        <xsl:if test="@name = $valuename">
                            <xsl:for-each select="preceding-sibling::*">
                                <xsl:if test="@type = 'digest'">
                                    <xsl:text>x</xsl:text>
                                </xsl:if>
                            </xsl:for-each>
                            <xsl:for-each select="following-sibling::*">
                                <xsl:if test="@type = 'digest'">
                                    <xsl:text>x</xsl:text>
                                </xsl:if>
                            </xsl:for-each>
                        </xsl:if>
                    </xsl:for-each>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="digestID">
            <xsl:choose>
                <xsl:when test="$tFlag != '' and $suffix != ''">
                    <xsl:value-of select="$suffix"/>
                </xsl:when>
                <xsl:when test="$tFlag != '' and $suffix = ''">
                    <xsl:text>1000000</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>null</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="onchangeTarget">
            <xsl:choose>
                <xsl:when test="$tFlag != ''">
                    <xsl:text>parent.calcDigest(this,</xsl:text>
                    <xsl:value-of select="$digestID"/>
                    <xsl:text>);parent.setSaveFlag()</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>parent.setSaveFlag()</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:for-each select="/details/refproperties/refproperty">
            <xsl:if test="name = $refpropname and code = $code">
                <xsl:for-each select="content/definition/descendant::*[name() = 'value']">
                    <xsl:if test="@name = $valuename">
                        <xsl:variable name="index">
                            <xsl:choose>
                                <xsl:when test="@index">
                                    <xsl:text>i_true.</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text></xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="containerUID">
                            <xsl:choose>
                                <xsl:when test="$cuid != ''">
                                    <xsl:value-of select="$cuid"/>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text>null</xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="container">
                            <xsl:apply-templates select="." mode="getContainerName"/>
                        </xsl:variable>
                        <tr>
                            <td valign="top">
                                <xsl:value-of select="@description"/>
                            </td>
                            <td colspan="2" align="left" width="50%">
                                <select id="{@name}.{$digestID}" name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}{$suffix}" size="3" onchange="{$onchangeTarget}">
                                    <option value="">&lt;BLANK&gt;</option>
                                    <xsl:for-each select="enum_value">
                                        <xsl:choose>
                                            <xsl:when test="$value = .">
                                                <option value="{.}" selected="true">
                                                    <xsl:value-of select="."/>
                                                </option>
                                            </xsl:when>
                                            <xsl:otherwise>
                                                <option value="{.}">
                                                    <xsl:value-of select="."/>
                                                </option>
                                            </xsl:otherwise>
                                        </xsl:choose>
                                    </xsl:for-each>
                                </select>
                            </td>
                        </tr>
                    </xsl:if>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="value" mode="string">
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:param name="suffix"/>
        <xsl:variable name="transcuid">
            <xsl:value-of select="translate($cuid,
    'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwyz','')"/>
        </xsl:variable>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
        <xsl:variable name="tFlag">
            <xsl:for-each select="/details/refproperties/refproperty">
                <xsl:if test="name = $refpropname">
                    <xsl:for-each select="descendant::value">
                        <xsl:if test="@name = $valuename">
                            <xsl:for-each select="preceding-sibling::*">
                                <xsl:if test="@type = 'digest'">
                                    <xsl:text>x</xsl:text>
                                </xsl:if>
                            </xsl:for-each>
                            <xsl:for-each select="following-sibling::*">
                                <xsl:if test="@type = 'digest'">
                                    <xsl:text>x</xsl:text>
                                </xsl:if>
                            </xsl:for-each>
                        </xsl:if>
                    </xsl:for-each>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="digestID">
            <xsl:choose>
                <xsl:when test="$tFlag != '' and $suffix != ''">
                    <xsl:value-of select="$suffix"/>
                </xsl:when>
                <xsl:when test="$tFlag != '' and $suffix = ''">
                    <xsl:value-of select="$transcuid"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>null</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="onchangeTarget">
            <xsl:choose>
                <xsl:when test="$tFlag != ''">
                    <xsl:text>parent.calcDigest(this,</xsl:text>
                    <xsl:value-of select="$digestID"/>
                    <xsl:text>);parent.setSaveFlag()</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>parent.setSaveFlag()</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:for-each select="/details/refproperties/refproperty">
            <xsl:if test="name = $refpropname and code = $code">
                <xsl:for-each select="content/definition/descendant::*[name() = 'value']">
                    <xsl:if test="@name = $valuename">
                        <xsl:variable name="index">
                            <xsl:choose>
                                <xsl:when test="@index">
                                    <xsl:text>i_true.</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text></xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="containerUID">
                            <xsl:choose>
                                <xsl:when test="$cuid != ''">
                                    <xsl:value-of select="$cuid"/>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text>null</xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="container">
                            <xsl:apply-templates select="." mode="getContainerName"/>
                        </xsl:variable>
                        <xsl:choose>
                            <xsl:when test="@visibility = 'hidden'">
                                <input type="hidden"
                id="{@name}.{$digestID}"
                size="40"
                name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}{$suffix}"
                onchange="{$onchangeTarget}"
                value="{@read_only_value}"/>
                            </xsl:when>
                            <xsl:otherwise>
                                <tr>
                                    <td valign="top">
                                        <xsl:value-of select="@description"/>
                                    </td>
                                    <td colspan="2" align="left" width="50%">
                                        <input type="text"
                                    id="{@name}.{$digestID}"
                                    size="40"
                                    name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}{$suffix}"
                                    onchange="{$onchangeTarget}"
                                    value="{$value}"/>
                                    </td>
                                </tr>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:if>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="value" mode="integer">
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:param name="suffix"/>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
        <xsl:variable name="tFlag">
            <xsl:for-each select="/details/refproperties/refproperty">
                <xsl:if test="name = $refpropname">
                    <xsl:for-each select="descendant::value">
                        <xsl:if test="@name = $valuename">
                            <xsl:for-each select="preceding-sibling::*">
                                <xsl:if test="@type = 'digest'">
                                    <xsl:text>x</xsl:text>
                                </xsl:if>
                            </xsl:for-each>
                            <xsl:for-each select="following-sibling::*">
                                <xsl:if test="@type = 'digest'">
                                    <xsl:text>x</xsl:text>
                                </xsl:if>
                            </xsl:for-each>
                        </xsl:if>
                    </xsl:for-each>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="digestID">
            <xsl:choose>
                <xsl:when test="$tFlag != '' and $suffix != ''">
                    <xsl:value-of select="$suffix"/>
                </xsl:when>
                <xsl:when test="$tFlag != '' and $suffix = ''">
                    <xsl:text>1000000</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>null</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="onchangeTarget">
            <xsl:choose>
                <xsl:when test="$tFlag != ''">
                    <xsl:text>parent.calcDigest(this,</xsl:text>
                    <xsl:value-of select="$digestID"/>
                    <xsl:text>);parent.setSaveFlag()</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>parent.setSaveFlag()</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:for-each select="/details/refproperties/refproperty">
            <xsl:if test="name = $refpropname and code = $code">
                <xsl:for-each select="content/definition/descendant::*[name() = 'value']">
                    <xsl:if test="@name = $valuename">
                        <xsl:variable name="index">
                            <xsl:choose>
                                <xsl:when test="@index">
                                    <xsl:text>i_true.</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text></xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="containerUID">
                            <xsl:choose>
                                <xsl:when test="$cuid != ''">
                                    <xsl:value-of select="$cuid"/>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text>null</xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="container">
                            <xsl:apply-templates select="." mode="getContainerName"/>
                        </xsl:variable>
                        <tr>
                            <td valign="top">
                                <xsl:value-of select="@description"/>
                            </td>
                            <td colspan="2" align="left" width="50%">
                                <input type="text"
                            id="{@name}.{$digestID}"
                           size="40"
                           name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}{$suffix}"
                           onchange="{$onchangeTarget}"
                           value="{$value}"/>
                            </td>
                        </tr>
                    </xsl:if>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="value" mode="area">
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:param name="suffix"/>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
        <xsl:variable name="tFlag">
            <xsl:for-each select="/details/refproperties/refproperty">
                <xsl:if test="name = $refpropname">
                    <xsl:for-each select="descendant::value">
                        <xsl:if test="@name = $valuename">
                            <xsl:for-each select="preceding-sibling::*">
                                <xsl:if test="@type = 'digest'">
                                    <xsl:text>x</xsl:text>
                                </xsl:if>
                            </xsl:for-each>
                            <xsl:for-each select="following-sibling::*">
                                <xsl:if test="@type = 'digest'">
                                    <xsl:text>x</xsl:text>
                                </xsl:if>
                            </xsl:for-each>
                        </xsl:if>
                    </xsl:for-each>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="digestID">
            <xsl:choose>
                <xsl:when test="$tFlag != '' and $suffix != ''">
                    <xsl:value-of select="$suffix"/>
                </xsl:when>
                <xsl:when test="$tFlag != '' and $suffix = ''">
                    <xsl:text>10000000</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>null</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="onchangeTarget">
            <xsl:choose>
                <xsl:when test="$tFlag != ''">
                    <xsl:text>parent.calcDigest(this,</xsl:text>
                    <xsl:value-of select="$digestID"/>
                    <xsl:text>);parent.setSaveFlag()</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>parent.setSaveFlag()</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:for-each select="/details/refproperties/refproperty">
            <xsl:if test="name = $refpropname and code = $code">
                <xsl:for-each select="content/definition/descendant::*[name() = 'value']">
                    <xsl:if test="@name = $valuename">
                        <xsl:variable name="index">
                            <xsl:choose>
                                <xsl:when test="@index">
                                    <xsl:text>i_true.</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text></xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="containerUID">
                            <xsl:choose>
                                <xsl:when test="$cuid != ''">
                                    <xsl:value-of select="$cuid"/>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text>null</xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="container">
                            <xsl:apply-templates select="." mode="getContainerName"/>
                        </xsl:variable>
                        <tr>
                            <td valign="top">
                                <xsl:value-of select="@description"/>
                            </td>
                            <td colspan="2" align="left" width="50%">

                                <textarea
                            id="{@name}.{$digestID}"
                            cols="40"
                            rows="10"
                            name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}{$suffix}"
                            onchange="{$onchangeTarget}">
                                    <xsl:value-of select="$value"/>
                                </textarea>

                            </td>
                        </tr>
                    </xsl:if>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>


    <xsl:template match="value" mode="digest">
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:param name="suffix"/>
        <xsl:variable name="transcuid">
            <xsl:value-of select="translate($cuid,
    'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwyz','')"/>
        </xsl:variable>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
        <xsl:variable name="digestID">
            <xsl:choose>
                <xsl:when test="$suffix != ''">
                    <xsl:value-of select="$suffix"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$transcuid"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:for-each select="/details/refproperties/refproperty">
            <xsl:if test="name = $refpropname and code = $code">
                <xsl:for-each select="content/definition/descendant::*[name() = 'value']">
                    <xsl:if test="@name = $valuename">
                        <xsl:variable name="index">
                            <xsl:choose>
                                <xsl:when test="@index">
                                    <xsl:text>i_true.</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text></xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="containerUID">
                            <xsl:choose>
                                <xsl:when test="$cuid != ''">
                                    <xsl:value-of select="$cuid"/>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text>null</xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="container">
                            <xsl:apply-templates select="." mode="getContainerName"/>
                        </xsl:variable>
                        <input type="hidden"
                    id="digest.{$digestID}"
                    name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}{$suffix}"
                    value="{$value}"/>
                    </xsl:if>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="value" mode="password">
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:param name="suffix"/>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
        <xsl:variable name="tFlag">
            <xsl:for-each select="/details/refproperties/refproperty">
                <xsl:if test="name = $refpropname">
                    <xsl:for-each select="descendant::value">
                        <xsl:if test="@name = $valuename">
                            <xsl:for-each select="preceding-sibling::*">
                                <xsl:if test="@type = 'digest'">
                                    <xsl:text>x</xsl:text>
                                </xsl:if>
                            </xsl:for-each>
                            <xsl:for-each select="following-sibling::*">
                                <xsl:if test="@type = 'digest'">
                                    <xsl:text>x</xsl:text>
                                </xsl:if>
                            </xsl:for-each>
                        </xsl:if>
                    </xsl:for-each>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="digestID">
            <xsl:choose>
                <xsl:when test="$tFlag != '' and $suffix != ''">
                    <xsl:value-of select="$suffix"/>
                </xsl:when>
                <xsl:when test="$tFlag != '' and $suffix = ''">
                    <xsl:text>10000000</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>null</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:variable name="onchangeTarget">
            <xsl:choose>
                <xsl:when test="$tFlag != ''">
                    <xsl:text>parent.calcDigest(this,</xsl:text>
                    <xsl:value-of select="$digestID"/>
                    <xsl:text>);parent.setSaveFlag()</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:text>parent.setSaveFlag()</xsl:text>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:for-each select="/details/refproperties/refproperty">
            <xsl:if test="name = $refpropname and code = $code">
                <xsl:for-each select="content/definition/descendant::*[name() = 'value']">
                    <xsl:if test="@name = $valuename">
                        <xsl:variable name="index">
                            <xsl:choose>
                                <xsl:when test="@index">
                                    <xsl:text>i_true.</xsl:text>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text></xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="containerUID">
                            <xsl:choose>
                                <xsl:when test="$cuid != ''">
                                    <xsl:value-of select="$cuid"/>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:text>null</xsl:text>
                                </xsl:otherwise>
                            </xsl:choose>
                        </xsl:variable>
                        <xsl:variable name="container">
                            <xsl:apply-templates select="." mode="getContainerName"/>
                        </xsl:variable>
                        <tr>
                            <td valign="top">
                                <xsl:value-of select="@description"/>
                            </td>
                            <td colspan="2" align="left" width="50%">
                                <input type="password"
                            id="{$digestID}"
                            size="40"
                            name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}{$suffix}"
                            onchange="{$onchangeTarget}"
                            value="{$value}"/>
                            </td>
                        </tr>
                    </xsl:if>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="value" mode="getContainerName">
        <xsl:choose>
            <xsl:when test="parent::value/@type = 'container'">
                <xsl:value-of select="parent::value/@name"/>
                <xsl:text>~</xsl:text>
                <xsl:apply-templates select="parent::value" mode="getContainerName"/>
            </xsl:when>
            <xsl:otherwise>
                <xsl:text>EMPTY</xsl:text>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>
