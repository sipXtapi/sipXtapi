/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.widget.HtmlWidget");
dojo.require("dojo.widget.DomWidget");
dojo.require("dojo.html");
dojo.require("dojo.lang.extras");
dojo.require("dojo.lang.func");

dojo.widget.HtmlWidget = function(args){
	// mixin inheritance
	dojo.widget.DomWidget.call(this);
}

dojo.inherits(dojo.widget.HtmlWidget, dojo.widget.DomWidget);

dojo.lang.extend(dojo.widget.HtmlWidget, {
	widgetType: "HtmlWidget",

	templateCssPath: null,
	templatePath: null,

	// for displaying/hiding widget
	toggle: "plain",
	toggleDuration: 150,

	animationInProgress: false,
	bgIframe: null,

	initialize: function(args, frag){
	},

	toggleObj: {
		show: function(node, duration, explodeSrc, callback){
			dojo.style.show(node);
			if(dojo.lang.isFunction(callback)){ callback(); }
		},
	
		hide: function(node, duration, explodeSrc, callback){
			dojo.style.hide(node);
			if(dojo.lang.isFunction(callback)){ callback(); }
		}
	},

	postMixInProperties: function(args, frag){
		// now that we know the setting for toggle, get toggle object
		// If user hasn't included dojo.fx.html then we fall back to plain toggle
		var specialToggler =
			(dojo.fx && dojo.fx.html && dojo.fx.html.toggle[this.toggle.toLowerCase()]);
		if(specialToggler){
			this.toggleObj=specialToggler;
		}
	},

	getContainerHeight: function(){
		// NOTE: container height must be returned as the INNER height
		dj_unimplemented("dojo.widget.HtmlWidget.getContainerHeight");
	},

	getContainerWidth: function(){
		return this.parent.domNode.offsetWidth;
	},

	setNativeHeight: function(height){
		var ch = this.getContainerHeight();
	},

	resizeSoon: function(){
		if(this.isShowing()){
			dojo.lang.setTimeout(this, this.onResized, 0);
		}
	},

	resizeTo: function(w, h){
		dojo.style.setOuterWidth(this.domNode, w);
		dojo.style.setOuterHeight(this.domNode, h);
		this.onResized();
	},

	createNodesFromText: function(txt, wrap){
		return dojo.html.createNodesFromText(txt, wrap);
	},

	destroyRendering: function(finalize){
		try{
			if(!finalize){
				dojo.event.browser.clean(this.domNode);
			}
			this.domNode.parentNode.removeChild(this.domNode);
			delete this.domNode;
		}catch(e){ /* squelch! */ }
	},

	// Displaying/hiding the widget

	isShowing: function(){
		return dojo.style.isShowing(this.domNode);
	},

	toggleShowing: function(){
		dojo.style.toggleShowing(this.domNode);
	},

	show: function(){
		this.animationInProgress=true;
		this.toggleObj.show(this.domNode, this.toggleDuration, this.explodeSrc,
			dojo.lang.hitch(this, this.onShow));
	},

	onShow: function(){
		this.animationInProgress=false;
	},

	hide: function(){
		this.animationInProgress=true;
		this.toggleObj.hide(this.domNode, this.toggleDuration, this.explodeSrc,
			dojo.lang.hitch(this, this.onHide));
	},

	onHide: function(){
		this.animationInProgress=false;
	},
	
	onResized: function(){
		dojo.widget.HtmlWidget.superclass.onResized.call(this);
		if(this.bgIframe){ this.bgIframe.onResized(); }
	}
});
