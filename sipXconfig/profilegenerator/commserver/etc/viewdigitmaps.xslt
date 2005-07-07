<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
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
        <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="1" align='center'>
            <tr>
                <th>For an end user to dial</th>
                <th>Digit map dial plan</th>
                <th> Digit map SIP address</th>
            </tr>
            <tr>
                <td>an internal station</td>
                <td><xsl:value-of select="$internalextensions"/></td>
                <td>sip:{digits}@</td>
            </tr>
            <tr>
                <td>the auto attendant</td>
                <td><xsl:value-of select="$autoattendant"/></td>
                <td>sip:{digits}@</td>
            </tr>
            <tr>
                <td>a station's voicemail</td>
                <td><xsl:value-of select="$vmprefixfromextension"/><xsl:value-of select="$internalextensions"/></td>
                <td>sip:{digits}@</td>
            </tr>
            <tr>
                <td>an outside (PSTN) number</td>
                <td><xsl:value-of select="$pstnprefix"/>1xxxxxxxxxx</td>
                <td>sip:1{vdigits}@</td>
            </tr>
            <tr>
                <td>911 (emergency)</td>
                <td><xsl:value-of select="$pstnprefix"/>911</td>
                <td>sip:911@</td>
            </tr>
            <tr>
                <td>411 (directory assistance)</td>
                <td><xsl:value-of select="$pstnprefix"/>411</td>
                <td>sip:411@</td>
            </tr>
            <tr>
                <td>an outside (PSTN) operator</td>
                <td><xsl:value-of select="$pstnprefix"/>0T</td>
                <td>sip:{digits}@</td>
            </tr><tr>
                <td>an international operator</td>
                <td><xsl:value-of select="$pstnprefix"/>00</td>
                <td>sip:{digits}@</td>
            </tr><tr>
                <td>an international number</td>
                <td><xsl:value-of select="$pstnprefix"/><xsl:value-of select="$intldialprefix"/>xxxxx.T</td>
                <td>sip:<xsl:value-of select="$intldialprefix"/>{vdigits}@</td>
            </tr>
        </table>
    </xsl:template>


</xsl:stylesheet>