<%@ page import="javax.ejb.*, javax.naming.*,java.io.*,
                 javax.rmi.PortableRemoteObject,java.util.*,
                 com.pingtel.pds.pgs.organization.*,
                 com.pingtel.pds.common.PDSException,
                 java.rmi.RemoteException,
                 com.pingtel.pds.common.EJBHomeFactory"
%>


<%
    String orgName = request.getParameter( "orgname" ).trim();
    String password = request.getParameter( "password" ).trim();
    String dnsDomain = request.getParameter( "dnsdomain" ).trim();
    String stereotype = request.getParameter( "stereotype" ).trim();

    out.println("<br>" + orgName);
    out.println("<br>" + password);
    out.println("<br>" + dnsDomain);
    out.println("<br>" + stereotype + "<br>");

    // lookup home interface for organization bean
    try{
        OrganizationAdvocateHome organizationAdvocateHome =
                (OrganizationAdvocateHome)
                EJBHomeFactory.getInstance().getHomeInterface(
                        OrganizationAdvocateHome.class, "OrganizationAdvocate");

        OrganizationAdvocate orgAdvocate  = organizationAdvocateHome.create();

        Organization org =
                orgAdvocate.install(orgName, stereotype, dnsDomain, password);
    }
    catch(Exception ne ) {
        out.println("Exception: " + ne);
    }

    //Kill the session
    session.invalidate();
    // Redirect to login page
    response.sendRedirect( "../index.jsp" );
%>

