<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>
	<!-- begin processing at the root node -->
	<xsl:template match="/">
		<html>
			<body bgcolor="#FFFFCC">
				<table border="0" cellpadding="4" cellspacing="0">
					<tr>
						<th>Manufacturer</th>
						<th>Model</th>
						<th>Delete</th>
					</tr>
					<!-- select top level group -->
					<xsl:apply-templates select="items/item">
						<xsl:sort select="manufacturerid_name"/>
					</xsl:apply-templates>
				</table>
			</body>
		</html>
	</xsl:template>
	<!-- output information for a group and recursively select
      all children -->
	<xsl:template match="item">
		<tr>
			<td>
                <a href="test.jsp">
                    <xsl:value-of select="manufacturerid_name"/>
                </a>
			</td>
			<td align="center">
                <xsl:value-of select="model"/>
			</td>
			<td align="center">
                <!--  @JC TODO Place Delete Button here -->
			</td>
		</tr>
	</xsl:template>
</xsl:stylesheet>
