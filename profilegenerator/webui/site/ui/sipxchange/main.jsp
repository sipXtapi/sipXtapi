<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
   // get userid from session
   String userid = (String) session.getAttribute( "userid" );
%>
<html>
<head>
<link rel="stylesheet" href="style/dms_sipxchange.css" type="text/css">
<link rel="stylesheet" type="text/css" href="../../style/dms.css">
<style type="text/css">
h1.list {
	font-size: 20px;
	text-align: left;
	padding: 10px;
}
</style>
<script language="javascript">
    function checkBrowser()
    {
        // convert all characters to lowercase to simplify testing
        var agt = navigator.userAgent.toLowerCase();

        // **** Browser Version ***
        var is_major = parseInt( navigator.appVersion );
        var is_minor = parseFloat( navigator.appVersion );

        // check for netscape navigator
        var is_nav = ((agt.indexOf('mozilla') != -1) && (agt.indexOf('spoofer') == -1)
                && (agt.indexOf('compatitble') == -1) && (agt.indexOf('opera') == -1)
                && (agt.indexOf('webtv') == -1) && (agt.indexOf('hotjava') == -1));
        var is_nav6up = (is_nav && (is_major >= 5));

        // check for ie
        var is_ie = ((agt.indexOf('msie') != -1) && (agt.indexOf('opera') == -1));
        var is_ie3 = (is_ie && (is_major < 4));
        var is_ie4 = (is_ie && (is_major == 4) && (agt.indexOf('msie 4') != -1));
        var is_ie4up = (is_ie && (is_major >= 4));
        var is_ie5 = (is_ie && (is_major == 4) && (agt.indexOf('msie 5.0') != -1));
        var is_ie5_5 = (is_ie && (is_major == 4) && (agt.indexOf('msie 5.5') != -1));
        var is_ie5up = (is_ie && !is_ie3 && !is_ie4);
        var is_ie5_5up = (is_ie && !is_ie3 && !is_ie4 && !is_ie5);
        var is_ie6 = (is_ie && (is_major == 4) && (agt.indexOf('msie 6.') != -1));
        var is_ie6up = (is_ie && !is_ie3 && !is_ie4 && !is_ie5 && !is_ie5_5);

        //warn if incompatible browser is found
        if ( !is_nav6up && !is_ie6up )
        {
            var msg = "You are using a browser that is not \n" +
                    "recommended for use with this product. \n" +
                    "Please use either Netscape Navigator 6.2+, or \n" +
                    "Microsoft Internet Explorer 6+.  Thank you. ";
            alert( msg );
        }
    }
</script>
<title>SIPXchange User Interface</title>
</head>

<body class="bglight" onload="checkBrowser()">
<h1 class="list">Pingtel SIPxchange<SUP><FONT SIZE="-1">TM</FONT></SUP> User Interface</h1>
</body>
</html>
