// ===================================================================
// Author: Matt Kruse <matt@mattkruse.com>
// WWW: http://www.mattkruse.com/
//
// NOTICE: You may use this code for any purpose, commercial or
// private, without any further permission from the author. You may
// remove this notice from your final code if you wish, however it is
// appreciated by the author if at least my web site address is kept.
//
// You may *NOT* re-distribute this code in any way except through its
// use. That means, you can include it in your product, or your web
// site, or any other form where the code is actually being used. You
// may not put the plain javascript up on your site for download or
// include it in your javascript libraries for download. Instead,
// please just point to my URL to ensure the most up-to-date versions
// of the files. Thanks.
// ===================================================================
/* 
DynamicOptionList.js
Author: Matt Kruse
Last modified: 6/5/01

DESCRIPTION: This library allows you to create dynamic select list contents,
when the options in a list depend on which item is selected in another list.
It correctly supports multiple selection lists, including maintaining the
selected state of dynamic options.

COMPATABILITY: Works with Netscape 4.x, 6.x, IE 4.x, 5.x.
Not guaranteed to work correctly in Netscape 3.x, or Opera 5 (current
version of Opera does not support new Option()! ).

USAGE:
// Create a new DynamicOptionList object
// Pass it the NAME of its select list, then the NAME of each list that it
// is dependent on. In this example, the contents of the SELECT list named
// "B" is being defined, and its contents depend on this value selected in
// list "A"

var listB = new DynamicOptionList("B","A");

// Add options to the list
// The contents of this list depend on the value selected in "A". So, the 
// first argument to this function is the value of "A" which these values
// correspond to. Then text/value pairs follow as the other arguments.
// In this example, if list "A" has "West" selected as its value, then the
// contents of list "B" will be "California" and "Washington".
// You can add as many options as you wish in this call, or you may separate
// it into just one option added per call to addOptions()

listB.addOptions("West","California","California","Washington","Washington");

// When the new list is generated, you can set which of the options will be
// the default selected value. The first argument to this function is the
// value of the parent list that this default corresponds to.
// In this example, if list "A" as "West" selected then list "B" will be
// populated with "California" as the default option.

listB.setDefaultOption("West","California");

// This example creates a third-level list
// The contents of list "C" depend on the values selected in both list "A"
// and list "B". So when the new DynamicOptionList object is created, it is
// passed both "A" and "B" as the parent select lists

var listC = new DynamicOptionList("C","A","B");

// The contents of list "C" depend on both "A" and "B". So when the contents
// of list "C" are defined, they must be given for each possible combination
// of the selected values of "A" and "B". This is done by combining the values
// with a delimiter character. The default delimiter is "|" - this may be
// changed by calling setDelimiter(value).
// In this example, list "C" will contain the options "Los Angeles" and 
// "San Diego" if the value of list "A" is "West" and the value of list "B" is
// "California". Similar style is used when defining the default option.

listC.addOptions("West|California","Los Angeles","Los Angeles","San Diego","San Diego");

// Once the lists are defined (presumably in the HEAD of your HTML document)
// then several changes need to be made in the HTML itself to trigger the
// population of the lists.

// Add calls in the onLoad of your BODY tag to initialize the dynamic lists.
// Pass a reference to the FORM that they belong to. This must be done onLoad
// of the document because before that point the FORM object does not exist.

<BODY onLoad="listB.init(document.forms[0]); listC.init(document.forms[0]);">

// In each of the parent select lists, add an onChange handler to trigger the
// population of the child lists. populate() must be called on each list that
// depends on this select element. Since list "B" depends on "A" and list "C"
// depends on both "A" and "B", both must be populated when "A" is changed.
// When list "B" is changed, only list "C" needs to be populated.

<SELECT NAME="A" onChange="listB.populate(); listC.populate();">
...
<SELECT NAME="B" onChange="listC.populate();">

// Netscape<6 does not create new Options correctly. If you have no OPTION 
// tags in your SELECT list, newly-created OPTIONS will not display correctly.
// Also, Netscape does not change the size of the SELECT list depending on its
// contents. So if it is empty by default and then is populated with options,
// it will not expand to fit the whole text in the option space.
// To work around these bugs, blank OPTION tags are generated for Netscape.
// The last OPTION tag is given display text equal to the longest possible 
// OPTION text value that the list will ever hold. This ensures that there will
// be enough room to display all possible values in the SELECT list.
// Call printOptions() inside the SELECT list to generate these blank OPTION
// tags. For browsers other than Netscape<=4.x, this will not do anything.
// NOTE: In this example, the SCRIPT and /SCRIPT are split up because
// otherwise Netscape gets confused. In your actual source, do NOT include the
// space between R and I.

<SELECT NAME="B" onChange="listC.populate();">
	<SCR IPT LANGUAGE="JavaScript">listB.printOptions();</SCR IPT>
</SELECT>

// That's it! That is all that is required to make the lists function.

NOTES:
	None

*/ 

// CONSTRUCTOR
// Pass in the name of the element, then the names of the lists it depends on
function DynamicOptionList() {
	if (arguments.length < 2) { alert("Not enough arguments in DynamicOptionList()"); }
	// Name of the list containing dynamic values
	this.target = arguments[0];
	// Set the lists that this dynamic list depends on
	this.dependencies = new Array();
	for (var i=1; i<arguments.length; i++) {
		this.dependencies[this.dependencies.length] = arguments[i];
		}
	// The form this list belongs to
	this.form = null;
	// Place-holder for currently-selected values of dependent select lists
	this.dependentValues = new Object();
	// Hold default values to be selected for conditions
	this.defaultValues = new Object();
	// Storage for the dynamic values
	this.options = new Object();
	// Delimiter between dependent values
	this.delimiter = "|";
	// Logest string currently a potential options (for Netscape)
	this.longestString = "";
	// The total number of options that might be displayed, to build dummy options (for Netscape)
	this.numberOfOptions = 0;
	// Method mappings
	this.addOptions = DynamicOptionList_addOptions;
	this.populate = DynamicOptionList_populate;
	this.setDelimiter = DynamicOptionList_setDelimiter;
	this.setDefaultOption = DynamicOptionList_setDefaultOption;
	this.printOptions = DynamicOptionList_printOptions;
	this.init = DynamicOptionList_init;
	}

// Set the delimiter to something other than | when defining condition values
function DynamicOptionList_setDelimiter(val) {
	this.delimiter = val;
	}

// Set the default option to be selected when the list is painted
function DynamicOptionList_setDefaultOption(condition, val) {
	this.defaultValues[condition] = val;
	}

// Init call to map the form to the object and populate it
function DynamicOptionList_init(theform) {
	this.form = theform;
	this.populate();
	}

// Add options to the list.
// Pass the condition string, then the list of text/value pairs that populate the list	
function DynamicOptionList_addOptions(dependentValue) {
	if (typeof this.options[dependentValue] != "object") { this.options[dependentValue] = new Array(); }
	for (var i=1; i<arguments.length; i+=2) {
		// Keep track of the longest potential string, to draw the option list
		if (arguments[i].length > this.longestString.length) {
			this.longestString = arguments[i];
			}
		this.numberOfOptions++;
		this.options[dependentValue][this.options[dependentValue].length] = arguments[i];
		this.options[dependentValue][this.options[dependentValue].length] = arguments[i+1];
		}
	}

// Print dummy options so Netscape behaves nicely
function DynamicOptionList_printOptions() {
	// Only need to write out "dummy" options for Netscape
    if ((navigator.appName == 'Netscape') && (parseInt(navigator.appVersion) <= 4)){
		var ret = "";
		for (var i=0; i<this.numberOfOptions; i++) { 
			ret += "<OPTION>";
			}
		ret += "<OPTION>"
		for (var i=0; i<this.longestString.length; i++) {
			ret += "_";
			}
		document.writeln(ret);
		}
	}

// Populate the list
function DynamicOptionList_populate() {
	var theform = this.form;
	var i,j,obj,obj2;
	// Get the current value(s) of all select lists this list depends on
	this.dependentValues = new Object;
	var dependentValuesInitialized = false;
	for (i=0; i<this.dependencies.length;i++) {
		var sel = theform[this.dependencies[i]];
		var selName = sel.name;
		// If this is the first dependent list, just fill in the dependentValues
		if (!dependentValuesInitialized) {
			dependentValuesInitialized = true;
			for (j=0; j<sel.options.length; j++) {
				if (sel.options[j].selected) {
					this.dependentValues[sel.options[j].value] = true;
					}
				}
			}
		// Otherwise, add new options for every existing option
		else {
			var tmpList = new Object();
			var newList = new Object();
			for (j=0; j<sel.options.length; j++) {
				if (sel.options[j].selected) {
					tmpList[sel.options[j].value] = true;
					}
				}
			for (obj in this.dependentValues) {
				for (obj2 in tmpList) {
					newList[obj + this.delimiter + obj2] = true;
					}
				}
			this.dependentValues = newList;
			}
		}

	var targetSel = theform[this.target];
		
	// Store the currently-selected values of the target list to maintain them (in case of multiple select lists)
	var targetSelected = new Object();
	for (i=0; i<targetSel.options.length; i++) {
		if (targetSel.options[i].selected) {
			targetSelected[targetSel.options[i].value] = true;
			}
		}

	targetSel.options.length = 0; // Clear all target options
		
	for (i in this.dependentValues) {
		if (typeof this.options[i] == "object") {
			var o = this.options[i];
			for (j=0; j<o.length; j+=2) {
				var text = o[j];
				var val = o[j+1];
				targetSel.options[targetSel.options.length] = new Option(text, val, false, false);
				if (this.defaultValues[i] == val) {
					targetSelected[val] = true;
					}
				}
			}
		}
	targetSel.selectedIndex=-1;
	
	// Select the options that were selected before
	for (i=0; i<targetSel.options.length; i++) {
		if (targetSelected[targetSel.options[i].value] != null && targetSelected[targetSel.options[i].value]==true) {
			targetSel.options[i].selected = true;
			}
		}
	}
