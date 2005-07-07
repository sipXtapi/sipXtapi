<%@ page errorPage="/ui/error/error.jsp" %>
<%@ page import="java.io.IOException,
    javax.ejb.*,
    java.rmi.RemoteException,
    javax.naming.*,
    javax.rmi.PortableRemoteObject,
    com.pingtel.pds.pgs.phone.*,
    com.pingtel.pds.pgs.user.*"
%>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    String entityType = request.getParameter( "entitytype" );
    String entityID = request.getParameter( "entityid" );
    String displayText = "";
    UserHome uHome = null;
    UserGroupHome ugHome = null;
    DeviceHome dHome = null;
    DeviceGroupHome dgHome = null;

    if ( entityType.equals( "user" ) )
    {
        try
        {
            InitialContext initial = new InitialContext();
            Object objref = initial.lookup( "User" );
            uHome = ( UserHome ) PortableRemoteObject.narrow(
                objref, UserHome.class);
            User user = uHome.findByPrimaryKey( entityID );
            displayText = user.getDisplayID();
        }
        catch ( RemoteException ioe ) {
            throw new JspTagException(
                "RemoteException: " + ioe.getMessage() );
        }
        catch ( NamingException ne ) {
            throw new JspTagException(
                "NamingException: " + ne.getMessage() );
        }
        catch ( FinderException fe ) {
            throw new JspTagException(
                "FinderException: " + fe.getMessage() );
        }
    }
    else if ( entityType.equals( "usergroup" ) )
    {
        try
        {
            InitialContext initial = new InitialContext();
            Object objref = initial.lookup( "UserGroup" );
            ugHome = ( UserGroupHome ) PortableRemoteObject.narrow(
                objref, UserGroupHome.class);
            UserGroup usergroup = ugHome.findByPrimaryKey( new Integer( entityID ) );
            displayText = usergroup.getName();
        }
        catch ( RemoteException ioe ) {
            throw new JspTagException(
                "RemoteException: " + ioe.getMessage() );
        }
        catch ( NamingException ne ) {
            throw new JspTagException(
                "NamingException: " + ne.getMessage() );
        }
        catch ( FinderException fe ) {
            throw new JspTagException(
                "FinderException: " + fe.getMessage() );
        }
    }
    else if ( entityType.equals( "device" ) )
    {
        try
        {
            InitialContext initial = new InitialContext();
            Object objref = initial.lookup( "Device" );
            dHome = ( DeviceHome ) PortableRemoteObject.narrow(
                objref, DeviceHome.class);
            Device device = dHome.findByPrimaryKey( new Integer( entityID ) );
            displayText = device.getShortName();
        }
        catch ( RemoteException ioe ) {
            throw new JspTagException(
                "RemoteException: " + ioe.getMessage() );
        }
        catch ( NamingException ne ) {
            throw new JspTagException(
                "NamingException: " + ne.getMessage() );
        }
        catch ( FinderException fe ) {
            throw new JspTagException(
                "FinderException: " + fe.getMessage() );
        }
    }
    else if ( entityType.equals( "devicegroup" ) )
    {
        try
        {
            InitialContext initial = new InitialContext();
            Object objref = initial.lookup( "DeviceGroup" );
            dgHome = ( DeviceGroupHome ) PortableRemoteObject.narrow(
                objref, DeviceGroupHome.class);
            DeviceGroup devicegroup = dgHome.findByPrimaryKey( new Integer( entityID ) );
            displayText = devicegroup.getName();
        }
        catch ( RemoteException ioe ) {
            throw new JspTagException(
                "RemoteException: " + ioe.getMessage() );
        }
        catch ( NamingException ne ) {
            throw new JspTagException(
                "NamingException: " + ne.getMessage() );
        }
        catch ( FinderException fe ) {
            throw new JspTagException(
                "FinderException: " + fe.getMessage() );
        }
    }
%>
<html>
<head>
	<title>Send Profiles</title>
<link rel="stylesheet" href="../../style/dms.css" type="text/css">
<script language="JavaScript">
 function formSubmit( form )
 {

      if ( form.ppdevice.checked != true &&
	  	form.ppuser.checked != true &&
		form.ppapplication.checked != true ) {
			alert( "You must select at least one profile to send" );
		}
		else {
	  		form.submit();
		}
 }
</script>
</head>

<body class="bglight">
<p>&nbsp;</p>
<p class="tabtext" align="center"><u>Send Profiles to <%=displayText%> device(s)</u></p>
<p class="notetextdrk" align="left">Generate and deliver updated device settings, user settings, or applications to affected device(s):</p>
<form action="../profile/project.jsp" method="get" name="frmProject">
  <table width="300" border="0" cellspacing="3" cellpadding="0" align="center">
    <tr>
      <td class="formtext" nowrap>Send device settings</td>
      <td>
        <input type="checkbox" name="ppdevice" value="1" checked>
      </td>
    </tr>
    <tr>
      <td class="formtext" nowrap>Send user settings</td>
      <td>
        <input type="checkbox" name="ppuser" value="2" checked>
      </td>
    </tr>
    <tr>
      <td class="formtext" nowrap>Send application settings</td>
      <td>
        <input type="checkbox" name="ppapplication" value="5" checked>
      </td>
    </tr>

  </table>
  <br>
  <div align="center">
  <input type="button" name="cmdSubmitForm" value="Send Profiles" onclick="formSubmit(this.form)">
  <input type="Hidden" name="entityid" value='<%=request.getParameter("entityid")%>'>
  <input type="Hidden" name="entitytype" value='<%=request.getParameter("entitytype")%>'>
  </div>
 </form>
</body>
</html>
