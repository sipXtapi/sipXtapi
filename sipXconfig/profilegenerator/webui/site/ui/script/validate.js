//
// Define Validate class Constructor
//

function Validate() {}

Validate.prototype.isPhoneNumber = function(string)
{
    //var validNumberRegExp = /^(\*?\+?\d ?-?\*?\+?)+$/;
    var validNumberRegExp = /[^\d\*\#\+\-ip ]/;
    var isValid = !(validNumberRegExp.test(string));
    return isValid;
}

Validate.prototype.isUserId = function(string)
{
    var validUserIdRegExp = /[^a-z\d\_]/i;
    var isValid = !(validUserIdRegExp.test(string));
    return isValid;
}

Validate.prototype.isLegalName = function(string)
{
    var validNameRegExp =  /[^a-z\d\_\-'\.\+, ]/i;
    //var validNameRegExp = /[&\<\>]/i;
    var isValid = !(validNameRegExp.test(string));
    return isValid;
}

Validate.prototype.isSIPURL = function(string)
{
    var validSIPURL = /(sip:(\w)+@)|^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})$/i;
    var isValid = validSIPURL.test(string);
    return isValid;
}

Validate.prototype.isUserLineSIPURL = function(string)
{
    var validSIPURL = /(sip:(\w)+@)/i;
    var isValid = validSIPURL.test(string);
    return isValid;
}

Validate.prototype.isAlias = function( string )
{
    var validAlias = /sip:|@/i;
    var isValid = !(validAlias.test( string ) );
    return isValid;
}

Validate.prototype.isSerialNumber = function(string)
{
    var validSerialNumber = /[^a-z\d]/i;
    var isValid = !(validSerialNumber.test(string));
    return isValid;
}

Validate.prototype.isSerialNumberXpressa = function(string)
{
    var validSerialNumber = /^(00d01e[0-9a-fA-F]{6})$/i;
    var isValid = validSerialNumber.test(string);
    return isValid;
}

Validate.prototype.isSerialNumberIXpressa = function(string)
{
    var validSerialNumber = /^(ff0000[0-9a-fA-F]{6})$/i;
    var isValid = validSerialNumber.test(string);
    return isValid;
}

Validate.prototype.isSerialNumberOther = function(string)
{
    var validSerialNumber = /^[0-9a-fA-F]{12}$/i;
    var isValid = validSerialNumber.test(string);
    return isValid;
}

Validate.prototype.trim = function(string)
{
    var temp = string;
    var obj = /^(\s*)([\W\w]*)(\b\s*$)/;
    if (obj.test(temp)) { temp = temp.replace(obj, '$2'); }
    return temp;

}
