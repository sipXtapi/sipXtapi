<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

   <xsl:template match="/">

         <!-- select top level group -->


         <xsl:apply-templates select="//host">
         <xsl:sort select="@id"/>
         </xsl:apply-templates>

   </xsl:template>


   <!-- this template is for displaying host and groups/process in the host. -->

   <xsl:template match="host">
       <xsl:param name="hostlocation">
                <xsl:value-of select="@encodedID" />
       </xsl:param>


      <table border="0" cellpadding="4" cellspacing="1" class="bglist">
          <tr>
                <th width="200">Name</th>
                <th>Status</th>
                <th>Start</th>
                <th>Stop</th>
                <th>Restart</th>
          </tr>
          <!-- process host -->
           <tr>
        		<td>
        		    <a name="location_{$hostlocation}_group_"></a>
        			<xsl:value-of select="@id"/>
        			<br></br>
        			<i><xsl:value-of select="@name"/></i>
        		</td>

        		<xsl:call-template name="host_and_group_template" >
        		   <xsl:with-param name="hostlocation" select="$hostlocation"/>
        		</xsl:call-template>

           </tr>


            <!-- process groups -->
            <xsl:for-each select="process_definitions/group[@name !='Misc']">
                <xsl:sort order="ascending" select="@name"/>
                <tr>
            		<td>
            		<a name="location_{$hostlocation}_group_{@encodedName}"></a>

            		<div style="text-indent: 2em">
            			     <xsl:value-of select="@name"/>
            			</div>
                    </td>
                    <xsl:call-template name="host_and_group_template" >
                       <xsl:with-param name="hostlocation" select="$hostlocation"/>
                       <xsl:with-param name="group" select="@encodedName" />
                    </xsl:call-template>
                </tr>

                <!-- process processes -->
                <xsl:for-each select="process">
                  <xsl:sort order="ascending" select="@name"/>
                  <xsl:call-template name="process_template" >
                    <xsl:with-param name="hostlocation" select="$hostlocation"/>
                  </xsl:call-template>
                </xsl:for-each>

            </xsl:for-each>

         </table>
         <br></br>
         <br></br>
	 </xsl:template>



	<!-- this template is for displaying process rows. -->

    <xsl:template name="process_template">
          <xsl:param name="hostlocation"/>


          <tr>
    		<td>
    		<a name="location_{$hostlocation}_process_{@encodedName}"></a>
    		<div style="text-indent: 4em">
    <a href="details_service.jsp?process={@encodedName}&amp;location={$hostlocation}&amp;redirect=details"><xsl:value-of select="@name"/>
    			   </a>
    			 </div>
            </td>

            <td align="center">
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

                  <xsl:when test="@status = 'Failed'">
                     <div style="color:red">
                     <xsl:value-of select="@status"/>
                     </div>
                  </xsl:when>

                  <xsl:otherwise>
                     <div><xsl:value-of select="@status"/></div>
                  </xsl:otherwise>
             </xsl:choose>
            </td>



        	<xsl:choose>

    			<xsl:when test="((@status = 'Stopped') or (@status = 'Failed')) and (start/@control = 'true')">
        		   	<td align="center">
            			<a href="list_all_services.jsp?command=start&amp;process={@encodedName}&amp;location={$hostlocation}#location_{$hostlocation}_group_"
            			onMouseOut="MM_swapImgRestore()"
            			onMouseOver="MM_swapImage('sm_btn_start_{$hostlocation}_{@encodedName}','','../buttons/sm_btn_f2.gif',1)"
            			onFocus="if(this.blur)this.blur()" >
        				<img name="sm_btn_start_{$hostlocation}_{@encodedName}" src="../buttons/sm_btn.gif" width="23" height="17" border="0"></img>
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
        			<td align="center">
        			<a href="list_all_services.jsp?command=stop&amp;process={@encodedName}&amp;location={$hostlocation}#location_{$hostlocation}_group_"
        			 onMouseOut="MM_swapImgRestore()"
        			 onMouseOver="MM_swapImage('sm_btn_stop_{$hostlocation}_{@encodedName}','','../buttons/sm_btn_f2.gif',1)"
        			 onFocus="if(this.blur)this.blur()" >
    				<img name="sm_btn_stop_{$hostlocation}_{@encodedName}" src="../buttons/sm_btn.gif" width="23" height="17" border="0"></img>
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
        			<td align="center">
        			<a href="list_all_services.jsp?command=restart&amp;process={@encodedName}&amp;location={$hostlocation}#location_{$hostlocation}_group_"
        			onMouseOut="MM_swapImgRestore()"
        			onMouseOver="MM_swapImage('sm_btn_restart_{$hostlocation}_{@encodedName}','','../buttons/sm_btn_f2.gif',1)"
        			onFocus="if(this.blur)this.blur()" >
    				<img name="sm_btn_restart_{$hostlocation}_{@encodedName}" src="../buttons/sm_btn.gif" width="23" height="17" border="0"></img>
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
   </xsl:template>





<!-- this template is for groups and host rows. -->
    <xsl:template name="host_and_group_template">
        <xsl:param name="hostlocation"/>
        <xsl:param name="group"/>

        <xsl:choose>

            <xsl:when test="@status = 'unavailable'">
                <td align="center">
                  <div style="color:red">
                    Unavailable
                  </div>
                </td>
                <td></td>
                <td></td>
                <td></td>

            </xsl:when>

            <xsl:otherwise>
                <td></td>
                <xsl:choose>

        			<xsl:when test="@start = 'enable'">
            			<td align="center">
            			<a href="list_all_services.jsp?command=start&amp;group={$group}&amp;location={$hostlocation}#location_{$hostlocation}_group_{@encodedName}"
            			onMouseOut="MM_swapImgRestore()"
            			onMouseOver="MM_swapImage('sm_btn_del{$group}start','','../buttons/sm_btn_f2.gif',1)"
            			onFocus="if(this.blur)this.blur()" >
        				<img name="sm_btn_del{$group}start" src="../buttons/sm_btn.gif" width="23" height="17" border="0"></img>
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
                   <xsl:when test="@stop = 'enable'">
            			<td align="center">
            			<a href="list_all_services.jsp?command=stop&amp;group={$group}&amp;location={$hostlocation}#location_{$hostlocation}_group_{@encodedName}"
            			onMouseOut="MM_swapImgRestore()"
            			onMouseOver="MM_swapImage('sm_btn_del{$group}stop','','../buttons/sm_btn_f2.gif',1)"
            			onFocus="if(this.blur)this.blur()" >
        				<img name="sm_btn_del{$group}stop" src="../buttons/sm_btn.gif" width="23" height="17" border="0"></img>
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
        			<xsl:when test="@restart = 'enable'">
            			<td align="center">
            			<a href="list_all_services.jsp?command=restart&amp;group={$group}&amp;location={$hostlocation}#location_{$hostlocation}_group_{@encodedName}"
            			onMouseOut="MM_swapImgRestore()"
            			onMouseOver="MM_swapImage('sm_btn_del{$group}restart','','../buttons/sm_btn_f2.gif',1)"
            			onFocus="if(this.blur)this.blur()" >
        				<img name="sm_btn_del{$group}restart" src="../buttons/sm_btn.gif" width="23" height="17" border="0"></img>
        			    </a>
            			</td>
        			</xsl:when>

        			<xsl:otherwise>
        			    <td align="center">
        			    <img src="../../image/spacer.gif" width="23" height="17" border="0"></img>
        			    </td>
        			</xsl:otherwise>
            	</xsl:choose>
            </xsl:otherwise>
          </xsl:choose>

    </xsl:template>


</xsl:stylesheet>
