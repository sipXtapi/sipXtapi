<%@ page import="com.pingtel.pds.common.PDSDefinitions" %>

<%
  String strSType = session.getAttribute( "stereotype" ).toString();
  int intSType =  new Integer( strSType ).intValue();

  switch( intSType ){
  case PDSDefinitions.ORG_SERVICE_PROVIDER:
     response.sendRedirect( "provider_nav.html" );
     break;
  case PDSDefinitions.ORG_ENTERPRISE:
     response.sendRedirect( "enterprise_nav.html" );
     break;
  case PDSDefinitions.ORG_CUSTOMER:
     response.sendRedirect( "customer_nav.html" );
     break;
  default:
     out.println( "<h1>Error in Session Object</h1>" );
  }

%>