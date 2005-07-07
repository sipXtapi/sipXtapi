<html>
<head>
<title>list_all_applications_cmd</title>

<script src="../script/jsCommand.js"></script>

<link rel="stylesheet" href="../../style/dms.css" type="text/css">

</head>
<body class="bglight">

<%
  String refreshTime = (String)session.getValue("process_list_refresh");
%>	
    <form action="../administration/list_all_services.jsp" target="mainFrame">
<table border="0" cellpadding="4" cellspacing="1" >

<tr>

<td>   
<a href="../administration/list_all_services.jsp" target="mainFrame" onMouseOut="MM_swapImgRestore();"  onMouseOver="MM_swapImage('refresh_btn','','../administration/buttons/refresh_btn_f2.gif',1);" ><img name="refresh_btn" src="../administration/buttons/refresh_btn.gif" width="113" height="34" border="0"></a>
</td> 
  
  <td>&nbsp;&nbsp;</td>
  <td>&nbsp;&nbsp;</td>
  <td valign="center" class="formtext">      Refresh automatically every	
                                <select name="refresh">
                                   <% if( refreshTime != null  && refreshTime.equals("-1") ){%>
                                   <option value="-1" selected>
                                   <%}else{%>
                                   <option value="-1">
                                   <%}%>
                                   Do not refresh</option> 
								   
								   <% if( refreshTime != null  && refreshTime.equals("30") ){%>
                                   <option value="30" selected>
                                   <%}else{%>
                                   <option value="30">
                                   <%}%>
                                   30 seconds</option> 
								   
								   <% if( refreshTime != null  && refreshTime.equals("60") ){%>
                                   <option value="60" selected>
                                   <%}else{%>
                                   <option value="60">
                                   <%}%>
                                   60 seconds</option> 
								   
								   <% if( refreshTime != null  && refreshTime.equals("300") ){%>
                                   <option value="300" selected>
                                   <%}else{%>
                                   <option value="300">
                                   <%}%>
                                   5 minutes</option> 
								 </select>  
	</td>
	<td>
				<input type=image value="apply" name="apply_btn" src="../administration/buttons/apply_btn.gif" width="113" height="34" border="0" 
				onMouseOut="MM_swapImgRestore();"  onMouseOver="MM_swapImage('apply_btn','','../administration/buttons/apply_btn_f2.gif',1);">
	</td>
  </tr>
 </table> 	
	</form>							    



</body>


</html>
