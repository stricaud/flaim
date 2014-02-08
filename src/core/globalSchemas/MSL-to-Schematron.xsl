<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" indent="yes" omit-xml-declaration="yes"/>
<!-- $Id: MSL-to-Schematron.xsl 335 2006-08-15 18:04:50Z kiran $ -->
<!--
XSLT stylesheet that transforms a module schema specified in MSL (Module Schema Langauge) to a Schematron schema.
-->

  <xsl:template match="/">
    <xsl:comment>
      Schematron schema generated from a module policy specified in the module policy language. 
    </xsl:comment>
    <xsl:element name="schema">
      
      <xsl:attribute name="xmlns">
	<xsl:text>http://www.ascc.net/xml/schematron</xsl:text>
      </xsl:attribute>

      <pattern name="Check that all field elements have names..">
	<rule context="field">
	  <assert test="@name">
	    Attribute name is missing for element field  {ERROR}
	  </assert>
	</rule>
      </pattern>

      <xsl:comment>
	Create the pattern to make sure we are using only allowed anonymization algorithms
      </xsl:comment>
      
      <xsl:element name="pattern">
	<xsl:attribute name="name">Valid field name check.</xsl:attribute>
	
	<!-- Selection of the rule context -->
	<xsl:element name="rule">
	  <xsl:attribute name="context">
	    <xsl:text>//field</xsl:text>
	  </xsl:attribute>
	  
	  <xsl:element name="report">
	    <xsl:attribute name="test">
	      
	      <xsl:text>not(</xsl:text>
	      <xsl:for-each select="//constraint/fieldname">
		<xsl:text>@name=&apos;</xsl:text>
		<xsl:value-of select="text()"/>
		<xsl:text>&apos;</xsl:text>
		
		<xsl:if test="not(position()=last())">
		  <xsl:text> or </xsl:text>
		</xsl:if> 
		
	      </xsl:for-each>
	      <xsl:text>)</xsl:text>
	    </xsl:attribute>
	    
	    <xsl:text>
	      {ERROR} </xsl:text> <xsl:value-of select="./fieldname"/> 
	      <xsl:text> Cannot apply anonymization algorithm to this field!
	      </xsl:text>
	  </xsl:element> 
	  
	</xsl:element> 
      </xsl:element> <!-- pattern element -->
      
      <xsl:apply-templates select="/modulepolicy/constraint"/>
    </xsl:element>    
    
    
  </xsl:template>
  
  <!--- Template to create patterns for a field -->
 <!-- <xsl:template match="/">


  </xsl:template> -->

  <!-- ******************************************
       *********************************************** -->

  <xsl:template match="/modulepolicy/constraint"> 
    <!-- Below is to create a pattern that will make sure a field name is in this -->

    <!-- Below is to create the pattern that will check the anonymization algorithm -->
    <xsl:comment>
      Definition of a pattern.
    </xsl:comment>
    <xsl:element name="pattern">
      <xsl:attribute name="name">
	<xsl:value-of select="./fieldname"/>
      </xsl:attribute>

      <!-- Selection of the rule context -->
      <xsl:element name="rule">
	<xsl:attribute name="context">
	  <xsl:text>//field[@name='</xsl:text><xsl:value-of select="./fieldname"/><xsl:text>']</xsl:text>
	</xsl:attribute>

	<!-- Report the test.. -->
	<xsl:element name="report">
	  <xsl:attribute name="test">

	    <xsl:text>not(</xsl:text>
	    <xsl:for-each select="allowedAnony/*">

<!--	      <xsl:text> last: </xsl:text>
	      <xsl:value-of select="last()"/>
	      <xsl:text> position: </xsl:text>
	      <xsl:value-of select="position()"/> -->

	      <xsl:text>*[1]=</xsl:text>
	      <xsl:value-of select="name()"/>
	      <xsl:value-of select="./fieldname"/>
	      
	      <xsl:if test="not(position()=last())">
		<xsl:text> or </xsl:text>
	      </xsl:if> 

	    </xsl:for-each>	    
	    <xsl:text>)</xsl:text>
	  </xsl:attribute>
	  <xsl:text>
	    {ERROR} </xsl:text> <xsl:value-of select="./fieldname"/> 
	  <xsl:text> is using wrong anonymization algorithm!
	  </xsl:text>
	</xsl:element> <!-- report element -->

      </xsl:element>
    </xsl:element>
    <!--
	<xsl:text>
	</xsl:text>
	  <xsl:text>
	  </xsl:text>
 -->
  </xsl:template>
  
</xsl:stylesheet>
