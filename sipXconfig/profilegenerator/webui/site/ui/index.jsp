<%@ page    errorPage="/ui/error/error.jsp"
            import="javax.ejb.*, javax.naming.*,java.io.*,
                javax.rmi.PortableRemoteObject,java.util.*,
                com.pingtel.pds.pgs.organization.*,
                com.pingtel.pds.pgs.user.*,
                com.pingtel.pds.common.EJBHomeFactory,
                com.pingtel.pds.common.PDSException,
                com.pingtel.pds.common.PDSDefinitions,
                com.pingtel.pds.common.PathLocatorUtil"
%>

<%! private static final int INSTALL_ONLY_ORGID = 1000000; %>

<%
    try{
        OrganizationHome oHome = ( OrganizationHome ) EJBHomeFactory.getInstance().getHomeInterface(OrganizationHome.class, "Organization");
        //InitialContext initial = new InitialContext();
        //Object objref = initial.lookup( "Organization" );
//System.err.println("############ objref=" + objref.getClass().getName());        
        //OrganizationHome oHome = ( OrganizationHome ) PortableRemoteObject.narrow(
        //        objref, OrganizationHome.class);

        // check to see if organization install-only exist

        Organization org = oHome.findByPrimaryKey( new Integer(INSTALL_ONLY_ORGID));
        if ( org != null ) {
            response.sendRedirect("install/install.jsp" );
        }
    }
    catch( NamingException ne ) {
        //@todo fix this up!
        out.println("NamingExecption: " + ne);
    }
    catch( ObjectNotFoundException onfe ) {
        response.sendRedirect("initialize/initialize.jsp" );
    }

%>

