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
						<th>Groups</th>
						<th>Priveleges</th>
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
	<!-- output information for a group and recursively select
      all children -->
	<xsl:template match="item">
		<tr>
			<td>
                <a href="test.jsp">
                    <xsl:value-of select="displayid"/>
                </a>
			</td>
			<td align="center">
                <!--  @JC TODO Put AdminGroup information -->
			</td>
			<td align="center">
                <!--  @JC TODO Populate List Box Here -->
			</td>
			<td align="center">
                <!--  @JC TODO Place Delete Button here -->
			</td>
		</tr>
	</xsl:template>
</xsl:stylesheet>
