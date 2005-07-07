<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <xsl:param name="refpropertygroupid"/>
    <xsl:param name="mfgid"/>
    <xsl:param name="devicetypeid"/>
    <xsl:param name="usertype"/>
    <xsl:param name="detailstype"/>


    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:if test="$usertype != 'user'">
            <div class="formtext">
                <xsl:value-of select="$refpropertygroupid"/> <!-- refpropertygroupid = Call Handling -->
            </div>
            <hr align="left" class="dms" width="580"/>
        </xsl:if>
        <xsl:apply-templates select="/details/refpropertygroup_rp_mappings/property_group_link/manufacturer"/>
    </xsl:template>

    <xsl:template match="manufacturer">
        <xsl:param name="name"/>
        <xsl:if test="@name = $mfgid">
            <xsl:for-each select="device_type">
                <xsl:if test="@model = $devicetypeid">
                    <xsl:apply-templates select="ref_property_group">
                        <xsl:with-param name="name" select="$refpropertygroupid"/>
                    </xsl:apply-templates>
                </xsl:if>
            </xsl:for-each>
        </xsl:if>
    </xsl:template>

    <xsl:template match="ref_property_group">
        <xsl:param name="name"/>
        <xsl:if test="@name = $name and @usertype = $usertype">
            <xsl:apply-templates select="ref_property"/>
        </xsl:if>
    </xsl:template>

    <xsl:template match="ref_property">
        <xsl:variable name="code" select="@code"/>
        <xsl:apply-templates select="/details/refproperties/refproperty" mode="getProperty">
            <xsl:with-param name="code" select="$code"/>
        </xsl:apply-templates>
    </xsl:template>

    <xsl:template match="refproperty" mode="getProperty">
        <xsl:param name="code"/>
        <xsl:if test="normalize-space(code) = normalize-space($code)">
            <xsl:apply-templates select="." mode="buildForm">
                <xsl:with-param name="refpropname" select="name"/>
                <xsl:with-param name="code" select="$code"/>
                <xsl:with-param name="cardinality" select="content/definition/value/@cardinality"/>
                <xsl:with-param name="valuetype" select="content/definition/value/@type"/>
                <xsl:with-param name="valuename" select="content/definition/value/@name"/>
                <xsl:with-param name="refpropcode" select="normalize-space(code)"/>
            </xsl:apply-templates>
        </xsl:if>
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
            <div id="{$code}">
                <table border="0" cellpadding="2" cellspacing="0" class="bglist" width="600">
                    <tr>
                        <th colspan="2">
                            <img src="../../image/1x1green.gif"/>
                        </th>
                        <th align="right">
                            <xsl:if test="/details/setproperties/ref_property_id = id">
                                <img src="../../image/uparrow.gif" border="0" alt="set above"/>
                                <img src="../../image/1x1green.gif" border="0" width="10"/>
                            </xsl:if>
                            <a href="#">
                                <img src="../../image/info_link.gif" border="0"
                            onclick="window.top.MM_openBrWindow('/pds/ui/help/info/WebHelp/{$code}.htm','info','scrollbars=yes,width=600,height=400')"/>
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
                            </xsl:apply-templates>
                            <tr>
                                <td colspan="4">
                                    <hr class="dms"/>
                                </td>
                            </tr>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:apply-templates select="content/definition/value" mode="chkType">
                                <xsl:with-param name="code" select="$code"/>
                                <xsl:with-param name="valuename" select="$valuename"/>
                                <xsl:with-param name="valuetype" select="$valuetype"/>
                                <xsl:with-param name="refpropname" select="$refpropname"/>
                            </xsl:apply-templates>
                        </xsl:otherwise>
                    </xsl:choose>
                </table>
            </div>
        </xsl:if>
    </xsl:template>

    <xsl:template match="PROFILE" mode="getValues">
        <xsl:param name="refpropname"/>
        <xsl:param name="code"/>
        <xsl:param name="cardinality"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="valuename"/>
        <xsl:if test="child::*[name() = $refpropname]">
            <xsl:for-each select="child::*[name() = $refpropname]">
                <xsl:variable name="pid">
                    <xsl:if test="@id">
                        <xsl:value-of select="@id"/>
                    </xsl:if>
                </xsl:variable>
                <xsl:apply-templates select="/details/refproperties/refproperty/content/definition/value" mode="chkType">
                    <xsl:with-param name="code" select="$code"/>
                    <xsl:with-param name="refpropname" select="$refpropname"/>
                    <xsl:with-param name="value" select="descendant::*[name() = $valuename]"/>
                    <xsl:with-param name="valuetype" select="$valuetype"/>
                    <xsl:with-param name="valuename" select="$valuename"/>
                </xsl:apply-templates>
            </xsl:for-each>
        </xsl:if>
    </xsl:template>

    <!-- Mode: chkType - used for building standard form elements -->
    <xsl:template match="value" mode="chkType">
        <xsl:param name="code"/>
        <xsl:param name="cuid"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:for-each select=".">
            <xsl:if test="ancestor::refproperty/child::code = $code">
                <xsl:choose>
                    <xsl:when test="$valuetype = 'enumeration'">
                        <xsl:apply-templates select="." mode="enumeration">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="$valuetype = 'string'">
                        <xsl:apply-templates select="." mode="string">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="$valuetype = 'integer'">
                        <xsl:apply-templates select="." mode="integer">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="$valuetype = 'password'">
                        <xsl:apply-templates select="." mode="password">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:when test="$valuetype = 'area'">
                        <xsl:apply-templates select="." mode="area">
                            <xsl:with-param name="code" select="$code"/>
                            <xsl:with-param name="refpropname" select="$refpropname"/>
                            <xsl:with-param name="valuetype" select="$valuetype"/>
                            <xsl:with-param name="valuename" select="$valuename"/>
                            <xsl:with-param name="value" select="$value"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:otherwise>
                        <td colspan="2">Type Not Found</td>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="value" mode="enumeration">
        <xsl:param name="code"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
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
                            <xsl:text>null</xsl:text>
                        </xsl:variable>
                        <xsl:variable name="container">
                            <xsl:apply-templates select="." mode="getContainerName"/>
                        </xsl:variable>
                        <tr>
                            <td valign="top">
                                <xsl:value-of select="@description"/>
                            </td>
                            <td colspan="2" align="left" width="50%">
                                <select id="{@name}" name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}" size="3" onchange="checkVis();setSaveFlag()">
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
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
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
                            <xsl:text>null</xsl:text>
                        </xsl:variable>
                        <xsl:variable name="container">
                            <xsl:apply-templates select="." mode="getContainerName"/>
                        </xsl:variable>
                        <xsl:choose>
                            <xsl:when test="@visibility = 'hidden'">
                                <input type="hidden"
                                    id="{@name}"
                                    size="40"
                                    name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}"
                                    value="{@read_only_value}"
                                    onchange="setSaveFlag()"/>
                            </xsl:when>
                            <xsl:otherwise>
                                <tr>
                                    <td valign="top">
                                        <xsl:value-of select="@description"/>
                                    </td>
                                    <td colspan="2" align="left" width="50%">
                                        <input type="text"
                                    id="{@name}"
                                    size="40"
                                    name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}"
                                    value="{$value}"
                                    onchange="setSaveFlag()"/>
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
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
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
                            <xsl:text>null</xsl:text>
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
                                    id="{@name}"
                                   size="40"
                                   name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}"
                                   value="{$value}"
                                   onchange="setSaveFlag()"/>
                            </td>
                        </tr>
                    </xsl:if>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="value" mode="area">
        <xsl:param name="code"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
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
                            <xsl:text>null</xsl:text>
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
                                    id="{@name}"
                                    cols="40"
                                    rows="10"
                                    name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}"  >
                                    <xsl:value-of select="$value"/>
                                </textarea>
                           </td>
                        </tr>
                    </xsl:if>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>


    <xsl:template match="value" mode="password">
        <xsl:param name="code"/>
        <xsl:param name="refpropname"/>
        <xsl:param name="valuetype"/>
        <xsl:param name="value"/>
        <xsl:param name="valuename"/>
        <xsl:variable name="refpropertyid" select="ancestor::refproperty/child::id"/>
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
                            <xsl:text>null</xsl:text>
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
                                    id="{@name}"
                                    size="40"
                                    name="{$containerUID}.{$refpropname}.{$refpropertyid}.{$container}.{@name}.{$index}"
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
