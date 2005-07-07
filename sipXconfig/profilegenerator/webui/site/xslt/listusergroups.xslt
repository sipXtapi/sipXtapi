<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
   <xsl:output method="html"/>
   <!-- begin processing -->
   <xsl:template match="/">
      <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="600">
         <tr>
            <th>Name</th>
            <th width="250">Actions</th>
        </tr>
         <!-- select top level group -->
         <xsl:apply-templates select="groups/group">
            <xsl:with-param name="level" select="'0'"/>
            <xsl:sort select="name"/>
         </xsl:apply-templates>
      </table>
   </xsl:template>

   <!-- output information for a group and recursively select
   all children -->
   <xsl:template match="group">
      <xsl:param name="level"/>
      <!-- indent according to value of level -->
      <tr>
         <td valign="top">
            <div style="text-indent: {$level}em">
               <a href="../user/usergroup_frameset.jsp?usergroupid={id}&amp;mfgid=pingtel&amp;devicetypeid=common&amp;usertype=admin">
                  <xsl:value-of select="name"/>
               </a>
            </div>
         </td>
         <!-- Actions -->
         <td align="center" width="250">
            <select name="actionSelect{id}" size="1">
                <option value="0">Pick an Action</option>
                <option value="1">Send Profiles</option>
                <option value="2">Assign Applications</option>
                <option value="5">Delete User Group</option>
                <option value="6">Restart User Group Device(s)</option>
                <option value="7">Show Members</option>
            </select>
            <a href="#" onMouseOut="MM_swapImgRestore()" onclick="frmSubmit('{id}')" onMouseOver="MM_swapImage('sm_btn_ass{id}','','../buttons/go_btn_f2.gif',1)" onFocus="if(this.blur)this.blur()" >
                <img name="sm_btn_ass{id}" src="../buttons/go_btn.gif" width="23" height="17" border="0" alt="{id}"></img>
            </a>
        </td>
    </tr>
      <!-- recursively select children and increment the level -->
      <xsl:apply-templates select="group">
         <xsl:with-param name="level" select="$level + 2"/>
         <xsl:sort select="name"/>
      </xsl:apply-templates>
   </xsl:template>
</xsl:stylesheet>

