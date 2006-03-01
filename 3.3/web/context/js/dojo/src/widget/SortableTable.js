/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.widget.SortableTable");
dojo.require("dojo.widget.*");
dojo.requireAfterIf("html", "dojo.widget.html.SortableTable");
dojo.widget.tags.addParseTreeHandler("dojo:sortableTable");

//	set up the general widget
dojo.widget.SortableTable=function(){
	dojo.widget.Widget.call(this);
	this.widgetType="SortableTable";
	this.isContainer=false;

	//	custom properties
	this.enableMultipleSelect=false;
	this.maximumNumberOfSelections=1;
	this.enableAlternateRows=false;
	this.minRows=0;	//	0 means ignore.
	this.defaultDateFormat="#M/#d/#yyyy";
	this.data=[];
	this.selected=null
	this.columns=[];
	this.sortIndex=0;		//	index of the column sorted on, first is the default.
	this.sortDirection=0;	//	0==asc, 1==desc
	this.sortFunctions={};	//	you can add to this if needed.
};
dojo.inherits(dojo.widget.SortableTable, dojo.widget.Widget);
