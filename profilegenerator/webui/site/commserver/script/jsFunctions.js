var valAddress = true;
var valDP = true;

function MM_openBrWindow(theURL,winName,features) {
    var popupWindow = window.open(theURL,winName,features);
    popupWindow.focus();
}

function closeWindow() {
    this.window.close();
}

function MM_goToURL() {
    var i, args=MM_goToURL.arguments; document.MM_returnValue = false;
    for (i=0; i<(args.length-1); i+=2) eval(args[i]+".location='"+args[i+1]+"'");
}

function MM_swapImgRestore() {
    var i,x,a=document.MM_sr; for (i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}

function MM_preloadImages() {
    var d=document; if (d.images) {
        if (!d.MM_p) d.MM_p=new Array();
        var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for (i=0; i<a.length; i++)
            if (a[i].indexOf("#")!=0) {
                d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];
            }
    }
}

function MM_findObj(n, d) {
    var p,i,x;  if (!d) d=document;if ((p=n.indexOf("?"))>0&&parent.frames.length) {
        d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);
    }
    if (!(x=d[n])&&d.all) x=d.all[n];for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
    for (i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
    if (!x && d.getElementById) x=d.getElementById(n);return x;
}

function MM_swapImage() {
    var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for (i=0;i<(a.length-2);i+=3)
        if ((x=MM_findObj(a[i]))!=null) {
            document.MM_sr[j++]=x; if (!x.oSrc) x.oSrc=x.src;x.src=a[i+2];
        }
}
function MM_callJS(jsStr) {
    return eval(jsStr)
}

function restoreToDefaults(url) {
    if ( confirm('Restore to Defaults cannot be undone. OK to continue?')) {
        top.frames['mainFrame'].location.href=url;
    }
}

function submitform() {
    clearFlag();
    top.saved = true;
    top.frames['mainFrame'].document.inputform.submit();
}

function submitDPForm() {
    valDP = true;
    clearFlag();
    top.saved = true;
    var msg = "WARNING! The results of this action will replace\n" +
                "your existing call routing rules, and you will need\n" +
                "to restart your commserver components for the \n" +
                "changes to take place.\n" +
                "Are you sure you want to continue?";
    var inputs = top.frames['mainFrame'].document.getElementsByTagName("input");
	for( var i = 0; i < inputs.length; i++ ) {
		if( inputs[i].type = "text" ) {
			dpValidate( inputs[i] );
			if( valDP == false )
				break;
		}
	}
    if ( valDP == true && confirm( msg ) ) {
        top.frames['mainFrame'].document.inputform.submit();
    }
}

function submitGWForm() {
    valAddress = true;
    clearFlag();
    top.saved = true;
    var msg = "WARNING! The results of this action will replace\n" +
                "your existing call routing rules, and you will need\n" +
                "to restart your commserver components for the \n" +
                "changes to take place.\n" +
                "Are you sure you want to continue?";
    var inputs = top.frames['mainFrame'].document.getElementsByTagName("input");
    for( var i = 0; i < inputs.length; i++ ) {
        if( inputs[i].id.indexOf( "gw" ) != -1 &&
            inputs[i].id.indexOf( "label" ) == -1 &&
            inputs[i].id.indexOf( "q" ) == -1 &&
            inputs[i].id.indexOf( "seq" ) == -1 ) {
            validateAddress( inputs[i] );
			if( valAddress == false )
                break;
        }
    }
    if ( valAddress == true && confirm( msg ) ) {
        setSeqAndQ();
		top.frames['mainFrame'].document.inputform.submit();
	}
}

function del_definition(id)
{
    var msg =   "Are you sure you want to delete\n" +
                "this definition?";
    var userInput = confirm( msg );
    if( userInput == true )
    {
        setFlag();
        var parent = document.getElementById( "globaldefinitionslist" );
        var child = document.getElementById( id );
        parent.removeChild( child );
    }
}

function add_definition( definition, value )
{
    var date = new Date();
    var id = date.getTime();
    var newdefinition = document.createElement("div");
    newdefinition.setAttribute( "id", id);
    document.getElementById("globaldefinitionslist").appendChild(newdefinition);
    newdefinition.innerHTML = "<table border='0' class='bglistmargins' cellpadding='4' cellspacing='0' width='625'>" +
                                "<tr>" +
                                    "<td>" + definition + "</td>" +
                                    "<td width=\"300\" nowrap=\"yes\">" +
                                        "<input type=\"text\" size=\"43\" value=\"" + value + "\" name=\"" + definition + "\"/>" +
                                        "<img src=\"../image/spacer.gif\" width=\"5\" />" +
                                        "<a href=\"#\">" +
                                            "<img src=\"../image/del.gif\" border=\"0\" onclick=\"del_definition('" + id + "')\" alt=\"Delete\"/>" +
                                        "</a>" +
                                    "</td>" +
                                "</tr>" +
                                "</table>" ;
    setFlag();                                
}

function addGateway() {
    setFlag();
    var date = new Date();
    var id = "gw"+ date.getTime();
    var gw = document.createElement( "div" );
    gw.setAttribute( "id", id );
    document.getElementById( "insert_gw" ).appendChild( gw );
    gw.innerHTML =  "<hr align=\"left\" width=\"600\" class=\"seperator\">" +
                    "<table cellspacing=\"0\" cellpadding=\"4\" border=\"0\" width=\"600\" class=\"bglist\">" +
					"<tr>" +
					    "<th colspan=\"2\" align=\"left\">Gateway</th>" +
						"<th align=\"right\">" +
						    "<a href=\"#\"><img onclick=\"delGateway( '" + id + "' )\" border=\"0\" src=\"../image/del.gif\"></a>" +
						"</th>" +
					"</tr>" +
				"</table>" +
				"<table cellspacing=\"1\" cellpadding=\"4\" border=\"0\" width=\"600\" class=\"bglist\">" +
					"<tr>" +
						"<td width=\"259\">Description" +
						"</td>" +
						"<td colspan=\"2\">" +
							"<input name=\"" + id + "_label\" size=\"40\" type=\"text\"  value=\"\">" +
						"</td>" +
					"</tr>" +
					"<tr>" +
						"<td width=\"259\">Address" +
						"</td>" +
						"<td colspan=\"2\">" +
							"<input id=\"" + id + "\" name=\"" + id + "\" size=\"40\" type=\"text\" value=\"\"  >" +
							"<input id=\"" + id + "_q\" name=\"" + id + "_q\" type=\"hidden\"  value=\"\">" +
							"<input id=\"" + id + "_seq\" name=\"" + id + "_seq\" type=\"hidden\"  value=\"\">" +
						"</td>" +
					"</tr>" +
					"<tr>" +
						"<td colspan=\"3\">" +
							"<a href=\"#\" onclick=\"moveUp( 'insert_gw','" + id + "')\">Move Up</a>&nbsp;&nbsp;" +
							"<a href=\"#\" onclick=\"moveDown( 'insert_gw','" + id + "')\">Move Down</a>" +
						"</td>" +
					"</tr>" +
				"</table>" +
				"<table cellspacing=\"0\" cellpadding=\"4\" border=\"0\" width=\"600\" class=\"bglist\">" +
					"<tr>" +
						"<th align=\"left\" colspan=\"2\">Alternates" +
						"</th>" +
						"<th align=\"right\"><a href=\"#\"><img border=\"0\" onclick=\"addAlt( '" + id + "' )\" src=\"../image/add.gif\"></a>" +
						"</th>" +
					"</tr>" +
				"</table>";

}

function addEmergencyGateway() {
    setFlag();
    var date = new Date();
    var id = "egw"+ date.getTime();
    var egw = document.createElement( "div" );
    egw.setAttribute( "id", id );
    document.getElementById( "insert_egw" ).appendChild( egw );
    egw.innerHTML = "<hr align=\"left\" width=\"600\" class=\"seperator\">" +
                    "<table cellspacing=\"0\" cellpadding=\"4\" border=\"0\" width=\"600\" class=\"bglist\">" +
					"<tr>" +
					    "<th colspan=\"2\" align=\"left\">Emergency Gateway</th>" +
						"<th align=\"right\">" +
						    "<a href=\"#\"><img onclick=\"delEmergencyGateway( '" + id + "' )\" border=\"0\" src=\"../image/del.gif\"></a>" +
						"</th>" +
					"</tr>" +
				"</table>" +
				"<table cellspacing=\"1\" cellpadding=\"4\" border=\"0\" width=\"600\" class=\"bglist\">" +
					"<tr>" +
						"<td width=\"259\">Description" +
						"</td>" +
						"<td colspan=\"2\">" +
							"<input name=\"" + id + "_label\" size=\"40\" type=\"text\"  value=\"\">" +
						"</td>" +
					"</tr>" +
					"<tr>" +
						"<td width=\"259\">Address" +
						"</td>" +
						"<td colspan=\"2\">" +
							"<input id=\"" + id + "\" name=\"" + id + "\" size=\"40\" type=\"text\"  value=\"\"  >" +
							"<input id=\"" + id + "_q\" name=\"" + id + "_q\" type=\"hidden\"  value=\"\">" +
							"<input id=\"" + id + "_seq\" name=\"" + id + "_seq\" type=\"hidden\"  value=\"\">" +
						"</td>" +
					"</tr>" +
					"<tr>" +
						"<td colspan=\"3\">" +
							"<a href=\"#\" onclick=\"moveUp( 'insert_egw','" + id + "')\">Move Up</a>&nbsp;&nbsp;" +
							"<a href=\"#\" onclick=\"moveDown( 'insert_egw','" + id + "')\">Move Down</a>" +
						"</td>" +
					"</tr>" +
				"</table>" +
				"<table cellspacing=\"0\" cellpadding=\"4\" border=\"0\" width=\"600\" class=\"bglist\">" +
					"<tr>" +
						"<th align=\"left\" colspan=\"2\">Alternates" +
						"</th>" +
						"<th align=\"right\"><a href=\"#\"><img border=\"0\" onclick=\"addAlt( '" + id + "' )\" src=\"../image/add.gif\"></a>" +
						"</th>" +
					"</tr>" +
				"</table>";

}

function delGateway( id ) {
    setFlag();
    var parent = document.getElementById( "insert_gw" );
    var child = document.getElementById( id );
    parent.removeChild( child );
}

function delEmergencyGateway( id ) {
    setFlag();
    var parent = document.getElementById( "insert_egw" );
    var child = document.getElementById( id );
    parent.removeChild( child );
}

function addAlt( parent ) {
    setFlag();
    var date = new Date();
    var id = parent + "_alt_" + date.getTime();
    var alt = document.createElement( "div" );
    alt.setAttribute( "id", id );
    document.getElementById( parent ).appendChild( alt );
    alt.innerHTML = "<table cellspacing=\"1\" cellpadding=\"4\" border=\"0\" width=\"600\" class=\"bglist\">" +
						"<tr>" +
							"<td width=\"259\">Alternate Address" +
							"</td>" +
							"<td colspan=\"2\">" +
								"<input name=\"" + id + "\" size=\"40\" type=\"text\" >&nbsp;" +
								"<a href=\"#\"><img onclick=\"delAlt( '" + parent + "','" + id + "' )\" border=\"0\"" +
								" src=\"../image/del.gif\"></a>" +
							"</td>" +
						"</tr>" +
					"</table>";
}

function delAlt( parent, id ) {
    setFlag();
    var parentNode = document.getElementById( parent );
    var child = document.getElementById( id );
    parentNode.removeChild( child );
}

function moveUp( parent, id ) {
    setFlag();
    // check to see if we are already at the top of the list
    var node = document.getElementById( id );
	var prevNode = node.previousSibling;
    while ( prevNode != null && prevNode.nodeName != "DIV"  ) {
		prevNode = prevNode.previousSibling;
	}
	if ( prevNode != null ) {
		var parent = document.getElementById( parent );
		tempNode = node;
		parent.removeChild( node );
		parent.insertBefore( tempNode, prevNode );
	}

}

function moveDown( parent, id ) {
    setFlag();
	// check to see if we are already at the bottom of the list
    var node = document.getElementById( id );
	var nextNode = node.nextSibling;
    while ( nextNode != null && nextNode.nodeName != "DIV"  ) {
		nextNode = nextNode.nextSibling;
	}
	if ( nextNode != null ) {
		var parent = document.getElementById( parent );
		tempNode = nextNode;
		parent.removeChild( nextNode );
		parent.insertBefore( tempNode, node );
	}
}

function setSeqAndQ( ) {
	var parent = top.mainFrame.document.getElementById( "insert_gw" );
	var children = parent.childNodes;
	var id = null;
	var count = 1;

	for ( var i = 0; i < children.length; i ++ ) {
		if ( children[i].nodeName == "DIV" && children[i].id.indexOf( "gw" ) != -1 ) {
			id = children[i].id;
			top.mainFrame.document.getElementById( id + "_seq" ).value = count;
			if ( count == 1 ) {
			    top.mainFrame.document.getElementById( id + "_q" ).value = "1.0";
            }
            else {
			    top.mainFrame.document.getElementById( id + "_q" ).value = 1 - ( ( count - 1 ) * .01 );
            }
			++count;
		}
	}

	parent = top.mainFrame.document.getElementById( "insert_egw" );
	children = parent.childNodes;
	count = 1;
	for ( var i = 0; i < children.length; i ++ ) {
		if ( children[i].nodeName == "DIV" && children[i].id.indexOf( "egw" ) != -1 ) {
			id = children[i].id;
			top.mainFrame.document.getElementById( id + "_seq" ).value = count;
			if ( count == 1 ) {
			    top.mainFrame.document.getElementById( id + "_q" ).value = "1.0";
            }
            else {
			    top.mainFrame.document.getElementById( id + "_q" ).value = 1 - ( ( count - 1 ) * .01 );
            }
			++count;
		}
	}
}

function validateAddress ( addressToCheck ) {

    var value = addressToCheck.value;
    var ipDomainPat = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})/;
	var addressPat = /(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})\b|^([\w\d][-\w\d]*(\.[\w\d][-\w]*)+\.(com\b|edu\b|biz\b|gov\b|net\b|info\b|mil\b|org\b|[a-z][a-z]\b)))((\:\d+)?)$/i;

    var addressTest = value.match( addressPat );
	var ipTest = value.match( ipDomainPat );
    if ( addressTest == null ) {
        alert( "You have entered an invalid address, please try again" );
        addressToCheck.value = "";
        valAddress = false;
    }
	if ( ipTest != null ){
		for ( var i = 0; i < ipTest.length; i++ ) {
			if ( ipTest[i] > 255 ) {
				alert( "The IP Address you have entered contains values > 255, please try again" );
				addressToCheck.value = "";
				valAddress = false;
			}
		}
	}
}

 function dpValidate ( dpToCheck ) {
    var value = dpToCheck.value;
    var name = dpToCheck.name;

    if ( name == "internalextensions" ) {
        var intExtPat = /^\[[0-9]-[0-9]\]x+$/;
        var intExtTest = value.match( intExtPat );
        if ( intExtTest == null ) {
            alert( "The internal station numbering format\n" +
                "is invalid, please try again" );
            dpToCheck.value = "";
			valDP = false;
        }
    }
    else if ( name == "did" ) {
        var didPat = /^[0-9]*$/;
        var didTest = value.match( didPat );
        if ( didTest == null ) {
            alert( "The did prefix is invalid, please try again" );
            dpToCheck.value = "";
			valDP = false;
        }
    }
    else if ( name == "autoattendant" ) {
        var aaPat = /^[0-9]+$/;
        var aaTest = value.match( aaPat );
        if ( aaTest == null ) {
            alert( "The auto attendant number is invalid, please try again" );
            dpToCheck.value = "";
			valDP = false;
        }
    }
    else if ( name == "retrievevoicemail" ) {
        var rvmPat = /^[0-9]+$/;
        var rvmTest = value.match( rvmPat );
        if ( rvmTest == null ) {
            alert( "The retrieve voice mail number is invalid, please try again" );
            dpToCheck.value = "";
			valDP = false;
        }
    }
    else if ( name == "pstnprefix" ) {
        var pstnPat = /^[0-9]$/;
        var pstnTest = value.match( pstnPat );
        if ( pstnTest == null ) {
            alert( "The PSTN prefix is invalid, please try again" );
            dpToCheck.value = "";
			valDP = false;
        }
    }
    else if ( name == "vmprefixfromextension" ) {
        var vmpPat = /^[0-9]$/;
        var vmpTest = value.match( vmpPat );
        if ( vmpTest == null ) {
            alert( "The Voicemail prefix extension is invalid, please try again" );
            dpToCheck.value = "";
			valDP = false;
        }
    }
    else if ( name == "intldialprefix" ) {
        var idPat = /^[0-9]+$/;
        var idTest = value.match( idPat );
        if ( idTest == null ) {
            alert( "The international dialing prefix is invalid, please try again" );
            dpToCheck.value = "";
			valDP = false;
        }
    }

}

function checkSaveFlag( source ) {
    var msg = "Unsaved changes! To save your changes, click OK \n"  +
        "(Please note that if you are closing the browser, \n" +
        " your changes will not be saved)";
    if( top.unloadFlag == true && top.saved == false ) {
        clearFlag();
        if ( confirm( msg ) ) {
            if ( source == "dialplan" ) {
                submitDPForm();
            }
            else if ( source == "gateway" ) {
                submitGWForm();
            }
            else {
                submitform();
            }
        }
    }
    top.saved = false;
}
function setFlag() {
    top.unloadFlag = true;
 }

 function clearFlag() {
    top.unloadFlag = false;
}
