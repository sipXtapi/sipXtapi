<%@ page import="java.net.URLEncoder"%>
<HTML>
<HEAD><TITLE>MediaServer Authentication</TITLE>
<link rel="stylesheet" href="style/dms_sipxchange.css" type="text/css">
</HEAD>
<BODY class="bgdark">
<%
    if( session.getAttribute("mediaserverhost") != null )
    {
        String mediaserverHostName = (String) session.getAttribute("mediaserverhost");
        // Seperate out the protocol (ie http:// ) from the hostname.
        String mediaserverProtocol = "" ;
        if( mediaserverHostName.indexOf("://") != -1 )
        {
            mediaserverProtocol = mediaserverHostName.substring(0, mediaserverHostName.indexOf("://") + 3 ) ;
            mediaserverHostName = mediaserverHostName.substring( mediaserverHostName.indexOf("://") + 3 ) ;
        }
            
        String mediaserverURL=  mediaserverHostName + "/cgi-bin/voicemail/mediaserver.cgi?fromweb=yes";
        String redirectURL = "none";
        
        String context = request.getParameter("context" );
        if( "nav".equals(context) )
        { 
            mediaserverURL += "&action=getnav" ;
            
            // Check to see if this is the first time this page has been called.
            if( session.getAttribute("navIsAuthenticated") == null )
            {
                // Retrive username and password from session.
                if( session.getAttribute("user") != null && session.getAttribute("userpwd") != null )
                {
                    String username = session.getAttribute("user").toString();
                    String password = session.getAttribute("userpwd").toString();
                    if( !"".equals(username) && !"".equals(password) )
                    {
                        session.setAttribute("navIsAuthenticated", "yes");
                        // @JC Added URL Encoding to the username part so that it can
                        // contain an @ domain separator
                        redirectURL = mediaserverProtocol + 
                            URLEncoder.encode(username) + ":" + 
                            URLEncoder.encode(password) + "@" + 
                            mediaserverURL;
                    }
                }
            } else
            {
                redirectURL = mediaserverProtocol + mediaserverURL ;
            }
        } else 
        {
            mediaserverURL += "&action=login&from=web" ;
            // Check to see if this is the first time this page has been called.
            if( session.getAttribute("mainIsAuthenticated") == null )
            {
                // Retrive username and password from session.
                if( session.getAttribute("user") != null && session.getAttribute("userpwd") != null )
                {
                    String username = session.getAttribute("user").toString();
                    String password = session.getAttribute("userpwd").toString();
                    if( !"".equals(username) && !"".equals(password) )
                    {
                        session.setAttribute("mainIsAuthenticated", "yes");
                        // @JC Added URL Encoding to the username part so that it can
                        // contain an @ domain separator
                        redirectURL = mediaserverProtocol + 
                            URLEncoder.encode(username) + ":" + 
                            URLEncoder.encode(password) + "@" + 
                            mediaserverURL;
                    }
                }
            } else
            {
                redirectURL = mediaserverProtocol + mediaserverURL ;
            }
        }
            
        if( redirectURL != "none" )
        {
%>
            <SCRIPT LANGUAGE="Javascript">
                if ( document.layers )
                {   
                    // This is Netscape 4.x - it accepts '#' in the URL, but does not like the substitution of '%23' for #.
                    // Hence undo any encoding that you did.
                    url = unescape("<%= redirectURL %>");   
                    setTimeout('location.href="' + url + '"', 0);
                }
                else
                {
                    setTimeout('location.href="<%= redirectURL %>"', 0);
                }
            </SCRIPT>
<%      }
    }
%>

</BODY>
</HTML>

