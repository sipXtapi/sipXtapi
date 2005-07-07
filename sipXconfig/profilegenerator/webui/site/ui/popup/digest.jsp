
<%@ page import="java.io.*,
                java.util.*,
                com.pingtel.pds.pgs.common.*,
                com.pingtel.pds.common.*,
                java.net.*" %>

<%
    String userid = request.getParameter( "userid" );
    String password = request.getParameter( "password" );
    String realm = request.getParameter( "realm" );
    String target = request.getParameter( "target" );
    String oldid = request.getParameter( "oldid" );
    String replace = request.getParameter( "replace" );
    String rpid = request.getParameter( "rpid" );
    String ulid = null;
    if ( request.getParameter( "ulid" ) != null )
    {
        ulid = request.getParameter( "ulid" );
    }
    if ( password == null )
                   password = "";

    MD5Encoder MD5 = new MD5Encoder();

    String passtoken = MD5.encode( userid + ":" + realm + ":" + password );

%>

<html>
    <head>
        <script src="../script/validate.js"></script>
        <script language="JavaScript">
            var validator = new Validate();
            function loadDigest(target)
            {
                var userid = validator.trim('<%=userid%>');
                var passtoken = validator.trim('<%=passtoken%>');
                var realm = validator.trim('<%=realm%>');
                var rpid = validator.trim('<%=rpid%>');
                var replace = validator.trim('<%=replace%>');
                var oldid = validator.trim('<%=oldid%>');
                if ( target == "authuser" )
                {
                    self.opener.top.mainFrame.add_authuser(userid,
                    passtoken, realm, rpid,
                    replace, oldid);
                    this.window.close();
                }
                else if ( target == "plcredential" )
                {
                    self.opener.top.mainFrame.add_plcredential(userid,
                    passtoken, realm, rpid,
                    replace, oldid);
                    this.window.close();
                }
                else if ( target == "ulcredential" )
                {
                    var ulid = validator.trim('<%=ulid%>');
                    self.opener.top.mainFrame.add_ulcredential(userid,
                    passtoken, realm, rpid, ulid,
                    replace, oldid);
                    this.window.close();
                }
            }
        </script>
        <link rel="stylesheet" href="../../style/dms.css" type="text/css">
    </head>
    <body class="bglight" onload="loadDigest('<%=target%>')">
    <p class="formtitle" align="center">Calculating Digest...</p>
    </body>
</html>
