var protocol = document.location.protocol;
var host = document.location.host;

function add_alias( id, replace, oldID, dnsdomain )
{
    var aliasID = id;
    var alias = mainFrame.document.createElement("div");
    alias.setAttribute("id", aliasID);

    if ( replace == "false" )
    {
        mainFrame.document.getElementById("insert_alias").appendChild(alias);
    }
    else
    {
        mainFrame.document.getElementById("insert_alias").replaceChild(
                                                                         alias, mainFrame.document.getElementById( oldID ));
    }
    alias.innerHTML =  "<table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                            "<tr>" +
                            "<th colspan='3'><img src='../../image/1x1green.gif'>" +
                            "</th>" +
                            "<th align='right'>" +
                            "<a href='#'><img onclick=\"parent.edit_alias('" + aliasID + "','" + dnsdomain + "')\"" +
                            "src='../../image/editicon.gif' alt='edit' width='14' height='14' alt='' border='0'></a>" +
                            "<img src='../../image/1x1green.gif' width='10'/>" +
                            "<a href='#'><img src='../../image/del.gif'" +
                            "onclick=\"parent.del_alias('" + aliasID + "')\" alt='delete' width='14' height='14' border='0'></a>" +
                            "</th>" +
                            "</tr>" +
                            "</table>" +
                            "<table class='bglist' width='600' cellpaddding='2' cellspacing='1' border='0'>" +
                            "<tr>" +
                            "<td colspan='3'>Alias" +
                            "</td>" +
                            "<td class='readonly' width='259'>" +
                            "sip:" + aliasID + "@" + dnsdomain  +
                            "</td></tr>" +
                            "</table>" +
                            "<input type=\"hidden\" id=\"alias." + aliasID + "\" value=\"" + aliasID + "\" >";
    rebuildAliasString();
    setSaveFlag();


}

function add_speeddial( sdid,label,type,address,rpid,replace,oldID )
{
    var date = new Date();
    var id = date.getTime();
    var displayAddress = address.replace(/</,"&lt;");
    displayAddress = displayAddress.replace(/>/,"&gt;");
    var speeddial = mainFrame.document.createElement("div");
    speeddial.setAttribute("id", id);

    if ( replace == "false" )
    {
        mainFrame.document.getElementById("insert_speeddial").appendChild(speeddial);
    }
    else
    {
        mainFrame.document.getElementById("insert_speeddial").replaceChild(
                                                                          speeddial, mainFrame.document.getElementById( oldID ));
    }

    speeddial.innerHTML = "<table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                          "<tr>" +
                          "<th colspan='3'><img src='../../image/1x1green.gif'>" +
                          "</th>" +
                          "<th align='right'>" +
                          //"<a href='#'><img onclick='parent.edit_speeddial(\"" + id + "\",\"" + rpid + "\")'" +
                          "<a href='#'><img onclick='parent.edit_speeddial(" + id + "," + rpid + ")'" +
                          "src='../../image/editicon.gif' alt='edit' width='14' height='14' alt='' border='0'></a>" +
                          "<img src='../../image/1x1green.gif' width='10'/>" +
                          "<a href='#'><img src='../../image/del.gif'" +
                          "onclick='parent.del_speeddial(" + id + ")' alt='delete' width='14' height='14' alt='' border='0'></a>" +
                          "</th>" +
                          "</tr>" +
                          "</table>" +
                          "<table class='bglist' width='600' cellpaddding='2' cellspacing='1' border='0'>" +
                          "<tr>" +
                          "<td colspan='3'>Speed dial number" +
                          "</td>" +
                          "<td class='readonly' width='259'>" +
                          sdid +
                          "</td></tr>" +
                          "<tr>" +
                          "<td colspan='3'>Name" +
                          "</td>" +
                          "<td class='readonly' width='259'>" +
                          label +
                          "</td></tr>"  +
                          "<tr>" +
                          "<td colspan='3'>Type" +
                          "</td>" +
                          "<td class='readonly' width='259'>" +
                          type +
                          "</td></tr>"  +
                          "<tr>" +
                          "<td colspan='3'>Number/URL" +
                          "</td>" +
                          "<td class='readonly' width='259'>" +
                          displayAddress +
                          "</td></tr></table>" +
                          "<input type=\"hidden\" value=\"" + sdid + "\" name=\"" +
                          id + ".PHONESET_SPEEDDIAL." + rpid + ".PHONESET_SPEEDDIAL~EMPTY.ID.i_true.\" id=\"speeddial_sdid\">" +
                          "<input type=\"hidden\" value=\"" + label + "\" name=\"" +
                          id + ".PHONESET_SPEEDDIAL." + rpid + ".PHONESET_SPEEDDIAL~EMPTY.LABEL.\" id=\"speeddial_label\">" +
                          "<input type=\"hidden\" value=\"" + type + "\" name=\"" +
                          id + ".PHONESET_SPEEDDIAL." + rpid + ".PHONESET_SPEEDDIAL~EMPTY.TYPE.\" id=\"speeddial_type\">" +
                          "<input type=\"hidden\" value=\"" + address + "\" name=\"" +
                          id + ".PHONESET_SPEEDDIAL." + rpid + ".PHONESET_SPEEDDIAL~EMPTY.ADDRESS.\" id=\"speeddial_address\">";
    setSaveFlag();
}

function add_digitmap( number,target,rpid,replace,oldID )
{
    var date = new Date();
    var id = date.getTime();
    var displayTarget = target.replace(/</,"&lt;");
    displayTarget = displayTarget.replace(/>/,"&gt;");
    var digitmap = mainFrame.document.createElement("div");
    digitmap.setAttribute("id", id);

    if ( replace == "false" )
    {
        mainFrame.document.getElementById("insert_digitmap").appendChild(digitmap);
    }
    else
    {
        mainFrame.document.getElementById("insert_digitmap").replaceChild(
                                                                         digitmap, mainFrame.document.getElementById( oldID ));
    }

    digitmap.innerHTML = "<table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                         "<tr>" +
                         "<th colspan='3'><img src='../../image/1x1green.gif'>" +
                         "</th>" +
                         "<th align='right'>" +
                         "<a href='#'><img onclick='parent.edit_digitmap(" + id + "," + rpid + ")'" +
                         "src='../../image/editicon.gif' alt='edit' width='14' height='14' alt='' border='0'></a>" +
                         "<img src='../../image/1x1green.gif' width='10'/>" +
                         "<a href='#'><img src='../../image/del.gif'" +
                         "onclick='parent.del_digitmap(" + id + ")' alt='delete' width='14' height='14' alt='' border='0'></a>" +
                         "</th>" +
                         "</tr>" +
                         "</table>" +
                         "<table class='bglist' width='600' cellpaddding='2' cellspacing='1' border='0'>" +
                         "<tr>" +
                         "<td colspan='3'>Dial plan" +
                         "</td>" +
                         "<td class='readonly' width='259'>" +
                         number +
                         "</td></tr>" +
                         "<tr>" +
                         "<td colspan='3'>SIP Address" +
                         "</td>" +
                         "<td class='readonly' width='259'>" +
                         displayTarget +
                         "</td></tr></table>" +
                         "<input type=\"hidden\" value=\"" + number + "\" name=\"" +
                         id + ".PHONESET_DIGITMAP." + rpid + ".PHONESET_DIGITMAP~EMPTY.NUMBER.i_true.\" id=\"digitmap_number\">" +
                         "<input type=\"hidden\" value=\"" + target + "\" name=\"" +
                         id + ".PHONESET_DIGITMAP." + rpid + ".PHONESET_DIGITMAP~EMPTY.TARGET.\" id=\"digitmap_target\">";
    setSaveFlag();
}

function add_authuser( userid,passtoken,realm,rpid,replace,oldID )
{
    var date = new Date();
    var id = date.getTime();
    var authuser = mainFrame.document.createElement("div");
    authuser.setAttribute("id", id);

    if ( replace == "false" )
    {
        mainFrame.document.getElementById("insert_authuser").appendChild(authuser);
    }
    else
    {
        mainFrame.document.getElementById("insert_authuser").replaceChild(
                                                                         authuser, mainFrame.document.getElementById( oldID ));
    }

    authuser.innerHTML = "<table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                         "<tr>" +
                         "<th colspan='3'><img src='../../image/1x1green.gif'>" +
                         "</th>" +
                         "<th align='right'>" +
                         "<a href='#'><img onclick='parent.edit_authuser(" + id + "," + rpid + ")'" +
                         "src='../../image/editicon.gif' alt='edit' width='14' height='14' alt='' border='0'></a>" +
                         "<img src='../../image/1x1green.gif' width='10'/>" +
                         "<a href='#'><img src='../../image/del.gif'" +
                         "onclick='parent.del_authuser(" + id + ")' alt='delete' width='14' height='14' alt='' border='0'></a>" +
                         "</th>" +
                         "</tr>" +
                         "</table>" +
                         "<table class='bglist' width='600' cellpaddding='2' cellspacing='1' border='0'>" +
                         "<tr>" +
                         "<td colspan='3'>User Name" +
                         "</td>" +
                         "<td class='readonly' width='259'>" +
                         userid +
                         "</td></tr></table>" +
                         "<input type=\"hidden\" value=\"" + userid + "\" name=\"" +
                         id + ".PHONESET_HTTP_AUTH_DB." + rpid + ".PHONESET_HTTP_AUTH_DB~EMPTY.USERID.i_true.\" id=\"authuser_userid\">" +
                         "<input type=\"hidden\" value=\"" + realm + "\" name=\"" +
                         id + ".PHONESET_HTTP_AUTH_DB." + rpid + ".PHONESET_HTTP_AUTH_DB~EMPTY.REALM.\" id=\"authuser_realm\">" +
                         "<input type=\"hidden\" name=\"" +
                         id + ".PHONESET_HTTP_AUTH_DB." + rpid + ".PHONESET_HTTP_AUTH_DB~EMPTY.PASSTOKEN.\"" +
                         " value=\"" + passtoken + "\" id=\"authuser_passtoken\">";
    setSaveFlag();
}


function add_authcaller( user,password,rpid,replace,oldID )
{
    var date = new Date();
    var id = date.getTime();
    var authcaller = mainFrame.document.createElement("div");
    authcaller.setAttribute("id", id);

    if ( replace == "false" )
    {
        mainFrame.document.getElementById("insert_authcaller").appendChild(authcaller);
    }
    else
    {
        mainFrame.document.getElementById("insert_authcaller").replaceChild(
                                                                         authcaller, mainFrame.document.getElementById( oldID ));
    }

    authcaller.innerHTML = "<table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                         "<tr>" +
                         "<th colspan='3'><img src='../../image/1x1green.gif'>" +
                         "</th>" +
                         "<th align='right'>" +
                         "<a href='#'><img onclick='parent.edit_authcaller(" + id + "," + rpid + ")'" +
                         "src='../../image/editicon.gif' alt='edit' width='14' height='14' alt='' border='0'></a>" +
                         "<img src='../../image/1x1green.gif' width='10'/>" +
                         "<a href='#'><img src='../../image/del.gif'" +
                         "onclick='parent.del_authcaller(" + id + ")' alt='delete' width='14' height='14' alt='' border='0'></a>" +
                         "</th>" +
                         "</tr>" +
                         "</table>" +
                         "<table class='bglist' width='600' cellpaddding='2' cellspacing='1' border='0'>" +
                         "<tr>" +
                         "<td colspan='3'>User Name" +
                         "</td>" +
                         "<td class='readonly' width='259'>" +
                         user +
                         "</td></tr></table>" +
                         "<input type=\"hidden\" value=\"" + user + "\" name=\"" +
                         id + ".SIP_AUTHENTICATE_DB." + rpid + ".SIP_AUTHENTICATE_DB~EMPTY.USER.i_true.\" id=\"authcaller_user\">" +
                         "<input type=\"hidden\" value=\"" + password + "\" name=\"" +
                         id + ".SIP_AUTHENTICATE_DB." + rpid + ".SIP_AUTHENTICATE_DB~EMPTY.PASSWORD.\" id=\"authcaller_password\">"
    setSaveFlag();
}

function add_plcredential( userid,passtoken,realm,rpid,replace,oldID  )
{
    var date = new Date();
    var id = date.getTime();
    var plcredential = mainFrame.document.createElement("div");
    plcredential.setAttribute("id", id);

    if ( replace == "false" )
    {
        mainFrame.document.getElementById("insert_plcredential").appendChild(plcredential);
    }
    else
    {
        mainFrame.document.getElementById("insert_plcredential").replaceChild(
                                                           plcredential, mainFrame.document.getElementById( oldID ));
    }

    plcredential.innerHTML = "<table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                           "<tr>" +
                           "<th colspan='3'><img src='../../image/1x1green.gif'>" +
                           "</th>" +
                           "<th align='right'>" +
                           "<a href='#'><img onclick='parent.edit_plcredential(" + id + "," + rpid + ")'" +
                           "src='../../image/editicon.gif' width='14' height='14' alt='' border='0'></a>" +
                           "<img src='../../image/1x1green.gif' width='10'/>" +
                           "<a href='#'><img src='../../image/del.gif'" +
                           "onclick='parent.del_plcredential(" + id + ")' width='14' height='14' alt='' border='0'></a>" +
                           "</th>" +
                           "</tr>" +
                           "</table>" +
                           "<table class='bglist' width='600' cellpaddding='2' cellspacing='1' border='0'>" +
                           "<tr>" +
                           "<td width='21' class='cell_fill_dark'><img src='../../image/1x1green.gif'>" +
                           "</td>" +
                           "<td colspan='2'>User ID</td>" +
                           "<td width='257' class='readonly'>" +
                           userid +
                           "</td>" +
                           "</tr>" +
                           "<tr>" +
                           "<td class='cell_fill_dark'><img src='../../image/1x1green.gif'>" +
                           "</td>" +
                           "<td colspan='2'>Realm" +
                           "</td>" +
                           "<td width='257' class='readonly'>" +
                           realm +
                           "</td>" +
                           "</tr>" +
                           "</table>" +
                           "<input type=\"hidden\" value=\"" + userid + "\" name=\"" +
                         "null.PRIMARY_LINE." + rpid + ".CREDENTIAL~PRIMARY_LINE~EMPTY.USERID." + id + "\" id=\"plc_userid\">" +
                         "<input type=\"hidden\" value=\"" + realm + "\" name=\"" +
                         "null.PRIMARY_LINE." + rpid + ".CREDENTIAL~PRIMARY_LINE~EMPTY.REALM." + id + "\" id=\"plc_realm\">" +
                         "<input type=\"hidden\"  value=\"" + passtoken + "\" name=\"" +
                         "null.PRIMARY_LINE." + rpid + ".CREDENTIAL~PRIMARY_LINE~EMPTY.PASSTOKEN." + id + "\" id=\"plc_passtoken\">"

        setSaveFlag();
}

function add_userline( url,registration,allow_forwarding,rpid  )
{
    var date = new Date();
    var id = date.getTime();
    var contid = "cont" + id;
    var hiddenURL = url;
    while ( hiddenURL.indexOf('"') != -1 ) {
      hiddenURL = hiddenURL.replace(/"/,"&quot;");
    }
    var displayURL = url.replace(/</,"&lt;");
    displayURL = displayURL.replace(/>/,"&gt;");
    while ( displayURL.indexOf('"') != -1 ) {
      displayURL = displayURL.replace(/"/,"&quot;");
    }
    var userline = mainFrame.document.createElement("div");
    userline.setAttribute("id", contid);
    var insert_ulcredential = "insert_ulcredential" + id;

    mainFrame.document.getElementById("insert_userline").appendChild(userline);

    userline.innerHTML =  "<div id='" + id + "'>" +
                       "<hr class='seperator' width='600' align='left'>" +
                       "<table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                       "<tr>" +
                       "<th colspan='3' align='left'>User Line" +
                       "</th>" +
                       "<th align='right'>" +
                       "<a href='#'><img onclick=\"parent.edit_userline('" + id + "','" +
                       rpid + "','" + contid + "')\"" +
                       "src='../../image/editicon.gif' width='14' height='14' alt='' border='0'></a>" +
                       "<img src='../../image/1x1green.gif' width='10'/>" +
                       "<a href='#'><img src='../../image/del.gif'" +
                       "onclick=\"parent.del_userline('" + contid + "')\" width='14' height='14' alt='' border='0'></a>" +
                       "</th>" +
                       "</tr>" +
                       "</table>" +
                       "<table class='bglist' width='600' cellpaddding='2' cellspacing='1' border='0'>" +
                       "<tr>" +
                       "<td colspan='3'>SIP URL" +
                       "</td>" +
                       "<td width='257' class='readonly'>" +
                       displayURL +
                       "</td>" +
                       "</tr>" +
                       "<tr>" +
                       "<td colspan='3'>Register with host or provision line as is</td>" +
                       "<td width='257' class='readonly'>" +
                       registration +
                       "</td>" +
                       "</tr>" +
                       "<tr>" +
                       "<td colspan='3'>Apply rules for call forwarding</td>" +
                       "<td width='257' class='readonly'>" +
                       allow_forwarding +
                       "</td>" +
                       "</tr>" +
                       "</table>" +
                       "<input type=\"hidden\" value=\"" + hiddenURL + "\" name=\"" +
                            id + ".USER_LINE." + rpid + ".USER_LINE~EMPTY.URL.i_true.\" id=\"uline_url\">" +
                       "<input type=\"hidden\" value=\"" + registration + "\" name=\"" +
                            id + ".USER_LINE." + rpid + ".USER_LINE~EMPTY.REGISTRATION.\" id=\"uline_registration\">" +
                       "<input type=\"hidden\"  value=\"" + allow_forwarding + "\" name=\"" +
                            id + ".USER_LINE." + rpid + ".USER_LINE~EMPTY.ALLOW_FORWARDING.\" id=\"uline_allow_forwarding\">" +
                       "</div>" +
                       "<div id='" + insert_ulcredential + "'>" +
                       "<table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                       "<tr>" +
                       "<th colspan='3'>Credentials</th>" +
                       "<th width='58' align='right'>" +
                       "<a href='#'>" +
                       "<img onclick=\"parent.openBrWindow('/pds/ui/popup/add_ulcredential.html?ulid=" + id +
                       "&rpid=" + rpid + "','popup','scrollbars=yes,width=400,height=300')\"" +
                       "src='../../image/add.gif' alt='add' border='0'></a>" +
                       "</th></tr></table></div>";


        setSaveFlag();
}

function replace_userline( url,registration,allow_forwarding,rpid,oldid,cid  )
{
    var date = new Date();
    var id = date.getTime();
    var hiddenURL = url;
    while ( hiddenURL.indexOf('"') != -1 ) {
      hiddenURL = hiddenURL.replace(/"/,"&quot;");
    }
    var displayURL = url.replace(/</,"&lt;");
    displayURL = displayURL.replace(/>/,"&gt;");
    while ( displayURL.indexOf('"') != -1 ) {
      displayURL = displayURL.replace(/"/,"&quot;");
    }
    var userline = mainFrame.document.createElement("div");
    userline.setAttribute("id", id);

    mainFrame.document.getElementById(cid).replaceChild(
        userline, mainFrame.document.getElementById( oldid ));

    userline.innerHTML =  "<table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                       "<tr>" +
                       "<th colspan='3' align='left'>User Line" +
                       "</th>" +
                       "<th align='right'>" +
                       "<a href='#'><img onclick=\"parent.edit_userline('" + id + "','" +
                       rpid + "','" + cid + "')\"" +
                       "src='../../image/editicon.gif' width='14' height='14' alt='' border='0'></a>" +
                       "<img src='../../image/1x1green.gif' width='10'/>" +
                       "<a href='#'><img src='../../image/del.gif'" +
                       "onclick=\"parent.del_userline('" + cid + "')\" width='14' height='14' alt='' border='0'></a>" +
                       "</th>" +
                       "</tr>" +
                       "</table>" +
                       "<table class='bglist' width='600' cellpaddding='2' cellspacing='1' border='0'>" +
                       "<tr>" +
                       "<td colspan='3'>SIP URL" +
                       "</td>" +
                       "<td width='257' class='readonly'>" +
                       displayURL +
                       "</td>" +
                       "</tr>" +
                       "<tr>" +
                       "<td colspan='3'>Register with host or provision line as is</td>" +
                       "<td width='257' class='readonly'>" +
                       registration +
                       "</td>" +
                       "</tr>" +
                       "<tr>" +
                       "<td colspan='3'>Apply rules for call forwarding</td>" +
                       "<td width='257' class='readonly'>" +
                       allow_forwarding +
                       "</td>" +
                       "</tr>" +
                       "</table>" +
                       "<input type=\"hidden\" value=\"" + hiddenURL + "\" name=\"" +
                            oldid + ".USER_LINE." + rpid + ".USER_LINE~EMPTY.URL.i_true.\" id=\"uline_url\">" +
                       "<input type=\"hidden\" value=\"" + registration + "\" name=\"" +
                            oldid + ".USER_LINE." + rpid + ".USER_LINE~EMPTY.REGISTRATION.\" id=\"uline_registration\">" +
                       "<input type=\"hidden\"  value=\"" + allow_forwarding + "\" name=\"" +
                            oldid + ".USER_LINE." + rpid + ".USER_LINE~EMPTY.ALLOW_FORWARDING.\" id=\"uline_allow_forwarding\">" +
                       "</div>";


        setSaveFlag();
}

function add_ulcredential( userid,passtoken,realm,rpid,ulid,replace,oldID  )
{
    var date = new Date();
    var id = date.getTime();
    var ulcredential = mainFrame.document.createElement("div");
    ulcredential.setAttribute("id", id);
    var insert_ulcredential = "insert_ulcredential" + ulid;

    if ( replace == "false" )
    {
        mainFrame.document.getElementById(insert_ulcredential).appendChild(ulcredential);
    }
    else
    {
        var parent = mainFrame.document.getElementById(insert_ulcredential);
        var children = parent.childNodes;
        for( i = 0; i < children.length; i++ )
        {
            if( children[i].id == oldID )
            {
                parent.replaceChild( ulcredential, children[i] );
            }
        }
    }

    ulcredential.innerHTML = "<table class='bglist' width='600' cellpaddding='2' cellspacing='0' border='0'>" +
                           "<tr>" +
                           "<th colspan='3'><img src='../../image/1x1green.gif'>" +
                           "</th>" +
                           "<th align='right'>" +
                           "<a href='#'><img onclick=\"parent.edit_ulcredential('" + ulid + "','" + id + "','" + rpid + "')\"" +
                           "src='../../image/editicon.gif' width='14' height='14' alt='' border='0'></a>" +
                           "<img src='../../image/1x1green.gif' width='10'/>" +
                           "<a href='#'><img src='../../image/del.gif'" +
                           "onclick=\"parent.del_ulcredential('" + ulid + "','" + id + "')\" width='14' height='14' alt='' border='0'></a>" +
                           "</th>" +
                           "</tr>" +
                           "</table>" +
                           "<table class='bglist' width='600' cellpaddding='2' cellspacing='1' border='0'>" +
                           "<tr>" +
                           "<td width='21' class='cell_fill_dark'><img src='../../image/1x1green.gif'>" +
                           "</td>" +
                           "<td colspan='2'>User ID</td>" +
                           "<td width='257' class='readonly'>" +
                           userid +
                           "</td>" +
                           "</tr>" +
                           "<tr>" +
                           "<td class='cell_fill_dark'><img src='../../image/1x1green.gif'>" +
                           "</td>" +
                           "<td colspan='2'>Realm" +
                           "</td>" +
                           "<td width='257' class='readonly'>" +
                           realm +
                           "</td>" +
                           "</tr>" +
                           "</table>" +
                           "<input type=\"hidden\" value=\"" + userid + "\" name=\"" +
                         ulid + ".USER_LINE." + rpid + ".CREDENTIAL~USER_LINE~EMPTY.USERID." + id + "\" id=\"ulc_userid\">" +
                         "<input type=\"hidden\" value=\"" + realm + "\" name=\"" +
                         ulid + ".USER_LINE." + rpid + ".CREDENTIAL~USER_LINE~EMPTY.REALM." + id + "\" id=\"ulc_realm\">" +
                         "<input type=\"hidden\"  value=\"" + passtoken + "\" name=\"" +
                         ulid + ".USER_LINE." + rpid + ".CREDENTIAL~USER_LINE~EMPTY.PASSTOKEN." + id + "\" id=\"ulc_passtoken\">"

        setSaveFlag();
}

function replace_user(displayid,firstname,lastname,password)
{
    var pass = password;
    if ( pass == "" )
    {
        pass = "e_m_p_t_y";
    }
    var user = mainFrame.document.getElementById("edit_user");
    user.innerHTML = "<table class='bglist' width='600' cellpaddding='4' cellspacing='1' border='0'>" +
                     "<tr>" +
                     "<td width='100'>User ID" +
                     "</td>" +
                     "<td colspan='2' class='readonly'>" +
                     displayid +
                     "</td></tr>" +
                     "<tr>" +
                     "<td width='100'>First Name</td>" +
                     "<td colspan='2' class='readonly'>" +
                     firstname +
                     "&nbsp;</td></tr>" +
                     "<tr>" +
                     "<td width='100'>Last Name</td>" +
                     "<td colspan='2' class='readonly'>" +
                     lastname +
                     "&nbsp;</td></tr></table>" +
                     "<input type='hidden' name='general.DisplayID' value='" +
                     displayid + "' id='user_displayid'/>" +
                     "<input type=\"hidden\" name=\"general.FirstName\" value=\"" +
                     firstname + "\" id=\"user_firstname\"/>" +
                     "<input type=\"hidden\" name=\"general.LastName\" value=\"" +
                     lastname + "\" id=\"user_lastname\"/>" +
                     "<input type='hidden' name='general.Password' value='" +
                     pass + "' id='user_password'/>";
    setSaveFlag();

}

function del_alias(idToRemove)
{
    //var idToRemove = id;
    var msg = "Are you sure you want to delete\n" +
            "this alias?";
    var userInput = confirm( msg );
    if ( userInput == true )
    {
        var parent = mainFrame.document.getElementById( "insert_alias" );
        var child = mainFrame.document.getElementById( idToRemove );
        parent.removeChild(child);
        rebuildAliasString();
        setSaveFlag();
    }
}


function del_speeddial(id)
{
    var msg = "Are you sure you want to delete\n" +
    "this speed dial number?";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        mainFrame.saveFlag = true;
        var parent = mainFrame.document.getElementById("insert_speeddial");
        var divCount = 0;
        for ( var i = 0; i < parent.childNodes.length; i++ )
        {
            if( parent.childNodes[i].nodeName == "DIV" )
            {
                divCount = divCount + 1;
            }
        }
        if ( divCount != 1 )
        {
            var child = mainFrame.document.getElementById(id);
            parent.removeChild(child);
        }
        else
        {
            mainFrame.document.getElementById("speeddial_sdid").value = "";
            mainFrame.document.getElementById("speeddial_label").value = "";
            mainFrame.document.getElementById("speeddial_type").value = "";
            mainFrame.document.getElementById("speeddial_address").value = "";
            var contParent = mainFrame.document.getElementById(id);
            if (navigator.userAgent.toLowerCase().indexOf("mozilla") != -1)
            {
                for (var i = 0; i < contParent.childNodes.length; i++)
                {
                    if (contParent.childNodes[i].id == "display")
                    {
                        contParent.childNodes[i].style.display = "none";
                    }
                }
            }
            else
            {
                mainFrame.document.getElementById(id).style.display = "none";
            }
        }
        setSaveFlag();
    }
}


function del_digitmap(id)
{
    var msg = "Are you sure you want to delete\n" +
              "this digitmap?";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        var parent = mainFrame.document.getElementById("insert_digitmap");
        var divCount = 0;
        for ( var i = 0; i < parent.childNodes.length; i++ )
        {
            if( parent.childNodes[i].nodeName == "DIV" )
            {
                divCount = divCount + 1;
            }
        }
        if ( divCount != 1 )
        {
            var child = mainFrame.document.getElementById(id);
            parent.removeChild(child);
        }
        else
        {
            mainFrame.document.getElementById("digitmap_number").value = "";
            mainFrame.document.getElementById("digitmap_target").value = "";
            mainFrame.document.getElementById(id).style.display = "none";
        }
        setSaveFlag();
    }
}

function del_authuser(id)
{
    var msg = "Are you sure you want to delete\n" +
              "this authorized user?";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        var parent = mainFrame.document.getElementById("insert_authuser");
        var divCount = 0;
        for ( var i = 0; i < parent.childNodes.length; i++ )
        {
            if( parent.childNodes[i].nodeName == "DIV" )
            {
                divCount = divCount + 1;
            }
        }
        if ( divCount != 1 )
        {
            var child = mainFrame.document.getElementById(id);
            parent.removeChild(child);
        }
        else
        {
            mainFrame.document.getElementById("authuser_userid").value = "";
            mainFrame.document.getElementById("authuser_realm").value = "";
            mainFrame.document.getElementById("authuser_passtoken").value = "";
            mainFrame.document.getElementById(id).style.display = "none";
        }
        setSaveFlag();
    }
}

function del_authcaller(id)
{
    var msg = "Are you sure you want to delete\n" +
              "this authorized caller?";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        var parent = mainFrame.document.getElementById("insert_authcaller");
        var divCount = 0;
        for ( var i = 0; i < parent.childNodes.length; i++ )
        {
            if( parent.childNodes[i].nodeName == "DIV" )
            {
                divCount = divCount + 1;
            }
        }
        if ( divCount != 1 )
        {
            var child = mainFrame.document.getElementById(id);
            parent.removeChild(child);
        }
        else
        {
            mainFrame.document.getElementById("authcaller_user").value = "";
            mainFrame.document.getElementById("authcaller_password").value = "";
            mainFrame.document.getElementById(id).style.display = "none";
        }
        setSaveFlag();
    }
}

function del_plcredential(id)
{
    var msg = "Are you sure you want to delete\n" +
              "this primary line credential?";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        var parent = mainFrame.document.getElementById("insert_plcredential");
        var child = mainFrame.document.getElementById(id);
        parent.removeChild(child);
        setSaveFlag();
    }
}

/*
function del_userline(id)
{
    var msg = "Are you sure you want to delete\n" +
              "this user line?";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        var parent = mainFrame.document.getElementById("insert_userline");
        var child = mainFrame.document.getElementById(id);
        parent.removeChild(child);
        setSaveFlag();
    }
}

*/
function del_userline(id)
{
    var msg = "Are you sure you want to delete\n" +
              "this user line?";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        var parent = mainFrame.document.getElementById("insert_userline");
        var divCount = 0;
        for ( var i = 0; i < parent.childNodes.length; i++ )
        {
            if( parent.childNodes[i].nodeName == "DIV" )
            {
                divCount = divCount + 1;
            }
        }
        if ( divCount != 1 )
        {
            var child = mainFrame.document.getElementById(id);
            parent.removeChild(child);
        }
        else
        {
            mainFrame.document.getElementById("uline_url").value = "";
            mainFrame.document.getElementById("uline_registration").value = "";
            mainFrame.document.getElementById("uline_allow_forwarding").value = "";
            if( mainFrame.document.getElementById("ulc_userid") != null ) {
                var trimID = id.slice(4);
                var ulcID = "insert_ulcredential" + trimID;
                tempParent = mainFrame.document.getElementById(id);
                tempChild = mainFrame.document.getElementById(ulcID);
                tempParent.removeChild(tempChild);
            }
            mainFrame.document.getElementById(id).style.display = "none";
        }
        setSaveFlag();
    }
}

function del_ulcredential(ulid,id)
{
    var msg = "Are you sure you want to delete\n" +
              "this user line credential?";
    var userInput =  confirm ( msg );
    if ( userInput == true )
    {
        var insert_ulcredential = "insert_ulcredential" + ulid;
        var parent = mainFrame.document.getElementById(insert_ulcredential);
        var children = parent.childNodes;
        for( i = 0; i < children.length; i++ )
        {
            if( children[i].id == id )
            {
                var child = children[i];
                parent.removeChild(child);
                setSaveFlag();
            }
        }
    }

}

function edit_alias( id, dnsdomain )
{
   var theURL = protocol + "//" + host + "/pds/ui/popup/edit_alias.jsp?id=" + id + "&dnsdomain=" + dnsdomain;
    var winName = "edit";
    var features = "scrollbars=yes,width=600,height=200";
    top.MM_openBrWindow( theURL, winName, features );
    //var newWindow = window.open( theURL, winName, features);
    //newWindow.focus();
}

function edit_speeddial(id,rpid)
{
    var theURL = protocol + "//" + host + "/pds/ui/popup/edit_speeddial.html?id=" + id + "&rpid=" + rpid;
    var winName = "edit";
    var features = "scrollbars=yes,width=400,height=300";
    top.MM_openBrWindow( theURL, winName, features );
    //var newWindow = window.open( theURL, winName, features);
   // newWindow.focus();
}

function edit_digitmap(id,rpid)
{
    var theURL = protocol + "//" + host + "/pds/ui/popup/edit_digitmap.html?id=" + id + "&rpid=" + rpid;
    var winName = "edit";
    var features = "scrollbars=yes,width=400,height=300";
    top.MM_openBrWindow( theURL, winName, features );
    //var newWindow = window.open(theURL, winName, features);
    //newWindow.focus();
}

function edit_authuser(id,rpid)
{
    var theURL = protocol + "//" + host + "/pds/ui/popup/edit_authuser.html?id=" + id + "&rpid=" + rpid;
    var winName = "edit";
    var features = "scrollbars=yes,width=400,height=300";
    top.MM_openBrWindow( theURL, winName, features );
    //var newWindow = window.open(theURL, winName, features);
    //newWindow.focus();
}

function edit_authcaller(id,rpid)
{
    var theURL = protocol + "//" + host + "/pds/ui/popup/edit_authcaller.html?id=" + id + "&rpid=" + rpid;
    var winName = "edit";
    var features = "scrollbars=yes,width=400,height=300";
    top.MM_openBrWindow( theURL, winName, features );
    //var newWindow = window.open(theURL, winName, features);
    //newWindow.focus();
}

function edit_plcredential(id,rpid)
{
    var theURL = protocol + "//" + host + "/pds/ui/popup/edit_plcredential.html?id=" + id + "&rpid=" + rpid;
    var winName = "edit";
    var features = "scrollbars=yes,width=400,height=300";
    top.MM_openBrWindow( theURL, winName, features );
    //var newWindow = window.open(theURL, winName, features);
    //newWindow.focus();
}

function edit_userline(id,rpid,contid)
{
    var theURL = protocol + "//" + host + "/pds/ui/popup/edit_userline.html?id=" + id + "&rpid=" + rpid + "&contid=" + contid;
    var winName = "edit";
    var features = "scrollbars=yes,width=460,height=300";
    top.MM_openBrWindow( theURL, winName, features );
    //var newWindow = window.open(theURL, winName, features);
    //newWindow.focus();
}

function edit_ulcredential(ulid,id,rpid)
{
    var theURL = protocol + "//" + host + "/pds/ui/popup/edit_ulcredential.html?ulid=" + ulid + "&id=" + id + "&rpid=" + rpid;
    var winName = "edit";
    var features = "scrollbars=yes,width=400,height=300";
    top.MM_openBrWindow( theURL, winName, features );
    //var newWindow = window.open(theURL, winName, features);
    //newWindow.focus();
}

function edit_user()
{
    var theURL = protocol + "//" + host + "/pds/ui/popup/edit_user.html";
    var winName = "edit";
    var features = "scrollbars=yes,width=400,height=300";
    top.MM_openBrWindow( theURL, winName, features );
    //var newWindow = window.open(theURL, winName, features);
    //newWindow.focus();
}

function rebuildAliasString()
{
        // get parent node
        var inputArray = mainFrame.document.getElementsByTagName( "input" );
        var aliasString = "";

        for( var i = 0; i < inputArray.length; i++ )
        {
            if ( inputArray[i].id != null )
            {
                if ( inputArray[i].id.indexOf( "alias." ) != -1 )
                {
                    aliasString = aliasString + inputArray[i].value + ",";
                }
            }
        }
        aliasString = aliasString.substring( 0, aliasString.length - 1 );
        mainFrame.document.update.elements["general.Aliases"].value = aliasString;
}

function openBrWindow(theURL,winName,features)
{ //v2.0
    theURL = protocol + "//" + host + theURL;
    var popupWindow = window.open(theURL,winName,features);
    popupWindow.focus();
}
