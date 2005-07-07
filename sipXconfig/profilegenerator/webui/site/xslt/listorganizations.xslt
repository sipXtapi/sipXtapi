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
						<th># of Users</th>
						<th># of Devices</th>
						<th>Delete</th>
					</tr>
					<!-- select top level group -->
					<xsl:apply-templates select="items/item">
						<xsl:sort select="name"/>
					</xsl:apply-templates>
				</table>
			</body>
		</html>
	</xsl:template>

	<!-- output information for a group and recursively select all children -->
	<xsl:template match="item">
		<tr>
			<td>
                <a href="test.jsp">
                    <xsl:value-of select="name"/>
                </a>
			</td>
			<td align="center">
                <!--  # of Users -->
			</td>
			<td align="center">
                <!-- # of Devices -->
			</td>
			<td align="center">
                <!--  Delete Button -->
			</td>
		</tr>
	</xsl:template>
</xsl:stylesheet>
