<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <!-- begin processing -->
    <xsl:template match="destinations">
        <xsl:apply-templates select="gateways" mode="normal"/>
        <img src="../image/spacer.gif" height="2"/>
        <xsl:apply-templates select="gateways" mode="emergency"/>
    </xsl:template>

    <!--  path to take for normal Gateways -->
    <xsl:template match="gateways" mode="normal">
        <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="0">
            <tr>
                <th colspan="2" align="center">
                    Routine Gateways
                </th>
                <th align="right">
                    <a href="#">
                        <img src="../image/add.gif" onclick="addGateway()" border="0"/>
                    </a>
                </th>
            </tr>
        </table>
        <img src="../image/spacer.gif" height="1"/>
        <div id="insert_gw">
            <xsl:apply-templates select="gateway" mode="normal">
                <xsl:sort data-type="number" order="ascending" select="@seq"/>
            </xsl:apply-templates>
        </div>
    </xsl:template>

    <xsl:template match="gateway" mode="normal">
        <xsl:for-each select=".">
            <xsl:if test="@type = 'normal'">
                <div id="gw{@id}">
                    <hr class="seperator" width="600" align="left"/>
                    <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="0">
                        <tr>
                            <th align="left" colspan="2">
                                Gateway
                            </th>
                            <th align="right">
                                <a href="#">
                                    <img src="../image/del.gif"
                                        border="0"
                                        onclick="delGateway( 'gw{@id}' )"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <xsl:variable name="id">
                        <xsl:value-of select="@id"/>
                    </xsl:variable>
                    <!--<hr class="seperator" width="600" align="left"/>-->
                    <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="1">
                        <tr>
                            <td width="259">Description</td>
                            <td colspan="2">
                                <input
                                    type="text"
                                    size="40"
                                    name="gw{$id}_label"
                                    id="gw{$id}_label"
                                    value='{@label}'
                                    onchange="setFlag()"/>
                            </td>
                        </tr>
                        <tr>
                            <td width="259">Address</td>
                            <td colspan="2">
                                <input
                                    type="text"
                                    size="40"
                                    name="gw{$id}"
                                    id="gw{$id}"
                                    value='{primary_name}'
                                    onchange="setFlag()"/>
                                <input type="hidden"
                                    name="gw{$id}_q"
                                    id="gw{$id}_q"
                                    value='{@qvalue}'/>
                                <input type="hidden"
                                    name="gw{$id}_seq"
                                    id="gw{$id}_seq"
                                    value='{@seq}'/>
                            </td>
                        </tr>
                        <tr>
                            <td colspan="3">
                                <a href="#" onclick="moveUp('insert_gw','gw{$id}')">Move Up</a>&#160;&#160;
                                <a href="#" onclick="moveDown('insert_gw','gw{$id}')">Move Down</a>
                            </td>
                        </tr>
                    </table>
                    <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="0">
                        <tr>
                            <th colspan="2" align="left">
                                Alternates
                            </th>
                            <th align="right">
                                <a href="#">
                                    <img src="../image/add.gif"
                                        onclick="addAlt( 'gw{$id}' )"
                                        border="0"/>
                                </a>
                            </th>
                        </tr>
                    </table>

                    <xsl:for-each select="alias">
                        <div id="gw{$id}_alt_{position()}">
                            <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="1">
                                <tr>
                                    <td width="259">Alternate Address</td>
                                    <td colspan="2">
                                        <input
                                            type="text"
                                            size="40"
                                            name="gw{$id}_alt_{position()}"
                                            value="{.}"
                                            onchange="setFlag()"/>
                                        <xsl:text>&#160;</xsl:text>
                                        <a href="#">
                                            <img src="../image/del.gif"
                                                border="0"
                                                onclick="delAlt( 'gw{$id}','gw{$id}_alt_{position()}' )"/>
                                        </a>
                                    </td>
                                </tr>
                            </table>
                        </div>
                    </xsl:for-each>
                </div>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <!-- Path to take for Emergency Gateways -->
    <xsl:template match="gateways" mode="emergency">
        <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="0">
            <tr>
                <th colspan="2" align="center">
                    Emergency Gateways
                </th>
                <th align="right">
                    <a href="#">
                        <img src="../image/add.gif"
                            onclick="addEmergencyGateway()"
                            border="0"/>
                    </a>
                </th>
            </tr>
        </table>
        <img src="../image/spacer.gif" height="1"/>
        <div id="insert_egw">
            <xsl:apply-templates select="gateway" mode="emergency">
                <xsl:sort data-type="number" order="ascending" select="@seq"/>
            </xsl:apply-templates>
        </div>
    </xsl:template>

    <xsl:template match="gateway" mode="emergency">
        <xsl:for-each select=".">
            <xsl:if test="@type = 'emergency'">
                <div id="egw{@id}">
                    <hr class="seperator" width="600" align="left"/>
                    <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="0">
                        <tr>
                            <th align="left" colspan="2">
                                Emergency Gateway
                            </th>
                            <th align="right">
                                <a href="#">
                                    <img src="../image/del.gif"
                                        border="0"
                                        onclick="delEmergencyGateway( 'egw{@id}' )"/>
                                </a>
                            </th>
                        </tr>
                    </table>
                    <xsl:variable name="id">
                        <xsl:value-of select="@id"/>
                    </xsl:variable>
                    <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="1">
                        <tr>
                            <td width="259">Description</td>
                            <td colspan="2">
                                <input
                                    type="text"
                                    size="40"
                                    name="egw{$id}_label"
                                    id="egw{$id}_label"
                                    value='{@label}'
                                    onchange="setFlag()"/>
                            </td>
                        </tr>
                        <tr>
                            <td width="259">Address</td>
                            <td colspan="2">
                                <input
                                    type="text"
                                    size="40"
                                    name="egw{$id}"
                                    id="egw{$id}"
                                    value='{primary_name}'
                                    onchange="setFlag()"/>
                                <input type="hidden"
                                    name="egw{$id}_q"
                                    id="egw{$id}_q"
                                    value='{@qvalue}'/>
                                <input type="hidden"
                                    name="egw{$id}_seq"
                                    id="egw{$id}_seq"
                                    value='{@seq}'/>
                            </td>
                        </tr>
                        <tr>
                            <td colspan="3">
                                <a href="#" onclick="moveUp('insert_egw','egw{$id}')">Move Up</a>&#160;&#160;
                                <a href="#" onclick="moveDown('insert_egw','egw{$id}')">Move Down</a>
                            </td>
                        </tr>
                    </table>
                    <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="0">
                        <tr>
                            <th colspan="2" align="left">
                                Alternates
                            </th>
                            <th align="right">
                                <a href="#">
                                    <img src="../image/add.gif"
                                        onclick="addAlt( 'egw{$id}' )"
                                        border="0"/>
                                </a>
                            </th>
                        </tr>
                    </table>

                    <xsl:for-each select="alias">
                        <div id="egw{$id}_alt_{position()}">
                            <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="1">
                                <tr>
                                    <td width="259">Alternate Address</td>
                                    <td colspan="2">
                                        <input
                                            type="text"
                                            size="40"
                                            name="egw{$id}_alt_{position()}"
                                            value="{.}"
                                            onchange="setFlag()"/>
                                        <xsl:text>&#160;</xsl:text>
                                        <a href="#">
                                            <img src="../image/del.gif"
                                                border="0"
                                                onclick="delAlt( 'egw{$id}','egw{$id}_alt_{position()}' )"/>
                                        </a>
                                    </td>
                                </tr>
                            </table>
                        </div>
                    </xsl:for-each>
                </div>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>
