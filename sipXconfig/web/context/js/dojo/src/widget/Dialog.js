/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.widget.Dialog");
dojo.provide("dojo.widget.HtmlDialog");

dojo.require("dojo.widget.*");
dojo.require("dojo.event.*");
dojo.require("dojo.graphics.color");
dojo.require("dojo.fx.*");
dojo.require("dojo.html");

dojo.widget.tags.addParseTreeHandler("dojo:dialog");

dojo.widget.HtmlDialog = function(){
	dojo.widget.HtmlWidget.call(this);
}

dojo.inherits(dojo.widget.HtmlDialog, dojo.widget.HtmlWidget);

dojo.lang.extend(dojo.widget.HtmlDialog, {
	templatePath: dojo.uri.dojoUri("src/widget/templates/HtmlDialog.html"),
	widgetType: "Dialog",
	isContainer: true,

	_scrollConnected: false,
	
	// provide a focusable element or element id if you need to
	// work around FF's tendency to send focus into outer space on hide
	focusElement: "",

	bg: null,
	bgIframe: null,
	bgColor: "black",
	bgOpacity: 0.4,
	followScroll: true,
	_fromTrap: false,
	anim: null,

	trapTabs: function(e){
		if(e.target == this.tabStart) {
			if(this._fromTrap) {
				this._fromTrap = false;
			} else {
				this._fromTrap = true;
				this.tabEnd.focus();
			}
		} else if(e.target == this.tabEnd) {
			if(this._fromTrap) {
				this._fromTrap = false;
			} else {
				this._fromTrap = true;
				this.tabStart.focus();
			}
		}
	},

	clearTrap: function(e) {
		var _this = this;
		setTimeout(function() {
			_this._fromTrap = false;
		}, 100);
	},

	postCreate: function(args, frag, parentComp) {
		with(this.domNode.style) {
			position = "absolute";
			zIndex = 999;
			display = "none";
		}
		var b = document.body;
		b.appendChild(this.domNode);

		this.bgIframe = new dojo.html.BackgroundIframe();

		this.bg = document.createElement("div");
		this.bg.className = "dialogUnderlay";
		with(this.bg.style) {
			position = "absolute";
			left = top = "0px";
			zIndex = 998;
			display = "none";
		}
		this.setBackgroundColor(this.bgColor);
		b.appendChild(this.bg);
		this.bgIframe.setZIndex(this.bg);
	},

	setBackgroundColor: function(color) {
		if(arguments.length >= 3) {
			color = new dojo.graphics.color.Color(arguments[0], arguments[1], arguments[2]);
		} else {
			color = new dojo.graphics.color.Color(color);
		}
		this.bg.style.backgroundColor = color.toString();
		return this.bgColor = color;
	},
	
	setBackgroundOpacity: function(op) {
		if(arguments.length == 0) { op = this.bgOpacity; }
		dojo.style.setOpacity(this.bg, op);
		try {
			this.bgOpacity = dojo.style.getOpacity(this.bg);
		} catch (e) {
			this.bgOpacity = op;
		}
		return this.bgOpacity;
	},

	sizeBackground: function() {
		if(this.bgOpacity > 0) {
			var h = document.documentElement.scrollHeight || document.body.scrollHeight;
			var w = dojo.html.getViewportWidth();
			this.bg.style.width = w + "px";
			this.bg.style.height = h + "px";
			this.bgIframe.size([0, 0, w, h]);
		} else {
			this.bgIframe.size(this.domNode);
		}
	},

	showBackground: function() {
		this.sizeBackground();
		this.bgIframe.show();
		if(this.bgOpacity > 0) {
			this.bg.style.display = "block";
		}
	},

	placeDialog: function() {
		var scroll_offset = dojo.html.getScrollOffset();
		var viewport_size = dojo.html.getViewportSize();

		// find the size of the dialog
		// we should really be using dojo.style but i'm not sure
		// which (inner, outer, box, content, client) --cal
		this.domNode.style.display = "block";
		var w = this.domNode.offsetWidth;
		var h = this.domNode.offsetHeight;
		this.domNode.style.display = "none";

		var x = scroll_offset[0] + (viewport_size[0] - w)/2;
		var y = scroll_offset[1] + (viewport_size[1] - h)/2;

		with(this.domNode.style) {
			left = x + "px";
			top = y + "px";
		}

		// tied to domNode, so we need to update the position
		if(this.bgOpacity == 0) {
			this.bgIframe.size([x, y, w, h]);
		}
	},

	show: function() {
		this.setBackgroundOpacity();
		this.placeDialog();
		this.showBackground();

		// resize child widgets
		this.domNode.style.display="block";
		dojo.widget.HtmlDialog.superclass.onResized.call(this);
		this.domNode.style.display="none";

		dojo.widget.HtmlDialog.superclass.show.call(this);

		// FIXME: moz doesn't generate onscroll events for mouse or key scrolling (wtf)
		// we should create a fake event by polling the scrolltop/scrollleft every X ms.
		// this smells like it should be a dojo feature rather than just for this widget.

		if (this.followScroll && !this._scrollConnected){
			this._scrollConnected = true;
			dojo.event.connect(window, "onscroll", this, "onScroll");
		}
	},

	hide: function(){
		// workaround for FF focus going into outer space
		if (this.focusElement) { 
			dojo.byId(this.focusElement).focus(); 
			dojo.byId(this.focusElement).blur();
		}

		this.bg.style.display = "none";
		this.bg.style.width = this.bg.style.height = "1px";
		this.bgIframe.hide();

		dojo.widget.HtmlDialog.superclass.hide.call(this);

		if (this._scrollConnected){
			this._scrollConnected = false;
			dojo.event.disconnect(window, "onscroll", this, "onScroll");
		}
	},

	setCloseControl: function(node) {
		dojo.event.connect(node, "onclick", this, "hide");
	},

	setShowControl: function(node) {
		dojo.event.connect(node, "onclick", this, "show");
	},

	onScroll: function(){
		this.placeDialog();
		this.domNode.style.display = "block";
	},

	onResized: function() {
		if(this.isVisible()){
			this.sizeBackground();
			this.placeDialog();
			this.domNode.style.display="block";
			dojo.widget.HtmlDialog.superclass.onResized.call(this);
		}
	}
});

