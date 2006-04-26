dojo.provide("dojo.widget.validate");

dojo.provide("dojo.widget.validate.Textbox");
dojo.provide("dojo.widget.validate.ValidationTextbox");
dojo.provide("dojo.widget.validate.IntegerTextbox");
dojo.provide("dojo.widget.validate.RealNumberTextbox");
dojo.provide("dojo.widget.validate.CurrencyTextbox");
dojo.provide("dojo.widget.validate.IpAddressTextbox");
dojo.provide("dojo.widget.validate.UrlTextbox");
dojo.provide("dojo.widget.validate.EmailTextbox");
dojo.provide("dojo.widget.validate.EmailListTextbox");
dojo.provide("dojo.widget.validate.DateTextbox");
dojo.provide("dojo.widget.validate.TimeTextbox");
dojo.provide("dojo.widget.validate.UsStateTextbox");
dojo.provide("dojo.widget.validate.UsZipTextbox");
dojo.provide("dojo.widget.validate.UsPhoneNumberTextbox");

dojo.require("dojo.widget.Widget");
dojo.require("dojo.widget.Manager");
dojo.require("dojo.widget.Parse");
dojo.require("dojo.xml.Parse");
dojo.require("dojo.lang");
dojo.require("dojo.validate");

dojo.widget.manager.registerWidgetPackage("dojo.widget.validate");


/*
  ****** Textbox ******

  This widget is a generic textbox field.
  Serves as a base class to derive more specialized functionality in subclasses.
  Has the following properties that can be specified as attributes in the markup.

  @attr id         The textbox id attribute.
  @attr className  The textbox class attribute.
  @attr name       The textbox name attribute.
  @attr value      The textbox value attribute.
  @attr trim       Removes leading and trailing whitespace if true.  Default is false.
  @attr uppercase  Converts all characters to uppercase if true.  Default is false.
  @attr lowercase  Converts all characters to lowercase if true.  Default is false.
  @attr ucFirst    Converts the first character of each word to uppercase if true.
  @attr lowercase  Removes all characters that are not digits if true.  Default is false.
*/
dojo.widget.validate.Textbox = function() {  }

dojo.inherits(dojo.widget.validate.Textbox, dojo.widget.Widget);

dojo.lang.extend(dojo.widget.validate.Textbox, {
	// default values for new subclass properties
	widgetId: "", 
	widgetType: "Textbox", 
	id: "",
	className: "",
	name: "",
	value: "",
	trim: false,
	uppercase: false,
	lowercase: false,
	ucFirst: false,
	digit: false,
	
	// our DOM nodes
	replaceNode: null,
	widgetDomNode: null,
	textbox: null,

	// Apply various filters to textbox value
	filter: function() { 
		if (this.trim) {
			this.textbox.value = this.textbox.value.replace(/(^\s*|\s*$)/g, "");
		} 
		if (this.uppercase) {
			this.textbox.value = this.textbox.value.toUpperCase();
		} 
		if (this.lowercase) {
			this.textbox.value = this.textbox.value.toLowerCase();
		} 
		if (this.ucFirst) {
			this.textbox.value = this.textbox.value.replace(/\b\w+\b/g, 
				function(word) { return word.substring(0,1).toUpperCase() + word.substring(1).toLowerCase(); });
		} 
		if (this.digit) {
			this.textbox.value = this.textbox.value.replace(/\D/g, "");
		} 
	},

	// event handlers, you can over-ride these in your own subclasses
	onfocus: function() {},
	onblur: function() { this.filter(); },

	// All functions below are called by create from dojo.widget.Widget
	mixInProperties: function(localProperties, frag) {
		// Save the reference to the node in the DOM that we are going to replace with the widget.
		this.replaceNode = frag["dojo:" + this.widgetType.toLowerCase()].nodeRef;

		// Get widget properties from markup attibutes, and convert to correct type.
		if ( localProperties.id ) { 
			this.id = localProperties.id;
		}
		if ( localProperties["class"] ) { 
			this.className = localProperties["class"];
		}
		if ( localProperties.name ) { 
			this.name = localProperties.name;
		}
		if ( localProperties.value ) { 
			this.value = localProperties.value;
		}
		if ( localProperties.name ) { 
			this.name = localProperties.name;
		}
		if ( localProperties.trim ) { 
			this.trim = ( localProperties.trim == "true" );
		}
		if ( localProperties.uppercase ) { 
			this.uppercase = ( localProperties.uppercase == "true" );
		}
		if ( localProperties.lowercase ) { 
			this.lowercase = ( localProperties.lowercase == "true" );
		}
		if ( localProperties.ucfirst ) { 
			this.ucFirst = ( localProperties.ucfirst == "true" );
		}
		if ( localProperties.digit ) { 
			this.digit = ( localProperties.digit == "true" );
		}
	},

	buildRendering: function() {
		this.textbox = document.createElement("input");

		// Attach widget methods as event handlers to the textbox.  The tricky part is
		// that 'this' must continue to refer this widget object while in the event handler.
		var _this = this;
		this.textbox.onblur = function() { _this.onblur.call(_this); };
		this.textbox.onfocus = function() { _this.onfocus.call(_this); };

		this.widgetDomNode = this.textbox;
	},

	initialize: function() {
		this.textbox.id = this.id;
		this.textbox.name = this.name;
		this.textbox.value = this.value;
		this.textbox.className = this.className;

		// apply any filters to initial value
		this.filter();
	},

	postCreate: function() {
		// Attach widget to DOM
		this.replaceNode.parentNode.replaceChild(this.widgetDomNode, this.replaceNode);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:Textbox");


/*
  ****** ValidationTextbox ******

  A subclass of Textbox.
  Over-ride isValid in subclasses to perform specific kinds of validation.
  Has 4 new properties that can be specified as attributes in the markup.

  @attr required          Can be true or false, default is false.
  @attr validColor        The color textbox is highlighted for valid input. Default is #cfc.
  @attr invalidColor      The color textbox is highlighted for invalid input. Default is #fcc.
  @attr missingMessage    The message to display if value is missing.
  @attr invalidMessage    The message to display if value is invalid.
  @attr listenOnKeyPress  Updates messages on each key press.  Default is true.
*/
dojo.widget.validate.ValidationTextbox = function() {}

dojo.inherits(dojo.widget.validate.ValidationTextbox, dojo.widget.validate.Textbox);

dojo.lang.extend(dojo.widget.validate.ValidationTextbox, {
	// default values for new subclass properties
	widgetType: "ValidationTextbox", 
	required: false,
	validColor: "#cfc",
	invalidColor: "#fcc",
	invalidMessage: "* The value entered is not valid.",
	missingMessage: "* This value is required.",
	listenOnKeyPress: true,

	// new DOM nodes
	invalidSpan: null,
	missingSpan: null,

	// Need to over-ride with your own validation code in subclasses
	isValid: function() { return true; },

	// Returns true if value is all whitespace
	isEmpty: function() { 
		return ( /^\s*$/.test(this.textbox.value) );
	},

	// Returns true if value is required and it is all whitespace.
	isMissing: function() { 
		return ( this.required && this.isEmpty() );
	},

	// Called oninit, onblur, and onkeypress.
	// Show missing or invalid messages if appropriate, and highlight textbox field.
	update: function() {
		this.missingSpan.style.display = "none";
		this.invalidSpan.style.display = "none";

		var empty = this.isEmpty();
		var valid = this.isValid();
		var missing = this.isMissing();

		// Display at most one error message
		if ( missing ){
			this.missingSpan.style.display = "";
		}
		else if ( !empty && !valid ){
			this.invalidSpan.style.display = "";
		}
	},

	// Called oninit, and onblur.
	highlight: function() {
		// highlight textbox background 
		if ( this.isEmpty() ) {
			this.textbox.style.backgroundColor = "";
		}
		else if ( this.isValid() ) {
			this.textbox.style.backgroundColor = this.validColor;
		}
		else {
			this.textbox.style.backgroundColor = this.invalidColor;
		}
	},

	onfocus: function() {
		// Put the textbox background back to normal
		this.textbox.style.backgroundColor = "";
	},

	onblur: function() { 
		this.filter();
		this.update(); 
		this.highlight(); 
	},

	onkeypress: function() { 
		if ( this.listenOnKeyPress ) { 
			//this.filter();  trim is problem if you have to type two words
			this.update(); 
		} 
	},

	mixInProperties: function(localProperties, frag) {
		// First initialize properties in super-class.
		dojo.widget.validate.Textbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get widget properties from markup attibutes, and convert to correct type.
		if ( localProperties.listenonkeypress ) { 
			this.listenOnKeyPress = ( localProperties.listenonkeypress == "true" );
		}
		if ( localProperties.required ) { 
			this.required = ( localProperties.required == "true" );
		}
		if ( localProperties.invalidmessage ) { 
			this.invalidMessage = localProperties.invalidmessage;
		}
		if ( localProperties.missingmessage ) { 
			this.missingMessage = localProperties.missingmessage;
		}
		if ( localProperties.validcolor ) { 
			this.validColor = localProperties.validcolor;
		}
		if ( localProperties.invalidcolor ) { 
			this.invalidColor = localProperties.invalidcolor;
		}
	},

	buildRendering: function() {
		this.textbox = document.createElement("input");

		// Attach widget methods as event handlers to the textbox.  The tricky part is
		// that 'this' must continue to refer this widget object while in the event handler.
		var _this = this;
		this.textbox.onblur = function() { _this.onblur.call(_this); };
		this.textbox.onfocus = function() { _this.onfocus.call(_this); };
		this.textbox.onkeyup = function() { _this.onkeypress.call(_this); };

		// Attach isMissing and isValid methods to the textbox.
		// We may use them later in connection with a submit button widget.
		this.textbox.isValid = function() { _this.isValid.call(_this); };
		this.textbox.isMissing = function() { _this.isMissing.call(_this); };

		// Create spans for error messages.
		this.invalidSpan = document.createElement("span");
		this.missingSpan = document.createElement("span");

		// Put it all together
		var container = document.createElement("div");
		container.appendChild(this.textbox);
		container.appendChild(this.invalidSpan);
		container.appendChild(this.missingSpan);

		this.widgetDomNode = container;
	},

	initialize: function() {
		// init textbox
		this.textbox.id = this.id;
		this.textbox.name = this.name;
		this.textbox.value = this.value;
		this.textbox.className = this.className;

		// apply any filters to initial value
		this.filter();

		// highlight textbox as valid or invalid
		this.highlight(); 

		// init error message spans
		this.invalidSpan.innerHTML = this.invalidMessage;
		this.invalidSpan.className = "invalid";
		this.missingSpan.innerHTML = this.missingMessage;
		this.missingSpan.className = "missing";

		// show missing or invalid messages on init
		this.update(); 
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:ValidationTextbox");


/*
  ****** IntegerTextbox ******

  A subclass of ValidationTextbox.
  Over-rides isValid to test for integer input.
  Has two new properties that can be specified as attributes in the markup.

  @attr signed     The leading plus-or-minus sign. Can be true or false, default is either.
  @attr separator  The character used as the thousands separator.  Default is no separator.
*/
dojo.widget.validate.IntegerTextbox = function(node) {
	// this property isn't a primitive and needs to be created on a per-item basis.
	this.flags = {};
}

dojo.inherits(dojo.widget.validate.IntegerTextbox, dojo.widget.validate.ValidationTextbox);

dojo.lang.extend(dojo.widget.validate.IntegerTextbox, {
	// new subclass properties
	widgetType: "IntegerTextbox", 

	mixInProperties: function(localProperties, frag) {
		// First initialize properties in super-class.
		dojo.widget.validate.ValidationTextbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get properties from markup attibutes, and assign to flags object.
		if ( localProperties.signed ) { 
			this.flags.signed = ( localProperties.signed == "true" );
		}
		if ( localProperties.separator ) { 
			this.flags.separator = localProperties.separator;
		}
	},

	// Over-ride for integer validation
	isValid: function() { 
		return dojo.validate.isInteger(this.textbox.value, this.flags);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:IntegerTextbox");


/*
  ****** RealNumberTextbox ******

  A subclass that extends IntegerTextbox.
  Over-rides isValid to test for real number input.
  Has three new properties that can be specified as attributes in the markup.

  @attr places    The exact number of decimal places.  If omitted, it's unlimited and optional.
  @attr exponent  Can be true or false.  If omitted the exponential part is optional.
  @attr eSigned   Is the exponent signed?  Can be true or false, if omitted the sign is optional.
*/
dojo.widget.validate.RealNumberTextbox = function(node) {
	this.flags = {};
}

dojo.inherits(dojo.widget.validate.RealNumberTextbox, dojo.widget.validate.IntegerTextbox);

dojo.lang.extend(dojo.widget.validate.RealNumberTextbox, {
	// new subclass properties
	widgetType: "RealNumberTextbox", 

	mixInProperties: function(localProperties, frag) {
		// First initialize properties in super-class.
		dojo.widget.validate.IntegerTextbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get properties from markup attibutes, and assign to flags object.
		if ( localProperties.places ) { 
			this.flags.places = Number( localProperties.places );
		}
		if ( localProperties.exponent ) { 
			this.flags.exponent = ( localProperties.exponent == "true" );
		}
		if ( localProperties.esigned ) { 
			this.flags.eSigned = ( localProperties.esigned == "true" );
		}
	},

	// Over-ride for real number validation
	isValid: function() { 
		return dojo.validate.isRealNumber(this.textbox.value, this.flags);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:RealNumberTextbox");


/*
  ****** CurrencyTextbox ******

  A subclass that extends IntegerTextbox.
  Over-rides isValid to test if input denotes a monetary value .
  Has 2 new properties that can be specified as attributes in the markup.

  @attr cents      The two decimal places for cents.  Can be true or false, optional if omitted.
  @attr symbol     A currency symbol such as Yen "¥", Pound "£", or the Euro "€". Default is "$".
  @attr separator  Default is "," instead of no separator as in IntegerTextbox.
*/
dojo.widget.validate.CurrencyTextbox = function(node) {
	this.flags = {};
}

dojo.inherits(dojo.widget.validate.CurrencyTextbox, dojo.widget.validate.IntegerTextbox);

dojo.lang.extend(dojo.widget.validate.CurrencyTextbox, {
	// new subclass properties
	widgetType: "CurrencyTextbox", 

	mixInProperties: function(localProperties, frag) {
		// First initialize properties in super-class.
		dojo.widget.validate.IntegerTextbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get properties from markup attibutes, and assign to flags object.
		if ( localProperties.cents ) { 
			this.flags.cents = ( localProperties.cents == "true" );
		}
		if ( localProperties.symbol ) { 
			this.flags.symbol = localProperties.symbol;
		}
	},

	// Over-ride for currency validation
	isValid: function() { 
		return dojo.validate.isCurrency(this.textbox.value, this.flags);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:CurrencyTextbox");


/*
  ****** IpAddressTextbox ******

  A subclass of ValidationTextbox.
  Over-rides isValid to test for IP addresses.
  Can specify formats for ipv4 or ipv6 as attributes in the markup.

  @attr allowDottedDecimal  true or false, default is true.
  @attr allowDottedHex      true or false, default is true.
  @attr allowDottedOctal    true or false, default is true.
  @attr allowDecimal        true or false, default is true.
  @attr allowHex            true or false, default is true.
  @attr allowIPv6           true or false, default is true.
  @attr allowHybrid         true or false, default is true.
*/
dojo.widget.validate.IpAddressTextbox = function(node) {
	this.flags = {};
}

dojo.inherits(dojo.widget.validate.IpAddressTextbox, dojo.widget.validate.ValidationTextbox);

dojo.lang.extend(dojo.widget.validate.IpAddressTextbox, {
	// new subclass properties
	widgetType: "IpAddressTextbox", 

	mixInProperties: function(localProperties, frag) {
		// First initialize properties in super-class.
		dojo.widget.validate.ValidationTextbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get properties from markup attibutes, and assign to flags object.
		if ( localProperties.allowdotteddecimal ) { 
			this.flags.allowDottedDecimal = ( localProperties.allowdotteddecimal == "true" );
		}
		if ( localProperties.allowdottedhex ) { 
			this.flags.allowDottedHex = ( localProperties.allowdottedhex == "true" );
		}
		if ( localProperties.allowdottedoctal ) { 
			this.flags.allowDottedOctal = ( localProperties.allowdottedoctal == "true" );
		}
		if ( localProperties.allowdecimal ) { 
			this.flags.allowDecimal = ( localProperties.allowdecimal == "true" );
		}
		if ( localProperties.allowhex ) { 
			this.flags.allowHex = ( localProperties.allowhex == "true" );
		}
		if ( localProperties.allowipv6 ) { 
			this.flags.allowIPv6 = ( localProperties.allowipv6 == "true" );
		}
		if ( localProperties.allowhybrid ) { 
			this.flags.allowHybrid = ( localProperties.allowhybrid == "true" );
		}
	},

	// Over-ride for IP address validation
	isValid: function() { 
		return dojo.validate.isIpAddress(this.textbox.value, this.flags);
	}
});

dojo.widget.tags.addParseTreeHandler("dojo:IpAddressTextbox");


/*
  ****** UrlTextbox ******

  A subclass of IpAddressTextbox.
  Over-rides isValid to test for URL's.
  Can specify 5 additional attributes in the markup.

  @attr scheme        Can be true or false.  If omitted the scheme is optional.
  @attr allowIP       Allow an IP address for hostname.  Default is true.
  @attr allowLocal    Allow the host to be "localhost".  Default is false.
  @attr allowCC       Allow 2 letter country code domains.  Default is true.
  @attr allowGeneric  Allow generic domains.  Can be true or false, default is true.
*/
dojo.widget.validate.UrlTextbox = function(node) {
	this.flags = {};
}

dojo.inherits(dojo.widget.validate.UrlTextbox, dojo.widget.validate.IpAddressTextbox);

dojo.lang.extend(dojo.widget.validate.UrlTextbox, {
	// new subclass properties
	widgetType: "UrlTextbox", 

	mixInProperties: function(localProperties, frag) {
		// First initialize properties in super-class.
		dojo.widget.validate.IpAddressTextbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get properties from markup attibutes, and assign to flags object.
		if ( localProperties.scheme ) { 
			this.flags.scheme = ( localProperties.scheme == "true" );
		}
		if ( localProperties.allowip ) { 
			this.flags.allowIP = ( localProperties.allowip == "true" );
		}
		if ( localProperties.allowlocal ) { 
			this.flags.allowLocal = ( localProperties.allowlocal == "true" );
		}
		if ( localProperties.allowcc ) { 
			this.flags.allowCC = ( localProperties.allowcc == "true" );
		}
		if ( localProperties.allowgeneric ) { 
			this.flags.allowGeneric = ( localProperties.allowgeneric == "true" );
		}
	},

	// Over-ride for URL validation
	isValid: function() { 
		return dojo.validate.isUrl(this.textbox.value, this.flags);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:UrlTextbox");


/*
  ****** EmailTextbox ******

  A subclass of UrlTextbox.
  Over-rides isValid to test for email addresses.
  Can use all markup attributes/properties of UrlTextbox except scheme.
  One new attribute available in the markup.

  @attr allowCruft  Allow address like <mailto:foo@yahoo.com>.  Default is false.
*/
dojo.widget.validate.EmailTextbox = function(node) {
	this.flags = {};
}

dojo.inherits(dojo.widget.validate.EmailTextbox, dojo.widget.validate.UrlTextbox);

dojo.lang.extend(dojo.widget.validate.EmailTextbox, {
	// new subclass properties
	widgetType: "EmailTextbox", 

	mixInProperties: function(localProperties, frag) {
		// First initialize properties in super-class.
		dojo.widget.validate.UrlTextbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get properties from markup attibutes, and assign to flags object.
		if ( localProperties.allowcruft ) { 
			this.flags.allowCruft = ( localProperties.allowcruft == "true" );
		}
	},

	// Over-ride for email address validation
	isValid: function() { 
		return dojo.validate.isEmailAddress(this.textbox.value, this.flags);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:EmailTextbox");


/*
  ****** EmailListTextbox ******

  A subclass of EmailTextbox.
  Over-rides isValid to test for a list of email addresses.
  Can use all markup attributes/properties of EmailTextbox and ...

  @attr listSeparator  The character used to separate email addresses.  
    Default is ";", ",", "\n" or " ".
*/
dojo.widget.validate.EmailListTextbox = function(node) {
	this.flags = {};
}

dojo.inherits(dojo.widget.validate.EmailListTextbox, dojo.widget.validate.EmailTextbox);

dojo.lang.extend(dojo.widget.validate.EmailListTextbox, {
	// new subclass properties
	widgetType: "EmailListTextbox", 

	mixInProperties: function(localProperties, frag) {
		// First initialize properties in super-class.
		dojo.widget.validate.EmailTextbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get properties from markup attibutes, and assign to flags object.
		if ( localProperties.listseparator ) { 
			this.flags.listSeparator = localProperties.listseparator;
		}
	},

	// Over-ride for email address list validation
	isValid: function() { 
		return dojo.validate.isEmailAddressList(this.textbox.value, this.flags);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:EmailListTextbox");


/*
  ****** DateTextbox ******

  A subclass of ValidationTextbox.
  Over-rides isValid to test if input is in a valid date format.

  @attr format  Described in dojo.validate.js.  Default is  "MM/DD/YYYY".
*/
dojo.widget.validate.DateTextbox = function(node) {
	this.flags = {};
}

dojo.inherits(dojo.widget.validate.DateTextbox, dojo.widget.validate.ValidationTextbox);

dojo.lang.extend(dojo.widget.validate.DateTextbox, {
	// new subclass properties
	widgetType: "DateTextbox", 

	mixInProperties: function(localProperties, frag) {
		// First initialize properties in super-class.
		dojo.widget.validate.ValidationTextbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get properties from markup attibutes, and assign to flags object.
		if ( localProperties.format ) { 
			this.flags.format = localProperties.format;
		}
	},

	// Over-ride for date validation
	isValid: function() { 
		return dojo.validate.isValidDate(this.textbox.value, this.flags.format);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:DateTextbox");


/*
  ****** TimeTextbox ******

  A subclass of ValidationTextbox.
  Over-rides isValid to test if input is in a valid time format.

  @attr format    Described in dojo.validate.js.  Default is  "h:mm:ss t".
  @attr amSymbol  The symbol used for AM.  Default is "AM" or "am".
  @attr pmSymbol  The symbol used for PM.  Default is "PM" or "pm".
*/
dojo.widget.validate.TimeTextbox = function(node) {
	this.flags = {};
}

dojo.inherits(dojo.widget.validate.TimeTextbox, dojo.widget.validate.ValidationTextbox);

dojo.lang.extend(dojo.widget.validate.TimeTextbox, {
	// new subclass properties
	widgetType: "TimeTextbox", 

	mixInProperties: function(localProperties, frag) {
		// First initialize properties in super-class.
		dojo.widget.validate.ValidationTextbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get properties from markup attibutes, and assign to flags object.
		if ( localProperties.format ) { 
			this.flags.format = localProperties.format;
		}
		if ( localProperties.amsymbol ) { 
			this.flags.amSymbol = localProperties.amsymbol;
		}
		if ( localProperties.pmsymbol ) { 
			this.flags.pmSymbol = localProperties.pmsymbol;
		}
	},

	// Over-ride for time validation
	isValid: function() { 
		return dojo.validate.isValidTime(this.textbox.value, this.flags);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:TimeTextbox");


/*
  ****** UsStateTextbox ******

  A subclass of ValidationTextbox.
  Over-rides isValid to test if input is a US state abbr.

  @attr allowTerritories  Allow Guam, Puerto Rico, etc.  Default is true.
  @attr allowMilitary     Allow military 'states', e.g. Armed Forces Europe (AE). Default is true.
*/
dojo.widget.validate.UsStateTextbox = function(node) {
	this.flags = {};
}

dojo.inherits(dojo.widget.validate.UsStateTextbox, dojo.widget.validate.ValidationTextbox);

dojo.lang.extend(dojo.widget.validate.UsStateTextbox, {
	// new subclass properties
	widgetType: "UsStateTextbox", 

	mixInProperties: function(localProperties, frag) {
		// Initialize properties in super-class.
		dojo.widget.validate.ValidationTextbox.prototype.mixInProperties.call(this, localProperties, frag);

		// Get properties from markup attibutes, and assign to flags object.
		if ( localProperties.allowterritories ) { 
			this.flags.allowTerritories = ( localProperties.allowterritories == "true" );
		}
		if ( localProperties.allowmilitary ) { 
			this.flags.allowMilitary = ( localProperties.allowmilitary == "true" );
		}
	},

	isValid: function() { 
		return dojo.validate.us.isState(this.textbox.value, this.flags);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:UsStateTextbox");


/*
  ****** UsZipTextbox ******

  A subclass of ValidationTextbox.
  Over-rides isValid to test if input is a US zip code.
  Validates zip-5 and zip-5 plus 4.
*/
dojo.widget.validate.UsZipTextbox = function(node) {}

dojo.inherits(dojo.widget.validate.UsZipTextbox, dojo.widget.validate.ValidationTextbox);

dojo.lang.extend(dojo.widget.validate.UsZipTextbox, {
	// new subclass properties
	widgetType: "UsZipTextbox", 

	mixInProperties: function(localProperties, frag) {
		// Initialize properties in super-class.
		dojo.widget.validate.ValidationTextbox.prototype.mixInProperties.call(this, localProperties, frag);
	},

	isValid: function() { 
		return dojo.validate.us.isZipCode(this.textbox.value);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:UsZipTextbox");


/*
  ****** UsSocialSecurityNumberTextbox ******

  A subclass of ValidationTextbox.
  Over-rides isValid to test if input is a US Social Security Number.
*/
dojo.widget.validate.UsSocialSecurityNumberTextbox = function(node) {}

dojo.inherits(dojo.widget.validate.UsSocialSecurityNumberTextbox, dojo.widget.validate.ValidationTextbox);

dojo.lang.extend(dojo.widget.validate.UsSocialSecurityNumberTextbox, {
	// new subclass properties
	widgetType: "UsSocialSecurityNumberTextbox", 

	mixInProperties: function(localProperties, frag) {
		// Initialize properties in super-class.
		dojo.widget.validate.ValidationTextbox.prototype.mixInProperties.call(this, localProperties, frag);
	},

	isValid: function() { 
		return dojo.validate.us.isSocialSecurityNumber(this.textbox.value);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:UsSocialSecurityNumberTextbox");


/*
  ****** UsPhoneNumberTextbox ******

  A subclass of ValidationTextbox.
  Over-rides isValid to test if input is a 10-digit US phone number, an extension is optional.
*/
dojo.widget.validate.UsPhoneNumberTextbox = function(node) {}

dojo.inherits(dojo.widget.validate.UsPhoneNumberTextbox, dojo.widget.validate.ValidationTextbox);

dojo.lang.extend(dojo.widget.validate.UsPhoneNumberTextbox, {
	// new subclass properties
	widgetType: "UsPhoneNumberTextbox", 

	mixInProperties: function(localProperties, frag) {
		// Initialize properties in super-class.
		dojo.widget.validate.ValidationTextbox.prototype.mixInProperties.call(this, localProperties, frag);
	},

	isValid: function() { 
		return dojo.validate.us.isPhoneNumber(this.textbox.value);
	}

});

dojo.widget.tags.addParseTreeHandler("dojo:UsPhoneNumberTextbox");
