<%@ page errorPage="/ui/error/error.jsp" %>
<%@ page import="java.io.IOException,
    javax.naming.InitialContext,
    javax.sql.DataSource,
    java.sql.Connection,
    java.sql.Statement,
    java.sql.ResultSet,
    java.sql.SQLException,
    java.util.*,
    java.lang.*"
%>
<%
    //get params from uri
    String searchString = request.getParameter( "searchstring" );
    String searchBy = request.getParameter( "searchby" );
    String searchType = request.getParameter( "cb1" );
    String display_id = null;
    String ugid = null;
    String id = null;
    String first = null;
    String last = null;
    String ext = null;
    String htmlHeader = "<html><head><title>search results</title>" +
        "<link rel=\"stylesheet\" type=\"text/css\" href=\"../../style/dms.css\">" +
        "<script language=\"javascript\">\n" +
        "function searchAgain()\n" +
        "{ document.location = \"/pds/ui/popup/user_search_popup.jsp\"; }\n" +
        "function closeWindow()\n" +
        "{ window.close(); }\n" +
        "function gotoUser(ugid,id)\n" +
        "{ self.opener.top.mainFrame.document.location = \"../user/list_all_users.jsp?groupid=\" + ugid + " +
            "\"&id=\" + id + \"" +
            "\";\n" +
            "window.close(); }\n" +
        "</script>" +
        "</head><body class=\"bglight\"><p class=\"formtitle\" align=\"center\">Results</p>";
    String htmlFooter = "<div align=\"center\">" +
        "<form>" +
        "<input type=\"button\" value=\"Search Again\" " +
        "onclick=\"searchAgain()\">&nbsp;&nbsp;" +
        "<input type=\"button\" value=\"Close\" " +
        "onclick=\"closeWindow()\">" +
        "</form></div>" +
        "</body></html>";
    String userTableHeader = "<table class=\"bglist\" width=\"300\" " +
        "cellpadding=\"3\" cellspacing=\"1\" align=\"center\">" +
        "<tr><th align=\"left\">User ID</th><th>Last Name</th>" +
        "<th>First Name</th><th>Ext</th></tr>";
    String pStatement = null;
    DataSource m_dataSource = null;
    Statement ps = null;
    Connection con = null;
    ResultSet rs = null;


    if ( searchString != null )
    {
        // if search type is not exact, add wild cards to searchString
        if( searchType == null )
        {
            pStatement = "select * from users where lower(" + searchBy + ")" +
                    " like \'%" + searchString + "%\' order by " + searchBy;
        }
        else
        {
            pStatement = "select * from users where " + searchBy +
                    " = \'" + searchString + "\' order by " + searchBy;
        }
        InitialContext initial = new InitialContext();
        m_dataSource = (DataSource) initial.lookup( "java:/PDSDataSource" );
        con = m_dataSource.getConnection();
        ps = con.createStatement( );
        rs = ps.executeQuery( pStatement );

        if( rs != null )
        {
            out.println( htmlHeader );
            out.println( userTableHeader );
            while( rs.next() )
            {
                display_id = rs.getString("display_id");
                id = rs.getString("id");
                first = rs.getString("first_name");
                last = rs.getString("last_name");
                ext = rs.getString("extension");
                ugid = rs.getString("ug_id");
                out.println("<tr><td>");
                out.println( "<a href=\"#\" onclick=\"gotoUser(" + ugid + "," + id + ")\">" +
                    display_id + "</a>" );
                out.println("</td>");
                out.println("<td>" + last + "</td>");
                out.println("<td>" + first + "</td>");
                out.println("<td>" + ext + "</td></tr>");
            }
            out.println( "</table>");
            out.println( htmlFooter );
        }
        rs.close();
        ps.close();
        con.close();
    }
%>