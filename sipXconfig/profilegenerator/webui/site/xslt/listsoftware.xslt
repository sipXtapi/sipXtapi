<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>
	<!-- begin processing -->
	<xsl:template match="/">
		<html>
			<body bgcolor="#FFFFCC">
				<table border="0" cellpadding="4" cellspacing="0">
					<tr>
						<th>Name</th>
						<th>Version</th>
						<th>Model</th>
						<th>Install Date</th>
						<th>Delete</th>
					</tr>
					<!-- select top level group -->
					<xsl:apply-templates select="items/item">
						<xsl:sort select="displayid"/>
					</xsl:apply-templates>
				</table>
			</body>
		</html>
	</xsl:template>

	<xsl:template match="item">
		<tr>
			<td>
                <a href="test.jsp">
                    <xsl:value-of select="name"/>
                </a>
			</td>
			<td align="center">
                <xsl:value-of select="version"/>
			</td>
			<td align="center">
                <xsl:value-of select="assoc_devicetype_model"/>
			</td>
			<td align="center">
                <xsl:value-of select="installeddate"/>
			</td>
			<td align="center">
                <!--  @JC Delete Button -->
			</td>
		</tr>
	</xsl:template>
</xsl:stylesheet>
