<?xml version="1.0" encoding="ISO-8859-1"?>
<!-- Edited with XML Spy v2007 (http://www.altova.com) -->
<xsl:stylesheet xmlns:suse="http://novell.com/package/metadata/suse/patches" version="1.0"
xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:template match="/">  
  <xsl:for-each select="patches/patch">
    <xsl:value-of select="@id"/>    <br/>
    <xsl:value-of select="checksum/@type"/><br/>
    <xsl:value-of select="checksum"/><br/>
    <xsl:value-of select="location/@href"/><br/>
  </xsl:for-each>
</xsl:template>
</xsl:stylesheet>
