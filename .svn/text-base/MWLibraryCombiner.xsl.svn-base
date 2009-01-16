<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/contents ">
		<MWElements>
			<xsl:for-each select="url">
				<xsl:copy-of select="document(.)/MWElements/MWElementGroup"/>
				</xsl:for-each>
			<xsl:for-each select="url">
				<xsl:copy-of select="document(.)/MWElements/MWElement"/>
				</xsl:for-each>
			</MWElements>
		</xsl:template>
	</xsl:transform>
