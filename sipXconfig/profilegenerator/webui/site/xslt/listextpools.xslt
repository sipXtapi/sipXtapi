<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
   <xsl:output method="html"/>
   <!-- begin processing -->
   <xsl:template match="/">
      <table border="0" cellpadding="4" cellspacing="1" class="bglist" width="600">
         <tr>
            <th>Name</th>
            <th width="200">Actions</th>
        </tr>
         <!-- select top level group -->
         <xsl:apply-templates select="extensionpools/extensionpool">
            <xsl:sort select="name"/>
         </xsl:apply-templates>
      </table>
   </xsl:template>

   <!-- output information for a group and recursively select
   all children -->
    <xsl:template match="extensionpool">
        <tr>
            <td valign="top">
                <a href="extpool_details.jsp?extensionpoolid={id}">
                    <xsl:value-of select="name"/>
                </a>
            </td>
            <!-- Actions -->
            <td align="left" width="200">
                <select name="actionSelect{id}" size="1">
                    <option value="0">Pick an Action</option>
                    <option value="1">Add Extensions</option>
                    <option value="2">Delete Extensions</option>
                    <xsl:if test="name != 'reserved'">
                        <option value="3">Delete Ext Pool</option>
                    </xsl:if>
                </select>
                <a href="#" onMouseOut="MM_swapImgRestore()" onclick="frmSubmit('{id}')" onMouseOver="MM_swapImage('sm_btn_ass{id}','','../buttons/go_btn_f2.gif',1)" onFocus="if(this.blur)this.blur()" >
                    <img name="sm_btn_ass{id}" src="../buttons/go_btn.gif" width="23" height="17" border="0"></img>
                </a>
           </td>
        </tr>
    </xsl:template>
</xsl:stylesheet>

