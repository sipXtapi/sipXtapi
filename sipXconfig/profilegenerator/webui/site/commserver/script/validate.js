//
// Define Validate class Constructor
//

function Validate() {}

Validate.prototype.trim = function(string)
{
    var temp = string;
    var obj = /^(\s*)([\W\w]*)(\b\s*$)/;
    if (obj.test(temp)) { temp = temp.replace(obj, '$2'); }
    return temp;

}

Validate.prototype.preprocessorvalue = function(string)
{
    var temp = string;
    var obj = /^(\$\{*)([\W\w]*)(\b\}*$)/;
    if (obj.test(temp)) { temp = temp.replace(obj, '$2'); }
    return temp;

}


