<xsl:stylesheet version = '1.0'
     xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>


<!-- Reorganize the groups -->
<xsl:template match="/" name="reorganize_groups">
	<xsl:element name="MWElementLibrary">
		
		<xsl:element name="MWElementGroup">
			<xsl:attribute name="name">All</xsl:attribute>
			<xsl:attribute name="icon">smallFolder</xsl:attribute>
			<xsl:attribute name="description">A group containing all available objects</xsl:attribute>
			<xsl:for-each select="/MWElements/MWElement">
				<xsl:call-template name="copy_element"/>
			</xsl:for-each>
			
		</xsl:element>
		
		<xsl:for-each select="/MWElements/MWElementGroup">
			<xsl:call-template name="group_template"/>
		</xsl:for-each>
	</xsl:element>		
</xsl:template>



<xsl:template name="group_template">
	<xsl:copy select=".">
		<xsl:variable name="thisname" select="./@name"/>
		<xsl:attribute name="name"><xsl:value-of select="$thisname"/></xsl:attribute>
		<xsl:attribute name="description">
			<xsl:value-of select="./description"/>
		</xsl:attribute> 
		<xsl:attribute name="icon">
			<xsl:value-of select="./icon"/>
		</xsl:attribute>
		<xsl:attribute name="match_signature">
			<xsl:value-of select="./match_signature"/>
		</xsl:attribute>
		<xsl:attribute name="leaf">0</xsl:attribute>
		
		<xsl:for-each select="./MWElementGroup">
			<xsl:call-template name="group_template"/>
		</xsl:for-each>
		
		<xsl:for-each select="/MWElements/MWElement">
			<xsl:variable name="does_it_belong">
				<xsl:call-template name="recursive_belongs">
					<xsl:with-param name="group" select="$thisname"/>
				</xsl:call-template>
			</xsl:variable>
			
			<xsl:message>
				<xsl:value-of select="./@name"/>
				<xsl:text> belongs(</xsl:text>
				<xsl:value-of select="$does_it_belong"/>
				<xsl:text> ) to </xsl:text>
				<xsl:value-of select="$thisname"/>
			</xsl:message>
		
			<xsl:if	test="$does_it_belong = $thisname and not(./display_as_group) and not(./@hidden)">
				<xsl:call-template name="copy_element"/>
			</xsl:if>
		</xsl:for-each>
	</xsl:copy>
</xsl:template>

<xsl:template name="recursive_belongs">
	<xsl:param name="group"/>
	
	<xsl:variable name="belongs">
		<xsl:value-of select="./belongs_to_group"/>
	</xsl:variable>
	
	<xsl:if test="$belongs = $group">
		<!-- drop out the value -->
		<xsl:value-of select="$belongs"/>
	</xsl:if>
	<xsl:if test="not($belongs = $group) and ./isa">
		<xsl:variable name="is_a" select="./isa"/>
		<xsl:for-each select="/MWElements/MWElement">
			<xsl:if test="./@name = $is_a">
				<xsl:message>Recursing...
					(<xsl:value-of select="./@name"/> =
					 <xsl:value-of select="$is_a"/> )
				</xsl:message>
				<xsl:call-template name="recursive_belongs">
					<xsl:with-param name="group" select="$group"/>
				</xsl:call-template>
			</xsl:if>	
		</xsl:for-each>
	</xsl:if>
</xsl:template>

<xsl:template name="copy_element">
	<xsl:copy select=".">
		<xsl:copy-of select="./@name"/>
		<xsl:attribute name="description">
			<xsl:value-of select="./description"/>
		</xsl:attribute>
		<xsl:attribute name="icon">
			<xsl:value-of select="./icon"/>
		</xsl:attribute>
		<xsl:attribute name="leaf">1</xsl:attribute>
		<xsl:copy-of select="./code"/>
		
		<!-- Inherit properties -->
		<xsl:call-template name="inherit_properties"/>
		
		<!-- <xsl:if test="./isa">
			
			<xsl:variable name="is_a" select="./isa"/>
			<xsl:for-each select="/MWElements/MWElement">
				<xsl:if test="./@name = $is_a">
					<xsl:copy-of select="./allowed_parent"/>
					<xsl:copy-of select="./disallowed_sibling_before"/>
					<xsl:copy-of select="./disallowed_sibling_after"/>
					<xsl:copy-of select="./belongs_to_group"/>
				</xsl:if>
			</xsl:for-each>
		</xsl:if> -->
			
	</xsl:copy>
</xsl:template>

<xsl:template name="inherit_properties">
	<xsl:copy-of select="./allowed_parent"/>
	<xsl:copy-of select="./disallowed_sibling_before"/>
	<xsl:copy-of select="./disallowed_sibling_after"/>
	<!--<xsl:copy-of select="./icon"/>-->
	<xsl:copy-of select="./belongs_to_group"/>
	
	<xsl:if test="./isa">
		<xsl:variable name="is_a" select="./isa"/>
		<xsl:for-each select="/MWElements/MWElement">
			<xsl:if test="./@name = $is_a">
				<xsl:call-template name="inherit_properties"/>
			</xsl:if>
		</xsl:for-each>
	</xsl:if>
</xsl:template>
	
	

<!-- Drop the belongs_to_group elements -->
<!-- <xsl:template match="belongs_to_group/*" name="delete_belongs_to_group" mode="stage2"/> -->


</xsl:stylesheet>
