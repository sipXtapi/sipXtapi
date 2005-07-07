<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
   
   <xsl:template match="/">
         
         <!-- select top level group -->
         <xsl:apply-templates select="//process">
         </xsl:apply-templates>
         
   </xsl:template>

   
   
   
   <xsl:template match="process">
        <xsl:param name="processEncodedName">
                <xsl:value-of select="@encodedName"/>
       </xsl:param>
        <xsl:param name="encodedLocationID">
                <xsl:value-of select="@encodedLocationID"/>
       </xsl:param>
       
       
       
      <h1 class="list"><xsl:value-of select="@name"/></h1>
      <hr class="dms"></hr>

      <table border="0" cellpadding="4" cellspacing="1" class="bglist">
          <!-- process host -->
                
                <tr>
        	      <th colspan="2" align="middle"><xsl:value-of select="@name"/></th>
                </tr>
           
                 <tr>
                    <td>Group</td>
                    <td><a href="list_all_services.jsp#location_{@encodedLocationID}_group_{@encodedGroupName}">
                    <xsl:value-of select="@group"/></a></td>
                 </tr>
           
                 <tr>
                    <td>Status</td>
                 
                     <td>
                        <xsl:choose>
                              <xsl:when test="@status = 'Stopped'"> 
                                 <div style="color:red"> 
                                 <xsl:value-of select="@status"/>
                                 </div>
                              </xsl:when> 
                              
                              <xsl:when test="@status = 'Started'"> 
                                 <div style="color:green"> 
                                 <xsl:value-of select="@status"/>
                                 </div>
                              </xsl:when> 
                              
                              <xsl:otherwise>
                                 <div><xsl:value-of select="@status"/></div>
                              </xsl:otherwise>
                         </xsl:choose>        
                    </td>
                 </tr>
                  
                 
                 <tr>
                    <td>Host</td>
                    <td><a href="list_all_services.jsp#location_{@encodedLocationID}_group_">
                    <xsl:value-of select="@location_id"/>
                    <br></br><i> <xsl:value-of select="@location_hostname"/> </i></a></td>
                 </tr>
                 
                 <tr>
                    <td>PIDs</td>
                    <td>
                     <xsl:for-each select="pid"> 
                         <xsl:choose>
                                 <xsl:when test=". = '-1'"> 
                                </xsl:when>
                            
                                <xsl:otherwise>
                                       <xsl:value-of select="."/>
                                </xsl:otherwise>
                        </xsl:choose>
                     </xsl:for-each>   
                    </td>         
                 </tr>
                 
                 <tr>
                    <td>Uptime</td>
                    <td>
                        <xsl:for-each select="uptime"> 
                            <xsl:value-of select="."/>
                        </xsl:for-each>   
                    </td>
                 </tr>
                          
                 <tr>
                    <td>Dependencies</td>
                     
                    <td>    
                         <xsl:choose>
                             <xsl:when test="@dependencies_size = '0'"> 
                                none                   
                            </xsl:when>
                        
                            <xsl:otherwise>
                                 <table cellspacing="5">
                                 <xsl:for-each select="dependency"> 
                                    <xsl:sort order="ascending" select="."/> 
                                    <tr><td>
                                    <a href="details_service.jsp?process={@encodedName}&amp;location={$encodedLocationID}&amp;redirect=details">
                                    <xsl:value-of select="."/></a>
                                    </td></tr>
                                </xsl:for-each>
                                </table>  
                           </xsl:otherwise>
                	    </xsl:choose>		         
                   </td>
                 
                 </tr>
                
                   <tr>
                    <td>Dependents</td>
                    <td>    
                         <xsl:choose>
                             <xsl:when test="@dependents_size = '0'"> 
                                none                   
                            </xsl:when>
                        
                            <xsl:otherwise>
                                 <table cellspacing="5">
                                 <xsl:for-each select="dependent"> 
                                    <xsl:sort order="ascending" select="@name"/> 
                                    <tr><td>
                                    <a href="details_service.jsp?process={@encodedName}&amp;location={$encodedLocationID}&amp;redirect=details">
                                    <xsl:value-of select="@name"/></a>
                                    </td></tr>
                                </xsl:for-each>
                                </table>  
                           </xsl:otherwise>
                	    </xsl:choose>		         
                   </td>
                 </tr>
              </table>   
              <br></br>
           
    </xsl:template>
 
 </xsl:stylesheet>
