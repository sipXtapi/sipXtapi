<%@ page errorPage="/ui/error/error.jsp"
            import="javax.naming.*,
                    com.pingtel.pds.pgs.phone.*,
                    com.pingtel.pds.common.*" %>

<%
    DeviceTypeAdvocateHome deviceTypeAdvocateHome
            = (DeviceTypeAdvocateHome)
                EJBHomeFactory.getInstance().getHomeInterface(  DeviceTypeAdvocateHome.class,
                                                                "DeviceTypeAdvocate" );
    DeviceTypeAdvocate deviceTypeAdvocate = deviceTypeAdvocateHome.create();

    deviceTypeAdvocate.importDeviceType( request.getParameter( "device_type_file" ) );
%>