<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>
<%
    String pass = request.getParameter("password");
    if( pass.equals("") || pass == null )
    {
        pass = "";
    }
%>
<pgs:x_setUsersPassword userid='<%=request.getParameter("userid")%>'
	password='<%=pass%>'
    oldpassword='<%=request.getParameter("oldpassword")%>'
    parameters='<%="userid=" + request.getParameter("userid").trim() %>'  />

<pgs:generateCredentials/>


<%
    response.sendRedirect( "main.jsp?userid=" + request.getParameter( "userid" ) );
%>