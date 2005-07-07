var saveFlag = "false";
var tab = "tab_General";

function validateExt( ext )
{
    if( isNaN( ext ) )
    {
        alert( "The extension field must be a number.\n" +
            "Please try again." );
        mainFrame.document.update.elements["general.Extension"].value = "";
    }
    else
    {
        setSaveFlag();
    }
}
function validateSerialNumber( serialNumber, deviceType )
{
    if( deviceType == "1" &&
        !validator.isSerialNumberXpressa(serialNumber) &&
        relaxSerialValidation != "true" )
    {
        var msg = "You have entered an invalid serial number.\n" +
                  "A valid serial number for a Xpressa device \n" +
                  "must start with 00d01e, and end with 6 characters, \n" +
                  "all of which must be either 0-9, a-f, or A-F, no spaces are allowed.\n" +
                  "Please try again.";
        alert (msg);
        mainFrame.document.update.elements["general.SerialNumber"].value = "00d01e";
    }
    else if( deviceType == "2" &&
        !validator.isSerialNumberIXpressa(serialNumber) &&
        relaxSerialValidation != "true" )
    {
        var msg = "You have entered an invalid serial number.\n" +
                  "A valid serial number for an Instant Xpressa device \n" +
                  "must start with ff0000, and end with 6 characters, \n" +
                  "all of which must be either 0-9, a-f, or A-F, no spaces are allowed.\n" +
                  "Please try again.";
        alert (msg);
        mainFrame.document.update.elements["general.SerialNumber"].value = "ff0000";
    }
    else
    {
        setSaveFlag();
    }

}



function setSaveFlag() {
    saveFlag = "true";
}

function clearSaveFlag()
{
    saveFlag = "false";
}

function checkFlag( pageToLoad )
{
    var msg = "Unsaved changes! To save your changes, click OK";
    if( saveFlag == "true" )
    {
        if( confirm(msg) )
        {
            submitForm();
        }
        else
        {
            clearSaveFlag();
            top.leftFrame.loadPage( pageToLoad );
        }
    }
}
function saveOnExit()
{
    var msg = "Unsaved changes! To save your changes, click OK \n"  +
        "(Please note that if you are closing the browser, \n" +
        " your changes will not be saved)";
    if( saveFlag == "true" )
    {
        if( confirm(msg) )
        {
            submitForm();
        }
    }
}


function checkForNull(valueToCheck,oldValue) {
    msg = "The name value cannot be blank, or null.";
    if( valueToCheck.value == "" ) {
        alert(msg);
        eval( "mainFrame.document.update.elements[\"" + valueToCheck.name + "\"].value = oldValue" );
    } else {
        setSaveFlag();
    }
}
function submitForm() {
    clearSaveFlag();
    mainFrame.document.update.submit();
}

function changeTab(tabName) {
    tab = tabName;
    var docRoot = mainFrame.document.getElementsByTagName("*");
    for ( var i = 0; i < docRoot.length; i++ ) {
        if ( docRoot[i].nodeName == 'DIV' && docRoot[i].id.indexOf( "tab_" ) != -1 ) {
            docRoot[i].style.display = 'none';
         }
    }
    mainFrame.document.getElementById(tabName).style.display = '';
}

function loadTab( ) {
    var docRoot = mainFrame.document.getElementsByTagName("*");
    for ( var i = 0; i < docRoot.length; i++ ) {
        if ( docRoot[i].nodeName == 'DIV' && docRoot[i].id.indexOf( "tab_" ) != -1 ) {
            docRoot[i].style.display = 'none';
         }
    }
    mainFrame.document.getElementById(tab).style.display = '';
}