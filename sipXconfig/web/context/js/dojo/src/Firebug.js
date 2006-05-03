/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.Firebug");

dojo.firebug = function(){}
dojo.firebug.printfire = function () {
	printfire=function(){}
	printfire.args = arguments;
	var ev = document.createEvent("Events");
	ev.initEvent("printfire", false, true);
	dispatchEvent(ev);
}

if (document.createEvent) {
	dojo.hostenv.println=dojo.firebug.printfire;
}
