<%@ page errorPage="/ui/error/error.jsp" %>
<%@ taglib uri="pgstags" prefix="pgs" %>

<pgs:createExtensionRange
        extensionpoolid='<%=request.getParameter("extensionpoolid")%>'
        minextension='<%=request.getParameter("minextension")%>'
        maxextension='<%=request.getParameter("maxextension")%>' />

<%
    response.sendRedirect(
            "/pds/ui/popup/add_extensions.jsp?extensionpoolid=" +
            request.getParameter("extensionpoolid"));
%>