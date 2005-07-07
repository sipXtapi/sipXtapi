<%--
 - Copyright (C) 2004 SIPfoundry Inc.
 - Licensed by SIPfoundry under the LGPL license.
 - 
 - Copyright (C) 2004 Pingtel Corp.
 - Licensed to SIPfoundry under a Contributor Agreement.
 --%>
<%@ page language="Java" %>

<html>
    <head>
        <script language="javascript">
        function closeWindow()
        {
            window.close();
        }
        </script>
        <title>Search</title>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    </head>
    <body class="bglight">
        <p>&nbsp;</p>
        <p class="formtitle" align="center">Device Search</p>
        <form name="frmAuthUser" action="/pds/ui/device/device_search.jsp" method="get">
            <table width="150" border="0" cellspacing="3" cellpadding="2" align="center">
				<tr>
					<td class="formtext" nowrap valign="top">Search By</td>
					<td>
						<select name="searchby">
							<option value="short_name">Device ID</option>
							<option value="serial_number">Serial Number</option>
						</select>
					</td>
				</tr>
                <tr>
                    <td class="formtext" nowrap valign="top">Search String</td>
                    <td>
                        <input type="text" name="searchstring" size="35">
                    </td>
                </tr>
                <tr>
                    <td class="formtext" nowrap valign="top">Search Type:</td>
                    <td valign="top" class="formtext">
                        <input type="checkbox" name="cb1" value="match">
                        Exact Match
                    </td>
                </tr>
            </table>
            <br>
            <div align="center">
                <input type="submit" name="cmdSubmitForm" value="Search" >
                &nbsp;&nbsp;
                <input type="button" name="cmdCloseWindow" value="Cancel" onclick="closeWindow()"/>
            </div>
        </form>
    </body>
</html>