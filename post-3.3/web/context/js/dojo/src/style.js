/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.style");
dojo.require("dojo.graphics.color");

// values: content-box, border-box
dojo.style.boxSizing = {
	marginBox: "margin-box",
	borderBox: "border-box",
	paddingBox: "padding-box",
	contentBox: "content-box"
};

dojo.style.getBoxSizing = function(node) {
	node = dojo.byId(node);
	if (dojo.render.html.ie || dojo.render.html.opera){ 
		var cm = document["compatMode"];
		if (cm == "BackCompat" || cm == "QuirksMode"){ 
			return dojo.style.boxSizing.borderBox; 
		}else{
			return dojo.style.boxSizing.contentBox; 
		}
	}else{
		if(arguments.length == 0){ node = document.documentElement; }
		var sizing = dojo.style.getStyle(node, "-moz-box-sizing");
		if(!sizing){ sizing = dojo.style.getStyle(node, "box-sizing"); }
		return (sizing ? sizing : dojo.style.boxSizing.contentBox);
	}
}

/*

The following several function use the dimensions shown below

    +-------------------------+
    |  margin                 |
    | +---------------------+ |
    | |  border             | |
    | | +-----------------+ | |
    | | |  padding        | | |
    | | | +-------------+ | | |
    | | | |   content   | | | |
    | | | +-------------+ | | |
    | | +-|-------------|-+ | |
    | +-|-|-------------|-|-+ |
    +-|-|-|-------------|-|-|-+
    | | | |             | | | |
    | | | |<- content ->| | | |
    | |<------ inner ------>| |
    |<-------- outer -------->|
    +-------------------------+

    * content-box

    |m|b|p|             |p|b|m|
    | |<------ offset ----->| |
    | | |<---- client --->| | |
    | | | |<-- width -->| | | |

    * border-box

    |m|b|p|             |p|b|m|
    | |<------ offset ----->| |
    | | |<---- client --->| | |
    | |<------ width ------>| |
*/

/*
	Notes:

	General:
		- Uncomputable values are returned as NaN.
		- setOuterWidth/Height return *false* if the outer size could not be computed, otherwise *true*.
		- I (sjmiles) know no way to find the calculated values for auto-margins. 
		- All returned values are floating point in 'px' units. If a non-zero computed style value is not specified in 'px', NaN is returned.

	FF:
		- styles specified as '0' (unitless 0) show computed as '0pt'.

	IE:
		- clientWidth/Height are unreliable (0 unless the object has 'layout').
		- margins must be specified in px, or 0 (in any unit) for any sizing function to work. Otherwise margins detect as 'auto'.
		- padding can be empty or, if specified, must be in px, or 0 (in any unit) for any sizing function to work.

	Safari:
		- Safari defaults padding values to 'auto'.

	See the unit tests for examples of (un)computable values in a given browser.

*/

// FIXME: these work for most elements (e.g. DIV) but not all (e.g. TEXTAREA)

dojo.style.isBorderBox = function(node) {
	return (dojo.style.getBoxSizing(node) == dojo.style.boxSizing.borderBox);
}

dojo.style.getUnitValue = function (node, cssSelector, autoIsZero){
	node = dojo.byId(node);
	var result = { value: 0, units: 'px' };
	var s = dojo.style.getComputedStyle(node, cssSelector);
	if (s == '' || (s == 'auto' && autoIsZero)){ return result; }
	if (dojo.lang.isUndefined(s)){ 
		result.value = NaN;
	}else{
		// FIXME: is regex inefficient vs. parseInt or some manual test? 
		var match = s.match(/(\-?[\d.]+)([a-z%]*)/i);
		if (!match){
			result.value = NaN;
		}else{
			result.value = Number(match[1]);
			result.units = match[2].toLowerCase();
		}
	}
	return result;		
}

dojo.style.getPixelValue = function (node, cssSelector, autoIsZero){
	node = dojo.byId(node);
	var result = dojo.style.getUnitValue(node, cssSelector, autoIsZero);
	// FIXME: there is serious debate as to whether or not this is the right solution
	if(isNaN(result.value)){ return 0; }
	// FIXME: code exists for converting other units to px (see Dean Edward's IE7) 
	// but there are cross-browser complexities
	if((result.value)&&(result.units != 'px')){ return NaN; }
	return result.value;
}

dojo.style.getNumericStyle = dojo.style.getPixelValue; // backward compat

dojo.style.isPositionAbsolute = function(node){
	node = dojo.byId(node);
	return (dojo.style.getComputedStyle(node, 'position') == 'absolute');
}

dojo.style.getMarginWidth = function(node){
	node = dojo.byId(node);
	var autoIsZero = dojo.style.isPositionAbsolute(node);
	var left = dojo.style.getPixelValue(node, "margin-left", autoIsZero);
	var right = dojo.style.getPixelValue(node, "margin-right", autoIsZero);
	return left + right;
}

dojo.style.getBorderWidth = function(node){
	node = dojo.byId(node);
	var left = (dojo.style.getStyle(node, 'border-left-style') == 'none' ? 0 : dojo.style.getPixelValue(node, "border-left-width"));
	var right = (dojo.style.getStyle(node, 'border-right-style') == 'none' ? 0 : dojo.style.getPixelValue(node, "border-right-width"));
	return left + right;
}

dojo.style.getPaddingWidth = function(node){
	node = dojo.byId(node);
	var left = dojo.style.getPixelValue(node, "padding-left", true);
	var right = dojo.style.getPixelValue(node, "padding-right", true);
	return left + right;
}

dojo.style.getContentWidth = function (node){
	node = dojo.byId(node);
	return node.offsetWidth - dojo.style.getPaddingWidth(node) - dojo.style.getBorderWidth(node);
}

dojo.style.getInnerWidth = function (node){
	node = dojo.byId(node);
	return node.offsetWidth;
}

dojo.style.getOuterWidth = function (node){
	node = dojo.byId(node);
	return dojo.style.getInnerWidth(node) + dojo.style.getMarginWidth(node);
}

dojo.style.setOuterWidth = function (node, pxWidth){
	node = dojo.byId(node);
	if (!dojo.style.isBorderBox(node)){
		pxWidth -= dojo.style.getPaddingWidth(node) + dojo.style.getBorderWidth(node);
	}
	pxWidth -= dojo.style.getMarginWidth(node);
	if (!isNaN(pxWidth) && pxWidth > 0){
		node.style.width = pxWidth + 'px';
		return true;
	}else return false;
}

// FIXME: these aliases are actually the preferred names
dojo.style.getContentBoxWidth = dojo.style.getContentWidth;
dojo.style.getBorderBoxWidth = dojo.style.getInnerWidth;
dojo.style.getMarginBoxWidth = dojo.style.getOuterWidth;
dojo.style.setMarginBoxWidth = dojo.style.setOuterWidth;

dojo.style.getMarginHeight = function(node){
	node = dojo.byId(node);
	var autoIsZero = dojo.style.isPositionAbsolute(node);
	var top = dojo.style.getPixelValue(node, "margin-top", autoIsZero);
	var bottom = dojo.style.getPixelValue(node, "margin-bottom", autoIsZero);
	return top + bottom;
}

dojo.style.getBorderHeight = function(node){
	node = dojo.byId(node);
	var top = (dojo.style.getStyle(node, 'border-top-style') == 'none' ? 0 : dojo.style.getPixelValue(node, "border-top-width"));
	var bottom = (dojo.style.getStyle(node, 'border-bottom-style') == 'none' ? 0 : dojo.style.getPixelValue(node, "border-bottom-width"));
	return top + bottom;
}

dojo.style.getPaddingHeight = function(node){
	node = dojo.byId(node);
	var top = dojo.style.getPixelValue(node, "padding-top", true);
	var bottom = dojo.style.getPixelValue(node, "padding-bottom", true);
	return top + bottom;
}

dojo.style.getContentHeight = function (node){
	node = dojo.byId(node);
	return node.offsetHeight - dojo.style.getPaddingHeight(node) - dojo.style.getBorderHeight(node);
}

dojo.style.getInnerHeight = function (node){
	node = dojo.byId(node);
	return node.offsetHeight; // FIXME: does this work?
}

dojo.style.getOuterHeight = function (node){
	node = dojo.byId(node);
	return dojo.style.getInnerHeight(node) + dojo.style.getMarginHeight(node);
}

dojo.style.setOuterHeight = function (node, pxHeight){
	node = dojo.byId(node);
	if (!dojo.style.isBorderBox(node)){
		pxHeight -= dojo.style.getPaddingHeight(node) + dojo.style.getBorderHeight(node);
	}
	pxHeight -= dojo.style.getMarginHeight(node);
	if (!isNaN(pxHeight) && pxHeight > 0){
		node.style.height = pxHeight + 'px';
		return true;
	}else return false;
}

dojo.style.setContentWidth = function(node, pxWidth){
	node = dojo.byId(node);
	if (dojo.style.isBorderBox(node)){
		pxWidth += dojo.style.getPaddingWidth(node) + dojo.style.getBorderWidth(node);
	}

	if (!isNaN(pxWidth) && pxWidth > 0){
		node.style.width = pxWidth + 'px';
		return true;
	}else return false;
}

dojo.style.setContentHeight = function(node, pxHeight){
	node = dojo.byId(node);
	if (dojo.style.isBorderBox(node)){
		pxHeight += dojo.style.getPaddingHeight(node) + dojo.style.getBorderHeight(node);
	}

	if (!isNaN(pxHeight) && pxHeight > 0){
		node.style.height = pxHeight + 'px';
		return true;
	}else return false;
}

// FIXME: these aliases are actually the preferred names
dojo.style.getContentBoxHeight = dojo.style.getContentHeight;
dojo.style.getBorderBoxHeight = dojo.style.getInnerHeight;
dojo.style.getMarginBoxHeight = dojo.style.getOuterHeight;
dojo.style.setMarginBoxHeight = dojo.style.setOuterHeight;

dojo.style.getTotalOffset = function (node, type, includeScroll){
	node = dojo.byId(node);
	var typeStr = (type=="top") ? "offsetTop" : "offsetLeft";
	var typeScroll = (type=="top") ? "scrollTop" : "scrollLeft";
	
	var coord = (type=="top") ? "y" : "x";
	var offset = 0;
	if(node["offsetParent"]){
		
		// in Safari, if the node is an absolutly positioned child of the body
		// and the body has a margin the offset of the child and the body
		// contain the body's margins, so we need to end at the body
		if (dojo.render.html.safari
			&& node.style.getPropertyValue("position") == "absolute"
			&& node.parentNode == document.body)
		{
			var endNode = document.body;
		} else {
			var endNode = document.body.parentNode;
		}
		
		if(includeScroll && node.parentNode != document.body) {
			offset -= dojo.style.sumAncestorProperties(node, typeScroll);
		}
		// FIXME: this is known not to work sometimes on IE 5.x since nodes
		// soemtimes need to be "tickled" before they will display their
		// offset correctly
		do {
			var n = node[typeStr];
			offset += isNaN(n) ? 0 : n;
			node = node.offsetParent;
		} while (node != endNode && node != null);
	}else if(node[coord]){
		var n = node[coord];
		offset += isNaN(n) ? 0 : n;
	}
	return offset;
}

dojo.style.sumAncestorProperties = function (node, prop) {
	node = dojo.byId(node);
	if (!node) { return 0; } // FIXME: throw an error?
	
	var retVal = 0;
	while (node) {
		var val = node[prop];
		if (val) {
			retVal += val - 0;
		}
		node = node.parentNode;
	}
	return retVal;
}

dojo.style.totalOffsetLeft = function (node, includeScroll){
	node = dojo.byId(node);
	return dojo.style.getTotalOffset(node, "left", includeScroll);
}

dojo.style.getAbsoluteX = dojo.style.totalOffsetLeft;

dojo.style.totalOffsetTop = function (node, includeScroll){
	node = dojo.byId(node);
	return dojo.style.getTotalOffset(node, "top", includeScroll);
}

dojo.style.getAbsoluteY = dojo.style.totalOffsetTop;

dojo.style.getAbsolutePosition = function(node, includeScroll) {
	node = dojo.byId(node);
	var position = [
		dojo.style.getAbsoluteX(node, includeScroll),
		dojo.style.getAbsoluteY(node, includeScroll)
	];
	position.x = position[0];
	position.y = position[1];
	return position;
}

dojo.style.styleSheet = null;

// FIXME: this is a really basic stub for adding and removing cssRules, but
// it assumes that you know the index of the cssRule that you want to add 
// or remove, making it less than useful.  So we need something that can 
// search for the selector that you you want to remove.
dojo.style.insertCssRule = function (selector, declaration, index) {
	if (!dojo.style.styleSheet) {
		if (document.createStyleSheet) { // IE
			dojo.style.styleSheet = document.createStyleSheet();
		} else if (document.styleSheets[0]) { // rest
			// FIXME: should create a new style sheet here
			// fall back on an exsiting style sheet
			dojo.style.styleSheet = document.styleSheets[0];
		} else { return null; } // fail
	}

	if (arguments.length < 3) { // index may == 0
		if (dojo.style.styleSheet.cssRules) { // W3
			index = dojo.style.styleSheet.cssRules.length;
		} else if (dojo.style.styleSheet.rules) { // IE
			index = dojo.style.styleSheet.rules.length;
		} else { return null; } // fail
	}

	if (dojo.style.styleSheet.insertRule) { // W3
		var rule = selector + " { " + declaration + " }";
		return dojo.style.styleSheet.insertRule(rule, index);
	} else if (dojo.style.styleSheet.addRule) { // IE
		return dojo.style.styleSheet.addRule(selector, declaration, index);
	} else { return null; } // fail
}

dojo.style.removeCssRule = function (index){
	if(!dojo.style.styleSheet){
		dojo.debug("no stylesheet defined for removing rules");
		return false;
	}
	if(dojo.render.html.ie){
		if(!index){
			index = dojo.style.styleSheet.rules.length;
			dojo.style.styleSheet.removeRule(index);
		}
	}else if(document.styleSheets[0]){
		if(!index){
			index = dojo.style.styleSheet.cssRules.length;
		}
		dojo.style.styleSheet.deleteRule(index);
	}
	return true;
}

dojo.style.getBackgroundColor = function (node) {
	node = dojo.byId(node);
	var color;
	do{
		color = dojo.style.getStyle(node, "background-color");
		// Safari doesn't say "transparent"
		if(color.toLowerCase() == "rgba(0, 0, 0, 0)") { color = "transparent"; }
		if(node == document.getElementsByTagName("body")[0]) { node = null; break; }
		node = node.parentNode;
	}while(node && dojo.lang.inArray(color, ["transparent", ""]));

	if( color == "transparent" ) {
		color = [255, 255, 255, 0];
	} else {
		color = dojo.graphics.color.extractRGB(color);
	}
	return color;
}

dojo.style.getComputedStyle = function (node, cssSelector, inValue) {
	node = dojo.byId(node);
	var value = inValue;
	if (node.style.getPropertyValue) { // W3
		value = node.style.getPropertyValue(cssSelector);
	}
	if(!value) {
		if (document.defaultView) { // gecko
			var cs = document.defaultView.getComputedStyle(node, "");
			if (cs) { 
				value = cs.getPropertyValue(cssSelector);
			} 
		} else if (node.currentStyle) { // IE
			value = node.currentStyle[dojo.style.toCamelCase(cssSelector)];
		} 
	}
	
	return value;
}

dojo.style.getStyle = function (node, cssSelector) {
	node = dojo.byId(node);
	var camelCased = dojo.style.toCamelCase(cssSelector);
	var value = node.style[camelCased]; // dom-ish
	return (value ? value : dojo.style.getComputedStyle(node, cssSelector, value));
}

dojo.style.toCamelCase = function (selector) {
	var arr = selector.split('-'), cc = arr[0];
	for(var i = 1; i < arr.length; i++) {
		cc += arr[i].charAt(0).toUpperCase() + arr[i].substring(1);
	}
	return cc;		
}

dojo.style.toSelectorCase = function (selector) {
	return selector.replace(/([A-Z])/g, "-$1" ).toLowerCase() ;
}

/* float between 0.0 (transparent) and 1.0 (opaque) */
dojo.style.setOpacity = function setOpacity (node, opacity, dontFixOpacity) {
	node = dojo.byId(node);
	var h = dojo.render.html;
	if(!dontFixOpacity){
		if( opacity >= 1.0){
			if(h.ie){
				dojo.style.clearOpacity(node);
				return;
			}else{
				opacity = 0.999999;
			}
		}else if( opacity < 0.0){ opacity = 0; }
	}
	if(h.ie){
		if(node.nodeName.toLowerCase() == "tr"){
			// FIXME: is this too naive? will we get more than we want?
			var tds = node.getElementsByTagName("td");
			for(var x=0; x<tds.length; x++){
				tds[x].style.filter = "Alpha(Opacity="+opacity*100+")";
			}
		}
		node.style.filter = "Alpha(Opacity="+opacity*100+")";
	}else if(h.moz){
		node.style.opacity = opacity; // ffox 1.0 directly supports "opacity"
		node.style.MozOpacity = opacity;
	}else if(h.safari){
		node.style.opacity = opacity; // 1.3 directly supports "opacity"
		node.style.KhtmlOpacity = opacity;
	}else{
		node.style.opacity = opacity;
	}
}
	
dojo.style.getOpacity = function getOpacity (node){
	node = dojo.byId(node);
	if(dojo.render.html.ie){
		var opac = (node.filters && node.filters.alpha &&
			typeof node.filters.alpha.opacity == "number"
			? node.filters.alpha.opacity : 100) / 100;
	}else{
		var opac = node.style.opacity || node.style.MozOpacity ||
			node.style.KhtmlOpacity || 1;
	}
	return opac >= 0.999999 ? 1.0 : Number(opac);
}

dojo.style.clearOpacity = function clearOpacity (node) {
	node = dojo.byId(node);
	var h = dojo.render.html;
	if(h.ie){
		if( node.filters && node.filters.alpha ) {
			node.style.filter = ""; // FIXME: may get rid of other filter effects
		}
	}else if(h.moz){
		node.style.opacity = 1;
		node.style.MozOpacity = 1;
	}else if(h.safari){
		node.style.opacity = 1;
		node.style.KhtmlOpacity = 1;
	}else{
		node.style.opacity = 1;
	}
}

dojo.style.isVisible = function(node){
	node = dojo.byId(node);
	// FIXME: this should also look at visibility!
	return dojo.style.getComputedStyle(node||this.domNode, "display") != "none";
}

dojo.style.show  = function(node){
	node = dojo.byId(node);
	if(node.style){
		node.style.display = dojo.lang.inArray(['tr', 'td', 'th'], node.tagName.toLowerCase()) ? "" : "block";
	}
}

dojo.style.hide = function(node){
	node = dojo.byId(node);
	if(node.style){
		node.style.display = "none";
	}
}

dojo.style.toggleVisible = function(node) {
	if(dojo.style.isVisible(node)) {
		dojo.style.hide(node);
		return false;
	} else {
		dojo.style.show(node);
		return true;
	}
}

// in: coordinate array [x,y,w,h] or dom node
// return: coordinate array
dojo.style.toCoordinateArray = function(coords, includeScroll) {
	if(dojo.lang.isArray(coords)){
		// coords is already an array (of format [x,y,w,h]), just return it
		while ( coords.length < 4 ) { coords.push(0); }
		while ( coords.length > 4 ) { coords.pop(); }
		var ret = coords;
	} else {
		// coords is an dom object (or dom object id); return it's coordinates
		var node = dojo.byId(coords);
		var ret = [
			dojo.style.getAbsoluteX(node, includeScroll),
			dojo.style.getAbsoluteY(node, includeScroll),
			dojo.style.getInnerWidth(node),
			dojo.style.getInnerHeight(node)
		];
	}
	ret.x = ret[0];
	ret.y = ret[1];
	ret.w = ret[2];
	ret.h = ret[3];
	return ret;
};
