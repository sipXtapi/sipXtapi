
<%@ page isErrorPage="true" import="org.apache.log4j.*,
                                    com.pingtel.pds.common.PDSException,
                                    com.pingtel.pds.common.RedirectServletException" %>
<%! Category cat = Category.getInstance( "pgs" ); %>
<html>
<body bgcolor="#FFFFcc" text="#000000">
<h1>
Error Page
</h1>
<hr>
<h2>
Received the exception:<br>
<font color=red>

<%
    if ( exception != null && exception.getMessage() != null ) {
        NDC.push((String)session.getAttribute( "user" ));
        NDC.push((String)session.getAttribute( "orgname" ));
        cat.error( exception.getMessage() );
    }

    if ( exception instanceof RedirectServletException ) {
        pageContext.forward( ((RedirectServletException) exception).getSourceURL() );
    }
    else {
        out.println( exception.getMessage() );
    }
%>
</font>
</h2>
</body>
</html>