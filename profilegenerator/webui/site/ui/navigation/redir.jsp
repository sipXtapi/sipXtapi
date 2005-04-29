<%@ page errorPage="/ui/error/error.jsp" %>
<%
    String type = request.getParameter("type");
    if ( type.equalsIgnoreCase( "user" ) )
    {
        if ( ( session.getAttribute("ugid") != null ) &&
            ( !( session.getAttribute("ugid").toString().equals("all") ) ) )
        {
            response.sendRedirect( "../user/list_all_users.jsp?groupid=" +
                                    session.getAttribute("ugid").toString() );
        }
        else
        {
            response.sendRedirect( "../user/list_all_users.jsp" );
        }
    }
    else
    {
       if ( (  session.getAttribute("dgid") != null ) &&
            ( !( session.getAttribute("dgid").toString().equals("all") ) ) )
        {
            response.sendRedirect( "../device/list_all_devices.jsp?groupid=" +
                                    session.getAttribute("dgid").toString() );
        }
        else
        {
            response.sendRedirect( "../device/list_all_devices.jsp" );
        }
    }
%>
