<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
   <xsl:output method="html"/>
   <!-- begin processing -->
   <xsl:template match="/">
      <table border="0" width="600" cellpadding="4" cellspacing="1" class="bglist">
         <tr>
            <th width="400">Admin ID</th>
            <th>Change Password</th>
         </tr>
         <!-- select top level group -->
         <xsl:apply-templates select="items/item">
            <xsl:sort select="displayid"/>
         </xsl:apply-templates>
      </table>
   </xsl:template>

   <xsl:template match="item">
      <tr>
         <td valign="top" align="left" width="400">
            <xsl:value-of select="displayid"/>
         </td>
         <td align="center">
                <a href="#" onMouseOut="MM_swapImgRestore()" onclick="changePassword('{id}')" onMouseOver="MM_swapImage('sm_btn_cp','','../buttons/sm_btn_f2.gif',1)" onFocus="if(this.blur)this.blur()" >
                    <img name="sm_btn_cp" src="../buttons/sm_btn.gif" width="23" height="17" border="0"></img>
                </a>
            </td>
      </tr>
   </xsl:template>
</xsl:stylesheet>
