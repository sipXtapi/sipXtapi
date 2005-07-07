<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>

    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:variable name="intldialprefix">
            <xsl:for-each select="//dialplan">
                <xsl:if test="name = 'intldialprefix'">
                    <xsl:value-of select="value"/>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <mappings>
            <xsl:comment>WARNING: Do NOT edit or delete this file!
            SIPxchange automatically creates this file from the system-wide
            services and dial plans you configure in the SIPxchange interface for administrators</xsl:comment>
            <hostMatch>
                <xsl:comment>No permissions are required to dial an emergency number.</xsl:comment>
                <xsl:choose>
                    <xsl:when test="//gateway/@type= 'emergency'">
                        <xsl:apply-templates select="//gateway" mode="emergency">
                            <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        </xsl:apply-templates>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:apply-templates select="//gateway" mode="normal">
                            <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        </xsl:apply-templates>
                    </xsl:otherwise>
                </xsl:choose>

                <userMatch>
                    <userPattern>sos</userPattern>
                    <userPattern>911</userPattern>
                    <permissionMatch>
                    </permissionMatch>
                </userMatch>
            </hostMatch>
            <hostMatch>
                <xsl:comment>No permissions are required to dial an emergency number.</xsl:comment>
                <xsl:apply-templates select="//gateway" mode="normal">
                    <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                </xsl:apply-templates>
                <userMatch>
                    <xsl:comment> Dialing an "800" number requires LocalDialing permission.
                        Accept "800" numbers with an optional "1" prefix.</xsl:comment>
                    <userPattern>1800xxxxxxx.</userPattern>
                    <userPattern>1888xxxxxxx.</userPattern>
                    <userPattern>1877xxxxxxx.</userPattern>
                    <userPattern>800xxxxxxx.</userPattern>
                    <userPattern>888xxxxxxx.</userPattern>
                    <userPattern>877xxxxxxx.</userPattern>
                    <permissionMatch>
                        <permission>LocalDialing</permission>
                    </permissionMatch>
                </userMatch>
                <userMatch>
                    <xsl:comment>Dialing a "900" number requires 900Dialing permission.
                        Accept "900" numbers with an optional "1" prefix.</xsl:comment>
                    <userPattern>1900xxxxxxx.</userPattern>
                    <userPattern>900xxxxxxx.</userPattern>
                    <permissionMatch>
                        <permission>900Dialing</permission>
                    </permissionMatch>
                </userMatch>
                <userMatch>
                    <xsl:comment> Ten digit numbers that start with a digit in the [2-9]
                        set of digits or eleven digit numbers that include
                        an optional "1" prefix represent outside calls and require
                        LongDistanceDialing permission</xsl:comment>
                    <userPattern>1[2-9]xxxxxxxxx.</userPattern>
                    <userPattern>[2-9]xxxxxxxxx.</userPattern>
                    <permissionMatch>
                        <permission>LongDistanceDialing</permission>
                    </permissionMatch>
                </userMatch>
                <userMatch>
                    <xsl:comment>International calls are designated by the [prefix_international] prefix and
                        require InternationalDialing permission. </xsl:comment>
                    <userPattern><xsl:value-of select="$intldialprefix"/>x.</userPattern>
                    <permissionMatch>
                        <permission>InternationalDialing</permission>
                    </permissionMatch>
                </userMatch>
                <userMatch>
                    <xsl:comment>Allow calls to 411, 0 (PSTN operator) and 00 (international operator)</xsl:comment>
                    <userPattern>411</userPattern>
                    <userPattern>0</userPattern>
                    <userPattern>00</userPattern>
                    <permissionMatch>
                        <permission>LocalDialing</permission>
                    </permissionMatch>
                </userMatch>
            </hostMatch>
            <hostMatch>
                <xsl:apply-templates select="//gateway" mode="emergency">
                    <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                </xsl:apply-templates>
                <xsl:apply-templates select="//gateway" mode="normal">
                    <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                </xsl:apply-templates>
                <userMatch>
                    <userPattern>.</userPattern>
                    <permissionMatch>
                        <permission>NoAccess</permission>
                    </permissionMatch>
                </userMatch>
            </hostMatch>
        </mappings>
    </xsl:template>

    <xsl:template match="gateway" mode="emergency">
        <xsl:for-each select=".">
            <xsl:if test="@type = 'emergency'">
                <hostPattern><xsl:value-of select="primary_name"/></hostPattern>
                <xsl:for-each select="alias">
                    <hostPattern><xsl:value-of select="."/></hostPattern>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="gateway" mode="normal">
        <xsl:for-each select=".">
            <xsl:if test="@type = 'normal'">
                <hostPattern><xsl:value-of select="primary_name"/></hostPattern>
                <xsl:for-each select="alias">
                    <hostPattern><xsl:value-of select="."/></hostPattern>
                </xsl:for-each>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>
