<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 
 <xsl:template match="/">
  <xsl:apply-templates select="//process">
         </xsl:apply-templates>
         
   </xsl:template>

   
   
   
<xsl:template match="process">
 
  <form action="../administration/list_all_services.jsp" target="mainFrame">
        

  <table border="0" cellpadding="4" cellspacing="1" >
         <tr>
           
            <xsl:choose>
               <xsl:when test="(@status = 'Stopped') and (start/@control = 'true')"> 
                   <td>
                   <a href="../administration/details_service.jsp?command=start&amp;process={@encodedName}&amp;location={@encodedLocationID}&amp;redirect=details" 
                   target="mainFrame"
                   onMouseOut="MM_swapImgRestore()"      
                   onMouseOver="MM_swapImage('start_service_btn','','../administration/buttons/start_service_btn_f2.gif',1);" 
                   onFocus="if(this.blur)this.blur()" >
                   <img name="start_service_btn" src="../administration/buttons/start_service_btn.gif" alt="Start Service" width="113" height="34" border="0"></img>
                   </a>
                   </td>
                </xsl:when>
    			
    			<xsl:otherwise>
    			    <td align="center">
    			    <img src="../../image/spacer.gif" width="23" height="17" border="0"></img>
    			    </td>
    			</xsl:otherwise>   
           </xsl:choose>
           
           
           <xsl:choose>    
                <xsl:when test="(@status = 'Started') and (stop/@control = 'true')"> 
                   <td>
                   <a href="../administration/details_service.jsp?command=stop&amp;process={@encodedName}&amp;location={@encodedLocationID}&amp;redirect=details" 
                   target="mainFrame"
                   onMouseOut="MM_swapImgRestore()"      
                   onMouseOver="MM_swapImage('stop_service_btn','','../administration/buttons/stop_service_btn_f2.gif',1);" 
                   onFocus="if(this.blur)this.blur()" >
                   <img name="stop_service_btn" src="../administration/buttons/stop_service_btn.gif" alt="Stop Service" width="113" height="34" border="0"></img>
                   </a>
                   </td>
              </xsl:when>
    			
    			<xsl:otherwise>
    			    <td align="center">
    			    <img src="../../image/spacer.gif" width="23" height="17" border="0"></img>
    			    </td>
    			</xsl:otherwise>  
           </xsl:choose>
           
           
           <xsl:choose>    
               <xsl:when test="(@status = 'Started') and (restart/@control = 'true')">
                   <td>
                   <a href="../administration/details_service.jsp?command=restart&amp;process={@encodedName}&amp;location={@encodedLocationID}&amp;redirect=details" 
                   target="mainFrame" 
                   onMouseOut="MM_swapImgRestore()"      
                   onMouseOver="MM_swapImage('restart_service_btn','','../administration/buttons/restart_service_btn_f2.gif',1);" 
                   onFocus="if(this.blur)this.blur()" >
                   <img name="restart_service_btn" src="../administration/buttons/restart_service_btn.gif" alt="Restart Service" width="113" height="34" border="0"></img>
                   </a>
                   </td>
                </xsl:when>
    			
    			<xsl:otherwise>
    			    <td align="center">
    			    <img src="../../image/spacer.gif" width="23" height="17" border="0"></img>
    			    </td>
    			</xsl:otherwise>  
    		</xsl:choose>	   
         
         
         </tr>
       </table>
	</form>							    
</xsl:template>

</xsl:stylesheet>