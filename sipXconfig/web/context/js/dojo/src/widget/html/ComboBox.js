/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.widget.html.ComboBox");
dojo.require("dojo.widget.ComboBox");
dojo.require("dojo.widget.*");
dojo.require("dojo.io.*");
dojo.require("dojo.fx.*");
dojo.require("dojo.dom");
dojo.require("dojo.html");
dojo.require("dojo.string");
dojo.require("dojo.widget.html.stabile");

dojo.widget.html.ComboBox = function(){
	dojo.widget.ComboBox.call(this);
	dojo.widget.HtmlWidget.call(this);

	this.autoComplete = true;
	this.formInputName = "";
	this.name = ""; // clone in the name from the DOM node
	this.textInputNode = null;
	this.comboBoxValue = null;
	this.comboBoxSelectionValue = null;
	this.optionsListWrapper = null;
	this.optionsListNode = null;
	this.downArrowNode = null;
	this.cbTableNode = null;
	this.searchTimer = null;
	this.searchDelay = 100;
	this.dataUrl = "";
	// mode can also be "remote" for JSON-returning live search or "html" for
	// dumber live search
	this.mode = "local"; 
	this.selectedResult = null;
	this._highlighted_option = null;
	this._prev_key_backspace = false;
	this._prev_key_esc = false;
	this._result_list_open = false;
	this._gotFocus = false;
	this._mousewheel_connected = false;

}

dojo.inherits(dojo.widget.html.ComboBox, dojo.widget.HtmlWidget);

// copied from superclass since we can't really over-ride via prototype
dojo.lang.extend(dojo.widget.html.ComboBox, dojo.widget.ComboBox.defaults);

dojo.lang.extend(dojo.widget.html.ComboBox, {


	templatePath: dojo.uri.dojoUri("src/widget/templates/HtmlComboBox.html"),
	templateCssPath: dojo.uri.dojoUri("src/widget/templates/HtmlComboBox.css"),

	setValue: function(value) {
		this.comboBoxValue.value = value;
		if (this.textInputNode.value != value) { // prevent mucking up of selection
			this.textInputNode.value = value;
		}
		dojo.widget.html.stabile.setState(this.widgetId, this.getState(), true);
	},

	getValue: function() {
		return this.comboBoxValue.value;
	},

	getState: function() {
		return {value: this.getValue()};
	},

	setState: function(state) {
		this.setValue(state.value);
	},


	getCaretPos: function(element){
		// khtml 3.5.2 has selection* methods as does webkit nightlies from 2005-06-22
		if(dojo.lang.isNumber(element.selectionStart)){
			// FIXME: this is totally borked on Moz < 1.3. Any recourse?
			return element.selectionStart;
		}else if(dojo.render.html.ie){
			// in the case of a mouse click in a popup being handled,
			// then the document.selection is not the textarea, but the popup
			// var r = document.selection.createRange();
			// hack to get IE 6 to play nice. What a POS browser.
			var tr = document.selection.createRange().duplicate();
			var ntr = element.createTextRange();
			tr.move("character",0);
			ntr.move("character",0);
			try {
				// If control doesnt have focus, you get an exception.
				// Seems to happen on reverse-tab, but can also happen on tab (seems to be a race condition - only happens sometimes).
				// There appears to be no workaround for this - googled for quite a while.
				ntr.setEndPoint("EndToEnd", tr);
				return String(ntr.text).replace(/\r/g,"").length;
			} catch (e) {
				return 0; // If focus has shifted, 0 is fine for caret pos.
			}
			
		}
	},

	setCaretPos: function(element, location){
		location = parseInt(location);
		this.setSelectedRange(element, location, location);
	},

	setSelectedRange: function(element, start, end){
		if(!end){ end = element.value.length; }  // NOTE: Strange - should be able to put caret at start of text?
		// Mozilla
		// parts borrowed from http://www.faqts.com/knowledge_base/view.phtml/aid/13562/fid/130
		if(element.setSelectionRange){
			element.focus();
			element.setSelectionRange(start, end);
		}else if(element.createTextRange){ // IE
			var range = element.createTextRange();
			with(range){
				collapse(true);
				moveEnd('character', end);
				moveStart('character', start);
				select();
			}
		}else{ //otherwise try the event-creation hack (our own invention)
			// do we need these?
			element.value = element.value;
			element.blur();
			element.focus();
			// figure out how far back to go
			var dist = parseInt(element.value.length)-end;
			var tchar = String.fromCharCode(37);
			var tcc = tchar.charCodeAt(0);
			for(var x = 0; x < dist; x++){
				var te = document.createEvent("KeyEvents");
				te.initKeyEvent("keypress", true, true, null, false, false, false, false, tcc, tcc);
				twe.dispatchEvent(te);
			}
		}
	},

	onKeyDown: function(evt){
		if (evt.ctrlKey || evt.altKey) {
			return;
		}
		// These need to be detected in onKeyDown so that the events can be cancelled.
		// Perhaps use dojo.event.browser.keys.KEY_ENTER etc? 
		if(this._result_list_open && ((evt.keyCode == 32)||(evt.keyCode == 13))){ // space is 32, enter is 13.
			this.selectOption();
			dojo.event.browser.stopEvent(evt);
			return;
		}else if(evt.keyCode == 40){ // down is 40
			if(!this._result_list_open){
				this.startSearchFromInput();
			}
			this.highlightNextOption();
			dojo.event.browser.stopEvent(evt);
			return;
		}else if(evt.keyCode == 38){ // up is 38
			this.highlightPrevOption();
			dojo.event.browser.stopEvent(evt);
			return;
		}
	},

	onKeyUp: function(evt){
		if (evt.ctrlKey || evt.altKey) {
			return;
		}
		if((evt.keyCode == 32)||(evt.keyCode == 13)||(evt.keyCode == 40)||(evt.keyCode == 38)){
			return;
		}
		// FIXME Everything below here should really be moved into onKeyDown?
		if(evt.keyCode == 27){ // esc is 27
			this.hideResultList();
			if(this._prev_key_esc){
				// FIXME - probably get rid of?
				// Shouldnt deselect if something was selected on entry to form. Also it doesnt clear the input text for IE.
				// And shouldnt lose focus - users are likely to press escape more than once when they want to cancel something.
				/* 
				this.textInputNode.blur();
				this.selectedResult = null;
				*/
			}
			this._prev_key_esc = true;
			return;
		}else{
			this.setValue(this.textInputNode.value);
		}

		// backspace is 8
		this._prev_key_backspace = (evt.keyCode == 8) ? true : false;
		this._prev_key_esc = false;

		if(this.searchTimer){
			clearTimeout(this.searchTimer);
		}
		if((this._prev_key_backspace)&&(!this.textInputNode.value.length)){
			this.hideResultList();
		}else{
			// FIXME This is untidy. Occurs when tabbing in to a control gives a keyup event and maybe other wierd keys too?
			this.searchTimer = setTimeout(dojo.lang.hitch(this, this.startSearchFromInput), this.searchDelay);
		}
	},

	setSelectedValue: function(value){
		// FIXME, not sure what to do here!
		this.comboBoxSelectionValue.value = value;
		this.hideResultList();
	},

	highlightNextOption: function(){
		if(this._highlighted_option){
			dojo.html.removeClass(this._highlighted_option, "cbItemHighlight");
		}
		if((!this._highlighted_option)||(!this._highlighted_option.nextSibling)){
			this._highlighted_option = this.optionsListNode.firstChild;
		}else{
			this._highlighted_option = this._highlighted_option.nextSibling;
		}
		dojo.html.addClass(this._highlighted_option, "cbItemHighlight");
	},

	highlightPrevOption: function(){
		if(this._highlighted_option){
			dojo.html.removeClass(this._highlighted_option, "cbItemHighlight");
		}
		if((!this._highlighted_option)||(!this._highlighted_option.previousSibling)){
			this._highlighted_option = this.optionsListNode.lastChild;
		}else{
			this._highlighted_option = this._highlighted_option.previousSibling;
		}
		dojo.html.addClass(this._highlighted_option, "cbItemHighlight");
	},

	onMouseWheel: function(evt) {
		if (this._hasFocus) {
			if (evt.wheelDelta < 0) {
				if(!this._result_list_open){
					this.startSearchFromInput();
				}
				this.highlightNextOption();
			} else if (evt.wheelDelta > 0) {
				this.highlightPrevOption();
			}
			evt.preventDefault(); // so don't scroll window
			return false;
		};
	},
  
	fillInTemplate: function(args, frag){
		// FIXME: need to get/assign DOM node names for form participation here.
		this.comboBoxValue.name = this.name;
		this.comboBoxSelectionValue.name = this.name+"_selected";
		
		// NOTE: this doesn't copy style info inherited from classes;
		// it's just primitive support for direct style setting
		var sourceNodeStyle = this.getFragNodeRef(frag).style;
		if ( sourceNodeStyle ){
			this.domNode.style.cssText = sourceNodeStyle.cssText;
		}

		// FIXME: add logic
		this.dataProvider = new dojo.widget.ComboBoxDataProvider();

		if(!dojo.string.isBlank(this.dataUrl)){
			if("local" == this.mode){
				var _this = this;
				dojo.io.bind({
					url: this.dataUrl,
					load: function(type, data, evt){ 
						if(type=="load"){
							if(!dojo.lang.isArray(data)){
								var arrData = [];
								for(var key in data){
									arrData.push([data[key], key]);
								}
								data = arrData;
							}
							_this.dataProvider.setData(data);
						}
					},
					mimetype: "text/json"
				});
			}else if("remote" == this.mode){
				this.dataProvider = new dojo.widget.incrementalComboBoxDataProvider(this.dataUrl);
			}
		}else{
			// check to see if we can populate the list from <option> elements
			var node = frag["dojo:"+this.widgetType.toLowerCase()]["nodeRef"];
			if((node)&&(node.nodeName.toLowerCase() == "select")){
				// NOTE: we're not handling <optgroup> here yet
				var opts = node.getElementsByTagName("option");
				var ol = opts.length;
				var data = [];
				for(var x=0; x<ol; x++){
					data.push([new String(opts[x].innerHTML), new String(opts[x].value)]);
				}
				this.dataProvider.setData(data);
			}
		}

		// Prevent IE bleed-through problem
		this.optionsIframe = new dojo.html.BackgroundIframe(this.optionsListWrapper);
		this.optionsIframe.size([0,0,0,0]);
	},

	openResultList: function(results){
		this.clearResultList();
		if(!results.length){
			this.hideResultList();
		}else{
			this.showResultList();
		}
		if(	(this.autoComplete)&&
			(results.length)&&
			(!this._prev_key_backspace)&&
			(this.textInputNode.value.length > 0)){
			var cpos = this.getCaretPos(this.textInputNode);
			// only try to extend if we added the last charachter at the end of the input
			if((cpos+1) >= this.textInputNode.value.length){
				this.textInputNode.value = results[0][0];
				// build a new range that has the distance from the earlier
				// caret position to the end of the first string selected
				this.setSelectedRange(this.textInputNode, cpos, this.textInputNode.value.length);
			}
		}

		var even = true;
		while(results.length){
			var tr = results.shift();
			if(tr){
				var td = document.createElement("div");
				td.appendChild(document.createTextNode(tr[0]));
				td.setAttribute("resultName", tr[0]);
				td.setAttribute("resultValue", tr[1]);
				td.className = "cbItem "+((even) ? "cbItemEven" : "cbItemOdd");
				even = (!even);
				this.optionsListNode.appendChild(td);
			}
		}

		// prevent IE bleed through
		dojo.lang.setTimeout(this, "sizeBackgroundIframe", 100);
	},

	onFocusInput: function(){
		this._hasFocus = true;
		if (dojo.render.html.ie && !this._mousewheel_connected) {
			dojo.event.connect(document, "onmousewheel", this, "onMouseWheel");
			this._mousewheel_connected = true;
		}
	},

	onBlurInput: function(){
		this._hasFocus = false;
		if(this.blurTimer){
			clearTimeout(this.blurTimer);
		}
		this.blurTimer = dojo.lang.setTimeout(this, "checkBlurred", 100);
	},

	checkBlurred: function(){
		if (!this._hasFocus) {
			this.hideResultList();
		}
		if (this._mousewheel_connected) {
			dojo.event.disconnect(document, "onmousewheel", this, "onMouseWheel");
			this._mousewheel_connected = false;
		}
	},

	sizeBackgroundIframe: function(){
		var w = dojo.style.getOuterWidth(this.optionsListNode);
		var h = dojo.style.getOuterHeight(this.optionsListNode);
		if ( w==0 || h==0 ){
			// need more time to calculate size
			dojo.lang.setTimeout(this, "sizeBackgroundIframe", 100);
			return;
		}
		this.optionsIframe.size([0,0,w,h]);
	},

	selectOption: function(evt){
		if(!evt){
			evt = { target: this._highlighted_option };
		}

		if(!dojo.dom.isDescendantOf(evt.target, this.optionsListNode)){
			return;
		}

		var tgt = evt.target;
		while((tgt.nodeType!=1)||(!tgt.getAttribute("resultName"))){
			tgt = tgt.parentNode;
			if(tgt === document.body){
				return false;
			}
		}

		this.textInputNode.value = tgt.getAttribute("resultName");
		this.selectedResult = [tgt.getAttribute("resultName"), tgt.getAttribute("resultValue")];
		this.setValue(tgt.getAttribute("resultName"));
		this.comboBoxSelectionValue.value = tgt.getAttribute("resultValue");
		this.hideResultList();
		this.setSelectedRange(this.textInputNode, 0, null);
		this.tryFocus();
	},

	clearResultList: function(){
		var oln = this.optionsListNode;
		while(oln.firstChild){
			oln.removeChild(oln.firstChild);
		}
	},

	hideResultList: function(){
		if (this._result_list_open) {
			this._result_list_open = false;
			dojo.fx.fadeHide(this.optionsListNode, 200);
			this.optionsIframe.size([0,0,0,0]);
		}
		return;
	},

	showResultList: function(){
		if(this._result_list_open){
		  return;
		}
		this._result_list_open = true;
		with(this.optionsListNode.style){
			display = "";
			// visibility = "hidden";
			height = "";
			width = dojo.html.getInnerWidth(this.downArrowNode)+dojo.html.getInnerWidth(this.textInputNode)+"px";
		}
		dojo.html.setOpacity(this.optionsListNode, 0);
		dojo.fx.fadeIn(this.optionsListNode, 200);
	},

	handleArrowClick: function(){
		this.tryFocus();
		if(this._result_list_open){
			this.hideResultList();
		}else{
			this.startSearchFromInput();
		}
	},

	tryFocus: function(){
		try {    
			this.textInputNode.focus();
		} catch (e) {
			// element isnt focusable if disabled, or not visible etc - not easy to test for.
 		};
	},
	
	startSearchFromInput: function(){
		this.startSearch(this.textInputNode.value);
	},

	postCreate: function(){
		dojo.event.connect(this, "startSearch", this.dataProvider, "startSearch");
		dojo.event.connect(this.dataProvider, "provideSearchResults", this, "openResultList");
		dojo.event.connect(this.textInputNode, "onblur", this, "onBlurInput");
		dojo.event.connect(this.textInputNode, "onfocus", this, "onFocusInput");

		var s = dojo.widget.html.stabile.getState(this.widgetId);
		if (s) {
			this.setState(s);
		}
	}

});
