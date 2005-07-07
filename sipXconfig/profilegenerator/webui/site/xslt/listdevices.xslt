<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html"/>
  <xsl:param name="idtest"/>
  <!-- begin processing -->
    <xsl:template match="/">
        <xsl:apply-templates select="groups/group">
            <xsl:sort select="name"/>
        </xsl:apply-templates>
    </xsl:template>

    <xsl:template match="group">
        <table class="bglist" width="600" border="0" cellpadding="4" cellspacing="0">
            <tr>
                <xsl:choose>
                    <xsl:when test="child::member">
                        <td align="left" width="40">
                            <a href="list_all_devices.jsp">
                                <img src="../buttons/ofolder.gif" border="0"></img>
                            </a>
                        </td>
                        <td colspan="4" align="left" class="groups">
                            <xsl:apply-templates select="." mode="getTree"/>
                        </td>
                    </xsl:when>
                    <xsl:when test="not(child::member) and $idtest = id">
                        <td align="left" width="40">
                            <a href="list_all_devices.jsp">
                                <img src="../buttons/ofolder.gif" border="0"></img>
                            </a>
                        </td>
                        <td colspan="4" align="left" class="groups">
                            <xsl:apply-templates select="." mode="getTree"/>
                        </td>
                    </xsl:when>
                    <xsl:otherwise>
                        <td align="left" width="40">
                            <a href="#" onclick="filterGroups('{id}')">
                                <img src="../buttons/cfolder.gif" alt="{name}" border="0"></img>
                            </a>
                        </td>
                        <td colspan="4" align="left" class="groups">
                            <xsl:apply-templates select="." mode="getTree"/>
                        </td>
                    </xsl:otherwise>
                </xsl:choose>
            </tr>
        </table>
        <!-- img added for spacing -->
        <img src="../../image/spacer.gif" height="1" width="1"/>
        <xsl:choose>
            <xsl:when test="child::member">
                <table class="bglist" width="600" border="0" cellpadding="2" cellspacing="0">
                    <tr>
                        <th width="110">Name</th>
                        <th width="90">S/N</th>
                        <th width="100">Group</th>
                        <th width="100">Owner</th>
                        <th width="200">Actions</th>
                    </tr>
                </table>
            </xsl:when>
            <xsl:when test="not(child::member) and $idtest = id">
                <table class="bglist" width="600" border="0" cellpadding="2" cellspacing="0">
                    <tr>
                        <th>This group currently contains no members</th>
                    </tr>
                </table>
            </xsl:when>
        </xsl:choose>
        <table class="bglist" width="600" border="0" cellpadding="2" cellspacing="1">
            <xsl:apply-templates select="member">
                <xsl:sort select="shortname"/>
            </xsl:apply-templates>
        </table>
        <xsl:apply-templates select="group">
            <xsl:sort select="name" data-type="text"/>
        </xsl:apply-templates>
    </xsl:template>

    <xsl:template match="group" mode="getTree">
        <xsl:variable name="currentNode" select="name"/>
        <xsl:for-each select="ancestor-or-self::*/name">
            <xsl:value-of select="."/>
            <xsl:if test=". != $currentNode">
                <xsl:text>  -&gt;  </xsl:text>
            </xsl:if>
       </xsl:for-each>
    </xsl:template>

   <xsl:template match="member">
    <tr id="{id}">
	 <!-- removed banding
      <xsl:attribute name="class">
        <xsl:choose>
          <xsl:when test="(position() mod 3) = 0">bgdark</xsl:when>
          <xsl:when test="(position() mod 3) = 1">bglight</xsl:when>
          <xsl:when test="(position() mod 3) = 2">bglight</xsl:when>
        </xsl:choose>
      </xsl:attribute>
	-->
      <!-- Name -->
      <td valign="top" width="110">
        <a href="../device/device_frameset.jsp?deviceid={id}&amp;mfgid={manufacturer}&amp;devicetypeid={model}&amp;usertype=admin" id="anchor{id}">
        <xsl:value-of select="shortname"/>
        </a>
      </td>
      <!-- Serial Number -->
        <td width="90" valign="top">
            <xsl:value-of select="serialnumber"/>
        </td>
      <!-- Group -->
      <td align="left" valign="top" width="100">
        <a href="../device/devicegroup_frameset.jsp?devicegroupid={../id}&amp;mfgid=pingtel&amp;devicetypeid=common&amp;usertype=admin">
        <xsl:value-of select="../name"/>
        </a>
      </td>
      <!-- Owner -->
      <td align="left" width="100">
      <xsl:choose>
        <xsl:when test="userid !=''">
          <a href="../user/user_frameset.jsp?userid={userid}&amp;mfgid=pingtel&amp;devicetypeid=common&amp;usertype=admin">
          <xsl:value-of select="userid_displayid"/>
          </a>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>&#160;</xsl:text>
          <!--<img src="../../image/1x1green.gif"/>-->
        </xsl:otherwise>
      </xsl:choose>
      </td>
      <!-- Actions -->
         <td align="center" width="200">
            <select name="actionSelect{id}" size="1">
                <option value="0">Pick an Action</option>
                <option value="1">Send Profiles</option>
                <option value="5">Delete Device</option>
                <option value="6">Restart Device</option>
            </select>
            <a href="#" onMouseOut="MM_swapImgRestore()" onclick="frmSubmit('{id}','{../id}')" onMouseOver="MM_swapImage('sm_btn_ass{id}','','../buttons/go_btn_f2.gif',1)" onFocus="if(this.blur)this.blur()" >
                <img name="sm_btn_ass{id}" src="../buttons/go_btn.gif" width="23" height="17" border="0" alt="{id}"></img>
            </a>
        </td>
    </tr>
  </xsl:template>

</xsl:stylesheet>
