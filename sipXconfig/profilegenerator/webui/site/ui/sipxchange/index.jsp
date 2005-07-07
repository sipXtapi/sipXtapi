<%@ page errorPage="/ui/error/error.jsp" import="com.pingtel.pds.common.*,
                                                java.util.*,
                                                java.io.*" %>
<%
    // get userid from session
    String userid = (String) session.getAttribute( "userid" );

    // get userdevices from session and set to the following:
    // pingtel only = pingtel
    // cisco only = cisco
    // pingtel and cisco = both
    String userDevices = (String) session.getAttribute("userdevices");
    String devicesAssigned = null;
    if ( userDevices.toLowerCase().indexOf("pingtel") != -1 &&
            userDevices.toLowerCase().indexOf("cisco") == -1 ) {
        devicesAssigned = "pingtel";
    }
    else if ( userDevices.toLowerCase().indexOf("pingtel") == -1 &&
                userDevices.toLowerCase().indexOf("cisco") != -1 ) {
        devicesAssigned = "cisco";
    }
    else if ( userDevices.toLowerCase().indexOf("pingtel") != -1 &&
                userDevices.toLowerCase().indexOf("cisco") != -1 ) {
            devicesAssigned = "both";
    }
    else {
            devicesAssigned = "pingtel";
    }
%>

<html>
<head>
<title>Pingtel Configuration Server</title>
<script src="../script/dms.js"></script>
<script language="JavaScript">
var helpwindow = null;
var userid = <%=userid%>;
var devicesAssigned = '<%=devicesAssigned%>';

    function displayHelpPage(destPage){
    if( helpwindow != null && !helpwindow.closed )
        {
            helpwindow.close();
        }
	helpwindow=window.open(destPage, 'HelpWindow', 'scrollbars,menubar,location=no,resizable,width=750,height=500');
	helpwindow.focus();
    }

    function popup( vcuid, name, rpgid, mfgid, ut, dtid )
      {
        var targetWindow = "/pds/ui/popup/form_addspeeddial.jsp?userid="
            + userid + "&propname=" + name
            + "&refpropertygroupid=" + rpgid + "&mfgid="
            + mfgid + "&usertype=" + ut + "&devicetypeid="
            + dtid + "&vcuid=" + vcuid;

        window.top.MM_openBrWindow(targetWindow,'popup','scrollbars=yes,width=600,height=300');
      }
    function del_speeddial(id)
    {
        var msg = "Are you sure you want to delete\n" +
        "this speed dial number?";
        var userInput =  confirm ( msg );
        if ( userInput == true )
        {
            mainFrame.saveFlag = true;
            var parent = mainFrame.document.getElementById("insert_speeddial");
            var divCount = 0;
            for ( var i = 0; i < parent.childNodes.length; i++ )
            {
                if( parent.childNodes[i].nodeName == "DIV" )
                {
                    divCount = divCount + 1;
                }
            }
            if ( divCount != 1 )
            {
                var child = mainFrame.document.getElementById(id);
                parent.removeChild(child);
            }
            else
            {
                mainFrame.document.getElementById("speeddial_sdid").value = "";
                mainFrame.document.getElementById("speeddial_label").value = "";
                mainFrame.document.getElementById("speeddial_type").value = "";
                mainFrame.document.getElementById("speeddial_address").value = "";
                var contParent = mainFrame.document.getElementById(id);
                if (navigator.userAgent.toLowerCase().indexOf("mozilla") != -1)
                {
                    for (var i = 0; i < contParent.childNodes.length; i++)
                    {
                        if (contParent.childNodes[i].id == "display")
                        {
                            contParent.childNodes[i].style.display = "none";
                        }
                    }
                }
                else
                {
                    mainFrame.document.getElementById(id).style.display = "none";
                }
            }
        }
    }
    function edit_speeddial(id,rpid)
    {
        var protocol = document.location.protocol;
        var host = document.location.host;
        //var theURL = "/pds/ui/popup/edit_speeddial.html?id=" + id + "&rpid=" + rpid;
        var theURL = protocol + "//" + host + "/pds/ui/popup/edit_speeddial.html?id=" + id + "&rpid=" + rpid;
        var winName = "edit";
        var features = "scrollbars=yes,width=400,height=300";
        var newWindow = window.open(theURL, winName, features);
        newWindow.focus();
    }

    function checkMfg() {
        if( devicesAssigned == "both" ) {
            mainFrame.document.getElementById("msg1").style.display = '';
        }
    }

</script>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
</head>
<%
	boolean mediaserverIsAvailable = false ;
	// TODO: Check for the availability of mediaserver and set the flag "mediaserverIsAvailable" accordingly.
    String confPath =
        PathLocatorUtil.getInstance().getPath(  PathLocatorUtil.CONFIG_FOLDER,
                                                PathLocatorUtil.PGS );

    Properties pgsProps = new Properties ();
    pgsProps.load( new FileInputStream (    confPath +
                                                System.getProperty( "file.separator") +
                                                    "pgs.props" ) );

    String mediaServerHost = pgsProps.getProperty( "mediaserver.host" );

    if ( mediaServerHost != null && mediaServerHost.trim().length() > 0 )
        mediaserverIsAvailable = true;

	if( mediaserverIsAvailable ) {
		if( session.getAttribute("mediaserverhost") == null )
			session.setAttribute("mediaserverhost", mediaServerHost);
%>
		<frameset rows="110,0,*" frameborder="NO" border="0" framespacing="0">
		  <frame name="topFrame" scrolling="NO" noresize src="banner.jsp">
		  <frame name="hiddenFrame" scrolling="NO" noresize src="notes.html">
		  <frameset cols="217,*" frameborder="NO" border="0" framespacing="0">
		    <frame name="leftFrame" scrolling="AUTO" src="navigation_frameset.html" noresize>
		    <frame name="mainFrame" src="main_frameset.jsp?multiservers=yes&userid=<%=userid%>">
		  </frameset>
		</frameset>
<%  } else { %>
		<frameset rows="110,*" frameborder="NO" border="0" framespacing="0">
		  <frame name="topFrame" scrolling="NO" noresize src="banner.jsp">
		  <frameset cols="217,*" frameborder="NO" border="0" framespacing="0">
		    <frame name="leftFrame" scrolling="AUTO" src="navigation.jsp" noresize>
		    <frame name="mainFrame" src="main_frameset.jsp?multiservers=no&userid=<%=userid%>">
		  </frameset>
		</frameset>
<% } %>
<noframes>
<body bgcolor="#FFFFFF" text="#000000">
</body>
</noframes>
</html>
