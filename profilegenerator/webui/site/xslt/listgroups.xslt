<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="html"/>
	<!-- begin processing -->
	<xsl:template match="/">
		<html>
			<body bgcolor="#FFFFCC">
				<table border="0" cellpadding="4" cellspacing="0">
					<tr>
						<th>Group Name</th>
						<th>ID</th>
						<th>Parent ID</th>
						<th>Org ID</th>
						<th>RefSet ID</th>
					</tr>
					<!-- select top level group -->
					<xsl:apply-templates select="groups/group">
						<xsl:with-param name="level" select="'0'"/>
						<xsl:sort select="name"/>
					</xsl:apply-templates>
				</table>
			</body>
		</html>
	</xsl:template>
	<!-- output information for a group and recursively select
      all children -->
	<xsl:template match="group">
		<xsl:param name="level"/>
		<!-- indent according to value of level -->
		<tr>
			<td>
				<div style="text-indent: {$level}em">
					<a href="test.jsp?orgid={organizationid}&amp;usergroup={name}">
						<xsl:value-of select="name"/>
					</a>
				</div>
			</td>
			<td align="center">
				<xsl:value-of select="groupid"/>
			</td>
			<td align="center">
				<xsl:value-of select="parentgroupid"/>
			</td>
			<td align="center">
				<xsl:value-of select="organizationid"/>
			</td>
			<td align="center">
				<xsl:value-of select="refconfigsetid"/>
			</td>
		</tr>
		<!-- recursively select children and increment the level -->
		<xsl:apply-templates select="group">
			<xsl:with-param name="level" select="$level + 2"/>
			<xsl:sort select="name"/>
		</xsl:apply-templates>
	</xsl:template>
</xsl:stylesheet>
