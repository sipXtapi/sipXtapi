<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/>
    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:variable name="pstnprefix">
            <xsl:for-each select="//dialplan">
                <xsl:if test="name = 'pstnprefix'">
                    <xsl:value-of select="value"/>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="internalextensions">
            <xsl:for-each select="//dialplan">
                <xsl:if test="name = 'internalextensions'">
                    <xsl:value-of select="value"/>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="did">
            <xsl:for-each select="//dialplan">
                <xsl:if test="name = 'did'">
                    <xsl:value-of select="value"/>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="autoattendant">
            <xsl:for-each select="//dialplan">
                <xsl:if test="name = 'autoattendant'">
                    <xsl:value-of select="value"/>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="retrievevoicemail">
            <xsl:for-each select="//dialplan">
                <xsl:if test="name = 'retrievevoicemail'">
                    <xsl:value-of select="value"/>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
        <xsl:variable name="vmprefixfromextension">
            <xsl:for-each select="//dialplan">
                <xsl:if test="name = 'vmprefixfromextension'">
                    <xsl:value-of select="value"/>
                </xsl:if>
            </xsl:for-each>
        </xsl:variable>
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
                <hostPattern>${SIPXCHANGE_DOMAIN_NAME}</hostPattern>
                <hostPattern>${MY_FULL_HOSTNAME}</hostPattern>
                <hostPattern>${MY_HOSTNAME}</hostPattern>
                <hostPattern>${MY_IP_ADDR}</hostPattern>
                <userMatch>
                    <xsl:comment>Emergency dialing.  Allow both "911" and "[prefix_digit_PSTN]911".</xsl:comment>
                    <userPattern>sos</userPattern>
                    <userPattern><xsl:value-of select="$pstnprefix"/>911</userPattern>
                    <userPattern>911</userPattern>
                    <xsl:choose>
                        <xsl:when test="//gateway/@type= 'emergency'">
                            <xsl:apply-templates select="//gateway" mode="emergency">
                                <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                                <xsl:with-param name="user">
                                    <xsl:text>911</xsl:text>
                                </xsl:with-param>
                            </xsl:apply-templates>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:apply-templates select="//gateway" mode="normal">
                                <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                                <xsl:with-param name="user">
                                    <xsl:text>911</xsl:text>
                                </xsl:with-param>
                            </xsl:apply-templates>
                        </xsl:otherwise>
                    </xsl:choose>
                </userMatch>
                <xsl:if test="$did != ''">
                    <xsl:comment>This is added if DID is present</xsl:comment>
                    <userMatch>
                        <userPattern><xsl:value-of select="$internalextensions"/></userPattern>
                        <permissionMatch>
                            <transform>
                                <user><xsl:value-of select="$did"/>{vdigits}</user>
                                <host>${SIPXCHANGE_DOMAIN_NAME}</host>
                            </transform>
                        </permissionMatch>
                    </userMatch>
                </xsl:if>
                <userMatch>
                    <xsl:comment>operator or extension [auto_attendant_extn] goes to AutoAttendant</xsl:comment>
                    <userPattern>operator</userPattern>
                    <userPattern><xsl:value-of select="$autoattendant"/></userPattern>
                    <permissionMatch>
                        <transform>
                            <url>&lt;sip:{digits}@{mediaserver};play={voicemail}%2Fsipx-cgi%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dautoattendant&gt;</url>
                        </transform>
                    </permissionMatch>
                </userMatch>
                <userMatch>
                    <xsl:comment>extension [retrieve_voicemail_extn] is used to retrieve voicemail messages</xsl:comment>
                    <userPattern><xsl:value-of select="$retrievevoicemail"/></userPattern>
                    <permissionMatch>
                        <transform>
                            <url>&lt;sip:{digits}@{mediaserver};play={voicemail}%2Fsipx-cgi%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Dretrieve%26mailbox%3D{digits}&gt;</url>
                        </transform>
                    </permissionMatch>
                </userMatch>
                <userMatch>
                    <xsl:comment>transferring a call to [prefix_digit_direct_voicemail][internal_ext_format] transfers
                        the call to the voicemail box for extension [internal_ext_format]</xsl:comment>
                    <userPattern><xsl:value-of select="$vmprefixfromextension"/><xsl:value-of select="$internalextensions"/></userPattern>
                    <permissionMatch>
                        <transform>
                            <url>&lt;sip:{vdigits}@{mediaserver};play={voicemail}%2Fsipx-cgi%2Fvoicemail%2Fmediaserver.cgi%3Faction%3Ddeposit%26mailbox%3D{vdigits}&gt;</url>
                        </transform>
                    </permissionMatch>
                </userMatch>
                <userMatch>
                    <xsl:comment>800 number dialing.  Allow optional "1" or "[prefix_digit_PSTN]1" prefixes</xsl:comment>
                    <userPattern><xsl:value-of select="$pstnprefix"/>1800xxxxxxx.</userPattern>
                    <userPattern>1800xxxxxxx.</userPattern>
                    <userPattern>800xxxxxxx.</userPattern>
                    <xsl:apply-templates select="//gateway" mode="normal">
                        <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        <xsl:with-param name="user">
                            <xsl:text>800{vdigits}</xsl:text>
                        </xsl:with-param>
                    </xsl:apply-templates>
                </userMatch>
                <userMatch>
                    <xsl:comment>877 number dialing.  Allow optional "1" or "[prefix_digit_PSTN]1" prefixes.</xsl:comment>
                    <userPattern><xsl:value-of select="$pstnprefix"/>1877xxxxxxx.</userPattern>
                    <userPattern>1877xxxxxxx.</userPattern>
                    <userPattern>877xxxxxxx.</userPattern>
                    <xsl:apply-templates select="//gateway" mode="normal">
                        <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        <xsl:with-param name="user">
                            <xsl:text>877{vdigits}</xsl:text>
                        </xsl:with-param>
                    </xsl:apply-templates>
                </userMatch>
                <userMatch>
                    <xsl:comment>888 number dialing.  Allow optional "1" or "[prefix_digit_PSTN]1" prefixes.</xsl:comment>
                    <userPattern><xsl:value-of select="$pstnprefix"/>1888xxxxxxx.</userPattern>
                    <userPattern>1888xxxxxxx.</userPattern>
                    <userPattern>888xxxxxxx.</userPattern>
                    <xsl:apply-templates select="//gateway" mode="normal">
                        <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        <xsl:with-param name="user">
                            <xsl:text>888{vdigits}</xsl:text>
                        </xsl:with-param>
                    </xsl:apply-templates>
                </userMatch>
                <userMatch>
                    <xsl:comment>900 number dialing.  Allow optional "1" or "[prefix_digit_PSTN]1" prefixes.</xsl:comment>
                    <userPattern><xsl:value-of select="$pstnprefix"/>1900xxxxxxx.</userPattern>
                    <userPattern>1900xxxxxxx.</userPattern>
                    <userPattern>900xxxxxxx.</userPattern>
                    <xsl:apply-templates select="//gateway" mode="normal">
                        <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        <xsl:with-param name="user">
                            <xsl:text>900{vdigits}</xsl:text>
                        </xsl:with-param>
                    </xsl:apply-templates>
                </userMatch>
                <userMatch>
                    <xsl:comment>Local/long distance number dialing.
                        Allow optional "1" or "[prefix_digit_PSTN]1" prefixes.</xsl:comment>
                    <userPattern><xsl:value-of select="$pstnprefix"/>1[2-9]xxxxxxxxx.</userPattern>
                    <userPattern>1[2-9]xxxxxxxxx.</userPattern>
                    <userPattern>[2-9]xxxxxxxxx.</userPattern>
                    <xsl:apply-templates select="//gateway" mode="normal">
                        <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        <xsl:with-param name="user">
                            <xsl:text>{vdigits}</xsl:text>
                        </xsl:with-param>
                    </xsl:apply-templates>
                </userMatch>
                <userMatch>
                    <xsl:comment>International number dialing.
                        Do not allow dialing intl numbers with a "[prefix_digit_PSTN]" prefix as this
                        would conflict with the long distance pattern "[2-9]xxxxxxxxx."</xsl:comment>
                    <userPattern><xsl:value-of select="$intldialprefix"/>x.</userPattern>
                    <xsl:apply-templates select="//gateway" mode="normal">
                        <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        <xsl:with-param name="user">
                            <xsl:value-of select="$intldialprefix"/>{vdigits}</xsl:with-param>
                    </xsl:apply-templates>
                </userMatch>
                <userMatch>
                    <xsl:comment>Information dialing.  Allow both "411" and "[prefix_digit_PSTN]411".</xsl:comment>
                    <userPattern><xsl:value-of select="$pstnprefix"/>411</userPattern>
                    <userPattern>411</userPattern>
                    <xsl:apply-templates select="//gateway" mode="normal">
                        <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        <xsl:with-param name="user">
                            <xsl:text>411</xsl:text>
                        </xsl:with-param>
                    </xsl:apply-templates>
                </userMatch>
                <userMatch>
                    <xsl:comment>PSTN operator dialing.  Allow only "[prefix_digit_PSTN]0"</xsl:comment>
                    <userPattern><xsl:value-of select="$pstnprefix"/>0</userPattern>
                    <xsl:apply-templates select="//gateway" mode="normal">
                        <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        <xsl:with-param name="user">
                            <xsl:text>0</xsl:text>
                        </xsl:with-param>
                    </xsl:apply-templates>
                </userMatch>
                <userMatch>
                    <xsl:comment>PSTN international operator dialing.  Allow only "[prefix_digit_PSTN]00".</xsl:comment>
                    <userPattern><xsl:value-of select="$pstnprefix"/>00</userPattern>
                    <xsl:apply-templates select="//gateway" mode="normal">
                        <xsl:sort data-type="number" order="descending" select="@qvalue"/>
                        <xsl:with-param name="user">
                            <xsl:text>00</xsl:text>
                        </xsl:with-param>
                    </xsl:apply-templates>
                </userMatch>
            </hostMatch>
        </mappings>
    </xsl:template>


    <xsl:template match="gateway" mode="emergency">
        <xsl:param name="user"/>
        <xsl:for-each select=".">
            <xsl:if test="@type = 'emergency'">
                <permissionMatch>
                    <transform>
                        <user><xsl:value-of select="$user"/></user>
                        <host><xsl:value-of select="primary_name"/></host>
                        <fieldparams>Q=<xsl:value-of select="@qvalue"/></fieldparams>
                    </transform>
                </permissionMatch>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

    <xsl:template match="gateway" mode="normal">
        <xsl:param name="user"/>
        <xsl:for-each select=".">
            <xsl:if test="@type = 'normal'">
                <permissionMatch>
                    <transform>
                        <user><xsl:value-of select="$user"/></user>
                        <host><xsl:value-of select="primary_name"/></host>
                        <fieldparams>Q=<xsl:value-of select="@qvalue"/></fieldparams>
                    </transform>
                </permissionMatch>
            </xsl:if>
        </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>
