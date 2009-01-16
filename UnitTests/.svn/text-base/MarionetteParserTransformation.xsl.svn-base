<xsl:stylesheet version = '1.0'
	xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
<!--xsl:output method="text"/-->
	<xsl:template match="text()" />
		
	<xsl:template match="expected_events">
		<xsl:element name="expected_events">
			<xsl:apply-templates mode="data_match"/>
		</xsl:element>
	</xsl:template>

	<xsl:template match="//marionette_info/expected_events/event" mode="data_match">
			<xsl:copy-of select="."/>
	</xsl:template>

</xsl:stylesheet>