<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="html"/>
    <!-- begin processing -->
    <xsl:template match="/">
        <xsl:apply-templates select="statuses"/>
    </xsl:template>

    <xsl:template match="statuses">
        <table class="bglist" border="0" width="600" cellpadding="6" cellspacing="1">
            <tr>
                <th>ID</th>
                <th>Start Time</th>
                <th>Details</th>
                <th>Status</th>
                <th>Progress</th>
            </tr>
            <xsl:apply-templates select="jobstatus">
                <xsl:sort select="id" data-type="number" order="descending"/>
            </xsl:apply-templates>
        </table>
    </xsl:template>

    <xsl:template match="jobstatus">
        <tr>
            <td><xsl:value-of select="id"/></td>
            <td width="150"><xsl:value-of select="starttime"/></td>
            <td><xsl:value-of select="details"/></td>
            <td>
                <xsl:choose>
                    <xsl:when test="status = 'C'">
                        <xsl:text>Complete</xsl:text>
                    </xsl:when>
                    <xsl:when test="status = 'S'">
                        <xsl:text>Started</xsl:text>
                    </xsl:when>
                    <xsl:when test="status = 'F'">
                        <a href="#" onclick="window.parent.MM_openBrWindow('/pds/ui/popup/job_exception.jsp?jobid={id}','popup','scrollbars=yes,width=400,height=400')">Failed</a>
                    </xsl:when>
                    <xsl:otherwise/>
                </xsl:choose>
            </td>
            <td><xsl:value-of select="progress"/></td>
        </tr>
    </xsl:template>

</xsl:stylesheet>
