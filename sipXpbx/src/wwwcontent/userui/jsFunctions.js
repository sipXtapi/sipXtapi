function MM_preloadImages() { 
    var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
    var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
    if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}}

function MM_findObj(n, d) { var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
    if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
    for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document); return x;}

function MM_swapImage() {var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}}

function MM_swapImgRestore() { var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;}

function displayHelpPage(destPage){
    // The technique used for displaying help window is a little silly, 
    // but we are stuck with it due to the way our help is organized.
    // Help files are always mediaserver.htm#<anchor name>. 
    // When a help file is loaded on a already opened help window, as the change is in <anchor name>, 
    // the browser thinks that this is the old page and does not load the new one. 
    // User's only option is to close and click on the help link again.

    // Solution: 
    // 1. open the help window. If already open, nothing happens.
    // 2. close it.
    // 3. reopen it.

    helpwindow=window.open(destPage, 'HelpWindow', 'scrollbars,menubar,location=no,resizable,width=800,height=500'); 
    helpwindow.close();
    helpwindow=window.open(destPage, 'HelpWindow', 'scrollbars,menubar,location=no,resizable,width=800,height=500'); 
    helpwindow.focus();
}

function MM_displayStatusMsg(msgStr){ status=msgStr; document.MM_returnValue = true; }

var messagelist;
function createMsgList( msgid ){
	if( messagelist == undefined )
		messagelist = msgid ;
	else
		messagelist += ' ' + msgid ;
}

function getMessages( type, form )
{
    var inputs = document.getElementsByTagName( "INPUT" );
    var messagelist = "";

    for (  var i = 0; i < inputs.length; i++ )
    {
        if ( inputs[i].type == "checkbox" )
        {
            if ( type == "checked" ) 
            {
                if ( inputs[i].checked )
                {
                    messagelist = messagelist + ' ' + inputs[i].name;
                }
            } else
            {
                messagelist = messagelist + ' ' + inputs[i].name;
            }
        }
    }
    form.submit();
}

function validateFoldername( form )
{
    var foldername = form.newfoldername.value ;
    for (i = 0; i < foldername.length; i++)
    {   
        var c = foldername.charAt(i);
        if(	c == '&' || 
			c == '?' || 
			c == '%' || 
			c == '*' || 
			c == '|' || 
			c == '\"' || 
			c == ':' ||	
			c == '<' ||	
			c == '>' ||	
			c == '/' || 
			c == '\\')
		{
			alert("Invalid folder name - contains '" + c + "'.\nRecommended special characters are space, hyphen and underscore");
	        return false;
		}
    }

    return true;
}


function reloadMainFrame()
{
    if (document.images)
        top.mainFrame.location.reload();
    else
        top.mainFrame.location.href = top.mainFrame.location.href;
}

function CheckAll()
{
	var ml = document.messageList; 
	var len = ml.elements.length;  
	for (var i = 0; i < len; i++) {   
        	var e = ml.elements[i];   
		e.checked = true;   
	}  
}  

function ClearAll()   
{  
	var ml = document.messageList;   
	var len = ml.elements.length;   
	for (var i = 0; i < len; i++) {   
		var e = ml.elements[i];   
		e.checked = false;   
	}  
}  

function playMsgJs(url) 
{ 
	if( detectWindowsMedia() )	
	{
        var version = Player.versionInfo;
        if( version )
    		Player.URL = url; 
        else
            setTimeout('location.href="' + url + '"', 0);
	}
    else
    {
        setTimeout('location.href="' + url + '"', 0);
    }
} 

function embedMediaPlayer()
{
    if( detectWindowsMedia() )
    {
        document.write('<OBJECT ID="Player" height="0" width="0" ');
        document.writeln('CLASSID="CLSID:6BF52A52-394A-11d3-B153-00C04F79FAA6">');
        document.writeln('</OBJECT>');
    }
} 

// initialize global variables
var detectableWithVB = false;
var pluginFound = false;

function detectWindowsMedia(redirectURL, redirectIfFound) {
    // Check for media player only when using IE
    if (navigator.userAgent.indexOf('MSIE') != -1)
    {
        pluginFound = detectPlugin('Windows Media Player');
        // if not found, try to detect with VisualBasic
        if(!pluginFound && detectableWithVB) {
	        pluginFound = detectActiveXControl('MediaPlayer.MediaPlayer.1');
        }
    }
    return pluginFound;
}

function detectPlugin() {
    // allow for multiple checks in a single pass
    var daPlugins = detectPlugin.arguments;
    // consider pluginFound to be false until proven true
    var pluginFound = false;
    // if plugins array is there and not fake
    if (navigator.plugins && navigator.plugins.length > 0) {
	var pluginsArrayLength = navigator.plugins.length;
	// for each plugin...
	for (pluginsArrayCounter=0; pluginsArrayCounter < pluginsArrayLength; pluginsArrayCounter++ ) {
	    // loop through all desired names and check each against the current plugin name
	    var numFound = 0;
	    for(namesCounter=0; namesCounter < daPlugins.length; namesCounter++) {
		// if desired plugin name is found in either plugin name or description
		if( (navigator.plugins[pluginsArrayCounter].name.indexOf(daPlugins[namesCounter]) >= 0) || 
		    (navigator.plugins[pluginsArrayCounter].description.indexOf(daPlugins[namesCounter]) >= 0) ) {
		    // this name was found
		    numFound++;
		}   
	    }
	    // now that we have checked all the required names against this one plugin,
	    // if the number we found matches the total number provided then we were successful
	    if(numFound == daPlugins.length) {
		pluginFound = true;
		// if we've found the plugin, we can stop looking through at the rest of the plugins
		break;
	    }
	}
    }
    return pluginFound;
} // detectPlugin


// Here we write out the VBScript block for MSIE Windows
if ((navigator.userAgent.indexOf('MSIE') != -1) && (navigator.userAgent.indexOf('Win') != -1)) {
    document.writeln('<script language="VBscript">');

    document.writeln('\'do a one-time test for a version of VBScript that can handle this code');
    document.writeln('detectableWithVB = False');
    document.writeln('If ScriptEngineMajorVersion >= 2 then');
    document.writeln('  detectableWithVB = True');
    document.writeln('End If');

    document.writeln('\'this next function will detect most plugins');
    document.writeln('Function detectActiveXControl(activeXControlName)');
    document.writeln('  on error resume next');
    document.writeln('  detectActiveXControl = False');
    document.writeln('  If detectableWithVB Then');
    document.writeln('     detectActiveXControl = IsObject(CreateObject(activeXControlName))');
    document.writeln('  End If');
    document.writeln('End Function');

    document.writeln('</scr' + 'ipt>');
}

