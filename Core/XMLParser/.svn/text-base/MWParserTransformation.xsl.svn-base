<xsl:stylesheet version = '1.0'
	xmlns:xsl='http://www.w3.org/1999/XSL/Transform'>
		
	<xsl:template match="/">
		<xsl:element name="mw_unrolled">
			<xsl:apply-templates mode="experiment_create"/>
			<xsl:apply-templates mode="variable_create"/>
			
			<xsl:apply-templates mode="variable_assignment"/>
			
			<xsl:apply-templates mode="iodevice_create"/>
			<xsl:apply-templates mode="iochannel_create"/>
			
			<xsl:apply-templates mode="experiment_finalize"/>
			
			<xsl:apply-templates mode="iodevice_connect"/>
			<xsl:apply-templates mode="iodevice_finalize"/>
			
			
			<xsl:apply-templates mode="variable_transformers_create"/>
			
			<xsl:apply-templates mode="stimulus_group_create"/>
			<xsl:apply-templates mode="stimulus_create"/>
			<xsl:apply-templates mode="stimulus_group_connect"/>
			
			<xsl:apply-templates mode="sound_create"/>
			<xsl:apply-templates mode="list_create"/>
			<xsl:apply-templates mode="task_system_state_create"/>
			
			<xsl:apply-templates mode="paradigm_component_alias"/>
			<xsl:apply-templates mode="paradigm_component_range_replicator_alias"/>
			<xsl:apply-templates mode="paradigm_component_list_replicator_alias"/>
			
			<xsl:apply-templates mode="action_create"/>
			<xsl:apply-templates mode="action_connect"/>
			<xsl:apply-templates mode="spring_loaded_action_connect"/>
			
			<xsl:apply-templates mode="list_connect"/>
			
			
			<xsl:apply-templates mode="transition_create"/>
			
			<xsl:apply-templates mode="task_system_state_connect"/>
			
			<xsl:apply-templates mode="list_finalize"/>
			
			<xsl:element name="mw_passthrough">
				<xsl:apply-templates select="@*|node()" mode="passthrough"/>
			</xsl:element>
			
		</xsl:element>
	</xsl:template>


	
	<!-- ********************* -->
	<!-- Passthrough + tagging -->
	<!-- ********************* -->
	
	<xsl:template match="node()" mode = "passthrough">
		<xsl:copy>
			<xsl:attribute name="_id"><xsl:value-of select="generate-id(.)"/></xsl:attribute>
			<!-- passthrough -->
			<xsl:apply-templates select="@*|node()" mode="passthrough"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="@*" mode = "passthrough">
		<xsl:copy>
			<!-- passthrough -->
			<xsl:apply-templates select="@*|node()" mode="passthrough"/>
		</xsl:copy>
	</xsl:template>
	
	
	<!-- ******************* -->
	<!-- Create Passes       -->
	<!-- ******************* -->
	
	<xsl:template name="generic_create">
        <xsl:param name="parent_scope"/>
		<xsl:element name="mw_create">
			<xsl:attribute name="object">
				<xsl:value-of select="name()"/>
			</xsl:attribute>
			<xsl:attribute name="reference_id">
				<xsl:value-of select="generate-id(.)"/>
			</xsl:attribute>
            
            <xsl:if test="$parent_scope != ''">
                <xsl:attribute name="parent_scope">
                    <xsl:value-of select="$parent_scope"/>
                </xsl:attribute>
            </xsl:if>
            
			<xsl:for-each select="./@*">
				<xsl:element name="{name()}"><xsl:value-of select="."/></xsl:element>
			</xsl:for-each>
		</xsl:element>
	</xsl:template>
	

	<xsl:template name="anonymous_create">
        <xsl:param name="parent_scope"/>
		<xsl:element name="mw_anonymous_create">
			<xsl:attribute name="object">
				<xsl:value-of select="name()"/>
			</xsl:attribute>
			<xsl:attribute name="reference_id">
				<xsl:value-of select="generate-id(.)"/>
			</xsl:attribute>
            
            <xsl:if test="$parent_scope != ''">
                <xsl:attribute name="parent_scope">
                    <xsl:value-of select="$parent_scope"/>
                </xsl:attribute>
            </xsl:if>
            
			<xsl:for-each select="./@*">
				<xsl:element name="{name()}"><xsl:value-of select="."/></xsl:element>
			</xsl:for-each>
		</xsl:element>
	</xsl:template>
	
	<!-- Variable -->
	<xsl:template match="//variable" mode="variable_create">
		<xsl:call-template name="generic_create"/>
		<xsl:apply-templates select="node()" mode="variable_create"/>
	</xsl:template>

	<xsl:template match="text()" mode = "variable_create"/>

	<!-- Stimulus -->
	<xsl:template match="//stimulus" mode="stimulus_create">
		<xsl:call-template name="generic_create"/>
		<xsl:apply-templates select="node()" mode="stimulus_create"/>
	</xsl:template>

	<xsl:template match="//range_replicator" mode="stimulus_create">
		<xsl:if test=".//stimulus | .//range_replicator">
			<xsl:element name="mw_range_replicator">
				<xsl:attribute name="reference_id">
					<xsl:value-of select="generate-id(.)"/>
				</xsl:attribute>
				<xsl:attribute name="variable"><xsl:value-of select="./@variable"/></xsl:attribute>
				<xsl:attribute name="from"><xsl:value-of select="./@from"/></xsl:attribute>
				<xsl:attribute name="to"><xsl:value-of select="./@to"/></xsl:attribute>
				<xsl:attribute name="step"><xsl:value-of select="./@step"/></xsl:attribute>
				<xsl:apply-templates select="./node()" mode="stimulus_create"/>
			</xsl:element>
		</xsl:if>
	</xsl:template>
	
	<xsl:template match="//list_replicator" mode="stimulus_create">
		<xsl:if test=".//stimulus | .//list_replicator">
			<xsl:element name="mw_list_replicator">
				<xsl:attribute name="reference_id">
					<xsl:value-of select="generate-id(.)"/>
				</xsl:attribute>
				<xsl:attribute name="variable"><xsl:value-of select="./@variable"/></xsl:attribute>
				<xsl:attribute name="values"><xsl:value-of select="./@values"/></xsl:attribute>
				<xsl:apply-templates select="./node()" mode="stimulus_create"/>
			</xsl:element>
		</xsl:if>
	</xsl:template>
	
	<xsl:template match="text()" mode = "stimulus_create"/>


	<!-- Stimulus Group -->
	<xsl:template match="//stimulus_group" mode="stimulus_group_create">
		<xsl:call-template name="generic_create"/>
		<xsl:apply-templates select="node()" mode="stimulus_group_create"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "stimulus_group_create"/>

	<!-- Sound -->
	<xsl:template match="//sound" mode="sound_create">
		<xsl:call-template name="generic_create"/>
		<xsl:apply-templates select="node()" mode="sound_create"/>
	</xsl:template>

	<xsl:template match="text()" mode = "sound_create"/>

	<!-- IO Device -->
	<xsl:template match="//iodevice" mode="iodevice_create">
		<xsl:call-template name="generic_create"/>
		<xsl:apply-templates select="node()" mode="iodevice_create"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "iodevice_create"/>

	<!-- IO CHANNEL -->
	<xsl:template match="//iochannel" mode="iochannel_create">
		<xsl:call-template name="anonymous_create"/>
		<xsl:apply-templates select="node()" mode="iochannel_create"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "iochannel_create"/>

	<!-- Variable transformers -->
	<xsl:template match="//staircase | //bias_monitor | //calibrator | //filter" mode="variable_transformers_create">
		<xsl:call-template name="generic_create"/>
		<xsl:apply-templates select="node()" mode="variable_transformers_create"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "variable_transformers_create"/>

	<!-- Experiment -->
	<xsl:template match="//experiment" mode="experiment_create">
		<xsl:call-template name="generic_create"/>
		<xsl:apply-templates select="node()" mode="experiment_create"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "experiment_create"/>
		
	<!-- List-based Paradigm Components -->

	<!-- List-based Paradigm Components -->
	<xsl:template match="//protocol" mode="list_create">
		<xsl:call-template name="generic_create"/>
		<xsl:apply-templates select="node()" mode="list_create"/>
	</xsl:template>

	<xsl:template match="//block | //trial | //task_system | //list" mode="list_create">
		<xsl:call-template name="generic_create">
           <xsl:with-param name="parent_scope" select="ancestor::protocol/@tag"/>
        </xsl:call-template>
		<xsl:apply-templates select="node()" mode="list_create"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "list_create"/>
	
	<!-- Task System State -->
	<xsl:template match="//task_system_state" mode="task_system_state_create">
		<xsl:call-template name="generic_create">		
            <xsl:with-param name="parent_scope">
				<xsl:value-of select="ancestor::protocol/@tag"/>/<xsl:value-of select="ancestor::task_system/@tag"/>
			</xsl:with-param>
        </xsl:call-template>
		<xsl:apply-templates select="node()" mode="list_create"/>
	</xsl:template>
    
<!--	<xsl:template match="//task_system_state" mode="task_system_state_create">
        <xsl:element name="mw_create">
			<xsl:attribute name="object">
				<xsl:value-of select="name()"/>
			</xsl:attribute>
			<xsl:attribute name="reference_id">
				<xsl:value-of select="generate-id(.)"/>
			</xsl:attribute>
            
			<xsl:attribute name="parent_scope">
				<xsl:value-of select="ancestor::protocol/@tag"/>/<xsl:value-of select="ancestor::task_system/@tag"/>
			</xsl:attribute>
            
            <xsl:for-each select="./@*">
                <xsl:element name="{name()}"><xsl:value-of select="."/></xsl:element>
			</xsl:for-each>
		</xsl:element>
		<xsl:apply-templates select="node()" mode="task_system_state_create"/>
	</xsl:template> -->
	
	<xsl:template match="text()" mode = "paradigm_component_create"/>

	<!-- Action -->
	<xsl:template match="//action" mode="action_create">
		<xsl:call-template name="anonymous_create">
               <xsl:with-param name="parent_scope">
					<xsl:value-of select="ancestor::protocol/@tag"/>
				</xsl:with-param>
        </xsl:call-template>
		<xsl:apply-templates select="node()" mode="action_create"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "action_create"/>

	<!-- Transition -->
	<xsl:template match="//transition" mode="transition_create">
		<xsl:call-template name="anonymous_create">
               <xsl:with-param name="parent_scope">
					<xsl:value-of select="ancestor::protocol/@tag"/>/<xsl:value-of select="ancestor::task_system/@tag"/>
				</xsl:with-param>
        </xsl:call-template>
		<xsl:apply-templates select="node()" mode="transition_create"/>
	</xsl:template>
	
	<!--<xsl:template match="//transition" mode="transition_create">
		<xsl:element name="mw_anonymous_create">
			<xsl:attribute name="object">
				<xsl:value-of select="name()"/>
			</xsl:attribute>
			<xsl:attribute name="reference_id">
				<xsl:value-of select="generate-id(.)"/>
			</xsl:attribute>
			<xsl:for-each select="./@*">
				<xsl:param name="targetvar" select="name()"/>
				<xsl:choose>
					<xsl:when test="$targetvar ='target'">
						<xsl:element name="target"><xsl:value-of select="ancestor::protocol/@tag"/>/<xsl:value-of select="ancestor::task_system/@tag"/>/<xsl:value-of select="."/></xsl:element>
					</xsl:when>
					<xsl:otherwise>
						<xsl:element name="{name()}"><xsl:value-of select="."/></xsl:element>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>
		</xsl:element>
		<xsl:apply-templates select="node()" mode="transition_create"/>
	</xsl:template>	-->

	<xsl:template match="text()" mode = "transition_create"/>


	<!-- ********************* -->
	<!-- Aliasing Passes       -->
	<!-- ********************* -->

	<xsl:template name="generic_alias">
		<xsl:element name="mw_instance">
			<xsl:attribute name="object">
				<xsl:value-of select="./@tag"/>
			</xsl:attribute>
			<xsl:attribute name="type">
				<xsl:value-of select="name()"/>
			</xsl:attribute>
			<xsl:attribute name="reference_id">
				<xsl:value-of select="generate-id(.)"/>
			</xsl:attribute>
		</xsl:element>
	</xsl:template>
	
	
	
<!--	<xsl:template match="//protocol | //block | //trial | //task_system | //list" mode="paradigm_component_alias">
		<xsl:call-template name="generic_alias"/>
		<xsl:apply-templates select="node()" mode="paradigm_component_alias"/>
	</xsl:template> -->
	<xsl:template match="text()" mode = "paradigm_component_alias"/> 
	
	
	<xsl:template match="//range_replicator[count(ancestor::range_replicator) = 0]" mode="paradigm_component_range_replicator_alias">
		<xsl:call-template name="paradigm_component_range_replicator_aliaser"/>
	</xsl:template>
		
	<xsl:template name="paradigm_component_range_replicator_aliaser">
		<xsl:element name="mw_range_replicator">
			<xsl:attribute name="reference_id">
				<xsl:value-of select="generate-id(.)"/>
			</xsl:attribute>
			<xsl:attribute name="variable"><xsl:value-of select="./@variable"/></xsl:attribute>
			<xsl:attribute name="from"><xsl:value-of select="./@from"/></xsl:attribute>
			<xsl:attribute name="to"><xsl:value-of select="./@to"/></xsl:attribute>
			<xsl:attribute name="step"><xsl:value-of select="./@step"/></xsl:attribute>
			
			<xsl:if test="./@tag">
				<xsl:attribute name="tag"><xsl:value-of select="./@tag"/></xsl:attribute>
			</xsl:if>
			
			<xsl:for-each select="./*">
				<xsl:choose>
					<xsl:when test="name() = 'range_replicator'">
						<xsl:call-template name="paradigm_component_range_replicator_aliaser"/>
					</xsl:when>
					<xsl:when test="name() = 'list_replicator'">
						<xsl:call-template name="paradigm_component_list_replicator_aliaser"/>
					</xsl:when>
					<!-- TODO: Need to prevent this from aliasing things that shouldn't be aliased. A more general/robust solution is needed -->
					<xsl:when test="name() = 'stimulus'"/>
					<xsl:when test="name() = 'stimulus_group'"/>
					<!-- TODO -->
					<xsl:otherwise>
						<xsl:call-template name="generic_alias"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>
		</xsl:element>
		
		<xsl:apply-templates select="node()" mode="paradigm_component_range_replicator_alias"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "paradigm_component_range_replicator_alias"/>
	
	<xsl:template match="//list_replicator[count(ancestor::list_replicator) = 0]" mode="paradigm_component_list_replicator_alias">
		<xsl:call-template name="paradigm_component_list_replicator_aliaser"/>
	</xsl:template>
		
	<xsl:template name="paradigm_component_list_replicator_aliaser">
		<xsl:element name="mw_list_replicator">
			<xsl:attribute name="reference_id">
				<xsl:value-of select="generate-id(.)"/>
			</xsl:attribute>
			<xsl:attribute name="variable"><xsl:value-of select="./@variable"/></xsl:attribute>
			<xsl:attribute name="values"><xsl:value-of select="./@values"/></xsl:attribute>
			
			<xsl:if test="./@tag">
				<xsl:attribute name="tag"><xsl:value-of select="./@tag"/></xsl:attribute>
			</xsl:if>
			
			<xsl:for-each select="./*">
				<xsl:choose>
					<xsl:when test="name() = 'range_replicator'">
						<xsl:call-template name="paradigm_component_range_replicator_aliaser"/>
					</xsl:when>
					<xsl:when test="name() = 'list_replicator'">
						<xsl:call-template name="paradigm_component_list_replicator_aliaser"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:call-template name="generic_alias"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>
		</xsl:element>
		
		<xsl:apply-templates select="node()" mode="paradigm_component_list_replicator_alias"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "paradigm_component_list_replicator_alias"/>
	
	<!-- Alias task_system_state objects without processing range replicators -->
	<!--<xsl:template match="//task_system_state" mode="task_system_state_alias">
		<xsl:call-template name="generic_alias"/>
		<xsl:apply-templates select="node()" mode="paradigm_component_alias"/>
	</xsl:template>-->
	
	<xsl:template match="text()" mode = "task_system_state_alias"/>
	
	
	<!-- ************** -->	
	<!-- Connect Passes -->
	<!-- ************** -->
	
	<xsl:template name="generic_connect">
        <xsl:param name="parent_scope"/>
        
		<xsl:if test="count(node())">
			<xsl:element name="mw_connect">
				<xsl:attribute name="parent">
					<xsl:value-of select="./@tag"/>
				</xsl:attribute>
			
	            <xsl:attribute name="parent_scope">
	                <xsl:value-of select="$parent_scope"/>
	            </xsl:attribute>
            
				<xsl:attribute name="reference_id">
					<xsl:value-of select="generate-id(.)"/>
				</xsl:attribute>
			
				<xsl:for-each select = "./*">
					<xsl:call-template name="generic_connect_child"/>
				</xsl:for-each>
			</xsl:element>
		</xsl:if>
	</xsl:template>
	
	<xsl:template name="generic_connect_child">
		<xsl:choose>
			<xsl:when test="name() = 'range_replicator'">
				<xsl:call-template name="range_replicated_children"/>
				<!--<xsl:apply-templates mode="range_replicated_children"/>-->
			</xsl:when>
			<xsl:when test="name() = 'list_replicator'">
				<xsl:call-template name="list_replicated_children"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:element name="child">
					<xsl:attribute name="tag">
						<xsl:if test="name() = 'task_system_state'"><xsl:value-of select="ancestor::task_system/@tag"/>/</xsl:if>
						<xsl:value-of select="./@tag"/>
					</xsl:attribute>
					<xsl:attribute name="reference_id">
						<xsl:value-of select="generate-id(.)"/>
					</xsl:attribute>
				</xsl:element>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<!-- Range Replicator-->
	<xsl:template name="range_replicated_children">
			<xsl:element name="mw_range_replicator">
			<xsl:attribute name="reference_id">
				<xsl:value-of select="generate-id(.)"/>
			</xsl:attribute>
			<xsl:attribute name="variable"><xsl:value-of select="./@variable"/></xsl:attribute>
			<xsl:attribute name="from"><xsl:value-of select="./@from"/></xsl:attribute>
			<xsl:attribute name="to"><xsl:value-of select="./@to"/></xsl:attribute>
			<xsl:attribute name="step"><xsl:value-of select="./@step"/></xsl:attribute>
			<!--<xsl:for-each select = "child::*">
				<xsl:choose>
					<xsl:when test="mw_range_replicator">
						<xsl:call-template name="range_replicated_children"/>
					</xsl:when>
					<xsl:otherwise>
						<xsl:element name="child">
							<xsl:attribute name="tag">
								<xsl:value-of select="@tag"/>
							</xsl:attribute>
							<xsl:attribute name="reference_id">
								<xsl:value-of select="generate-id(.)"/>
							</xsl:attribute>
						</xsl:element>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>-->
			<xsl:for-each select="./*">
				<xsl:call-template name="generic_connect_child"/>
			</xsl:for-each>
		</xsl:element>
	</xsl:template>
		
	<!-- List Replicator-->
	<xsl:template name="list_replicated_children">
			<xsl:element name="mw_list_replicator">
			<xsl:attribute name="reference_id">
				<xsl:value-of select="generate-id(.)"/>
			</xsl:attribute>
			<xsl:attribute name="variable"><xsl:value-of select="./@variable"/></xsl:attribute>
			<xsl:attribute name="values"><xsl:value-of select="./@values"/></xsl:attribute>
			<xsl:for-each select="./*">
				<xsl:call-template name="generic_connect_child"/>
			</xsl:for-each>
		</xsl:element>
	</xsl:template>
		
	
	<xsl:template match="//experiment" mode="list_connect">
		<xsl:call-template name="generic_connect"/>
		<xsl:apply-templates select="node()" mode="list_connect"/>
	</xsl:template>

	<xsl:template match="//protocol" mode="list_connect">
		<xsl:call-template name="generic_connect">
			<xsl:with-param name="parent_scope" select="./@tag"/>
		</xsl:call-template>
		<xsl:apply-templates select="node()" mode="list_connect"/>
	</xsl:template>

	<xsl:template match="// block | //trial | //list | //task_system" mode="list_connect">
		<xsl:call-template name="generic_connect">
			<xsl:with-param name="parent_scopae" select="ancestor::protocol/@tag"/>
		</xsl:call-template>
		<xsl:apply-templates select="node()" mode="list_connect"/>
	</xsl:template>
	<xsl:template match="text()" mode = "list_connect"/>

	<xsl:template match="// block | //trial | //list | //task_system" mode="list_connect">
		<xsl:call-template name="generic_connect">
			<xsl:with-param name="parent_scope">
				<xsl:value-of select="ancestor::protocol/@tag"/>
			</xsl:with-param>
		</xsl:call-template>
		<xsl:apply-templates select="node()" mode="list_connect"/>
	</xsl:template>
	<xsl:template match="text()" mode = "list_connect"/>


	<xsl:template match="//task_system_state" mode="task_system_state_connect">
		<xsl:call-template name="generic_connect">
			<xsl:with-param name="parent_scope">
				<xsl:value-of select="ancestor::protocol/@tag"/>/<xsl:value-of select="ancestor::task_system/@tag"/>
			</xsl:with-param>
		</xsl:call-template>
		<xsl:apply-templates select="node()" mode="task_system_state_connect"/>
	</xsl:template>


	<xsl:template match="//action" mode="action_connect">
		<xsl:call-template name="generic_connect">
			<xsl:with-param name="parent_scope">anon</xsl:with-param>
		</xsl:call-template>
		<xsl:apply-templates select="node()" mode="action_connect"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "action_connect"/>
	
	<xsl:template match="//variable" mode="spring_loaded_action_connect">
		<xsl:call-template name="generic_connect"/>
		<xsl:apply-templates select="node()" mode="spring_loaded_action_connect"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "spring_loaded_action_connect"/>
	
	<xsl:template match="//stimulus_group" mode="stimulus_group_connect">
		<xsl:call-template name="generic_connect"/>
		<xsl:apply-templates select="node()" mode="stimulus_group_connect"/>
	</xsl:template>

	<xsl:template match="text()" mode = "stimulus_group_connect"/>

	<xsl:template match="//iodevice" mode="iodevice_connect">
		<xsl:call-template name="generic_connect"/>
		<xsl:apply-templates select="node()" mode="iodevice_connect"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "iodevice_connect"/>
	
	<!-- ******************* -->
	<!-- Finalize Passes       -->
	<!-- ******************* -->
	
	<xsl:template name="generic_finalize">
		<xsl:element name="mw_finalize">
			<xsl:attribute name="object">
				<xsl:value-of select="./@tag"/>
			</xsl:attribute>
			<xsl:attribute name="reference_id">
				<xsl:value-of select="generate-id(.)"/>
			</xsl:attribute>
			<xsl:for-each select="./@*">
				<xsl:element name="{name()}"><xsl:value-of select="."/></xsl:element>
			</xsl:for-each>
		</xsl:element>
	</xsl:template>
	
	
	<!-- Lists -->
	<xsl:template match="//list | //protocol | //block | //trial" mode="list_finalize">
		<xsl:call-template name="generic_finalize"/>
		<xsl:apply-templates select="node()" mode="list_finalize"/>
	</xsl:template>

	<xsl:template match="//range_replicator" mode="list_finalize">
		<xsl:if test=".//list | .//protocol | .//block | .//trial | .//range_replicator">
			<xsl:element name="mw_range_replicator">
				<xsl:attribute name="reference_id">
					<xsl:value-of select="generate-id(.)"/>
				</xsl:attribute>
				<xsl:attribute name="variable"><xsl:value-of select="./@variable"/></xsl:attribute>
				<xsl:attribute name="from"><xsl:value-of select="./@from"/></xsl:attribute>
				<xsl:attribute name="to"><xsl:value-of select="./@to"/></xsl:attribute>
				<xsl:attribute name="step"><xsl:value-of select="./@step"/></xsl:attribute>
				
				<xsl:apply-templates select="node()" mode="list_finalize"/>
			</xsl:element>
		</xsl:if>
	</xsl:template>
	
	<xsl:template match="//list_replicator" mode="list_finalize">
		<xsl:if test=".//list | .//protocol | .//block | .//trial | .//list_replicator">
			<xsl:element name="mw_list_replicator">
				<xsl:attribute name="reference_id">
					<xsl:value-of select="generate-id(.)"/>
				</xsl:attribute>
				<xsl:attribute name="variable"><xsl:value-of select="./@variable"/></xsl:attribute>
				<xsl:attribute name="values"><xsl:value-of select="./@values"/></xsl:attribute>
				
				<xsl:apply-templates select="node()" mode="list_finalize"/>
			</xsl:element>
		</xsl:if>
	</xsl:template>
	
	<xsl:template match="text()" mode = "list_finalize"/>


	<!-- Experiment -->
	<xsl:template match="//experiment" mode="experiment_finalize">
		<xsl:call-template name="generic_finalize"/>
		<xsl:apply-templates select="node()" mode="experiment_finalize"/>
	</xsl:template>
	<xsl:template match="text()" mode = "experiment_finalize"/>
	
	<!-- IO Devices -->
	<xsl:template match="//iodevice" mode="iodevice_finalize">
		<xsl:call-template name="generic_finalize"/>
		<xsl:apply-templates select="node()" mode="iodevice_finalize"/>
	</xsl:template>
	<xsl:template match="text()" mode = "iodevice_finalize"/>
	
	<!-- ********************* -->
	<!-- Variable assignments -->
	<!-- ********************* -->
	

	
	<xsl:template match="//variable_assignments/variable_assignment" mode = "variable_assignment">
			<xsl:copy-of select="."/>
			<xsl:apply-templates select="node()" mode="variable_assignment"/>
	</xsl:template>
	
	<xsl:template match="text()" mode = "variable_assignment"/>
	
</xsl:stylesheet>