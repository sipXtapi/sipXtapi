/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.widget.html.Container");

dojo.require("dojo.widget.*");
dojo.require("dojo.widget.Container");

dojo.deprecated("dojo.widget.Container",  "just set isContainer:true", "0.4");

dojo.widget.html.Container = function(){
	dojo.widget.HtmlWidget.call(this);
}

dojo.inherits(dojo.widget.html.Container, dojo.widget.HtmlWidget);

dojo.lang.extend(dojo.widget.html.Container, {
	widgetType: "Container",

	isContainer: true
});

dojo.widget.tags.addParseTreeHandler("dojo:Container");
