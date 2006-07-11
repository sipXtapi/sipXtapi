/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.widget.html.ContentPane");

dojo.require("dojo.widget.*");
dojo.require("dojo.io.*");
dojo.require("dojo.widget.HtmlWidget");
dojo.require("dojo.widget.ContentPane");
dojo.require("dojo.string");
dojo.require("dojo.string.extras");
dojo.require("dojo.style");

dojo.widget.html.ContentPane = function(){
	dojo.widget.HtmlWidget.call(this);
}
dojo.inherits(dojo.widget.html.ContentPane, dojo.widget.HtmlWidget);

dojo.lang.extend(dojo.widget.html.ContentPane, {
	widgetType: "ContentPane",
	isContainer: true,

	// remote loading options
	href: "",
	extractContent: true,
	parseContent: true,
	cacheContent: true,
	executeScripts: false,
	preload: false,			// force load of data even if pane is hidden
	refreshOnShow: false,
	handler: "",			// generate pane content from a java function


	// private
	_remoteStyles: null,		// array of stylenodes inserted to document head
					// by remote content, used when we clean up for new content


	postCreate: function(args, frag, parentComp){
		if ( this.handler != "" ){
			this.setHandler(this.handler);
		}
		if(this.preload){ this.loadContents(); }
	},

	onResized: function(){
		if(this.isShowing()){
			this.loadContents();
		}
		dojo.widget.html.ContentPane.superclass.onResized.call(this);
	},

	show: function(){
		// if refreshOnShow is true, reload the contents every time; otherwise, load only the first time
		if(this.refreshOnShow){
			this.refresh();
		}else{
			this.loadContents();
		}
		dojo.widget.html.ContentPane.superclass.show.call(this);
	},

	refresh: function(){
		this.isLoaded=false;
		this.loadContents();
	},

	loadContents: function() {
		if ( this.isLoaded ){
			return;
		}
		this.isLoaded=true;
		if ( dojo.lang.isFunction(this.handler)) {
			this._runHandler();
		} else if ( this.href != "" ) {
			this._downloadExternalContent(this.href, this.cacheContent);
		}
	},

	// Reset the (external defined) content of this pane
	setUrl: function(url) {
		this.href = url;
		this.isLoaded = false;
		if ( this.preload || this.isShowing() ){
			this.loadContents();
		}
	},

	_downloadExternalContent: function(url, useCache) {
		this._handleDefaults("Loading...", "onDownloadStart");
		var self = this;
		dojo.io.bind({
			url: url,
			useCache: useCache,
			preventCache: !useCache,
			mimetype: "text/html",
			handler: function(type, data, e) {
				if(type == "load") {
					self.onDownloadEnd.call(self, url, data);
				} else {
					// works best when from a live serveer instead of from file system
					self._handleDefaults.call(self, "Error loading '" + url + "' (" + e.status + " "+  e.statusText + ")", "onDownloadError");
					self.onLoad();
				}
			}
		});
	},

	// called when setContent is finished
	onLoad: function(e){ /*stub*/ },

	// called when content script eval error or Java error occurs, preventDefault-able
	onExecError: function(e){ /*stub*/ },

	// called on DOM faults, require fault etc in content, preventDefault-able
	onContentError: function(e){ /*stub*/ },

	// called when download error occurs, preventDefault-able
	onDownloadError: function(e){ /*stub*/ },

	// called before download starts, preventDefault-able
	onDownloadStart: function(e){ /*stub*/ },

	// called when download is finished
	onDownloadEnd: function(url, data){
		data = this.splitAndFixPaths(data, url);
		this.setContent(data);
	},

	// usefull if user wants to prevent default behaviour ie: _setContent("Error...")
	_handleDefaults: function(e, handler, useAlert){
		if(!handler){ handler = "onContentError"; }
		if(dojo.lang.isString(e)){
			e = {
				"text": e,
				"toString": function(){ return this.text; }
			}
		}
		if(typeof e.returnValue != "boolean"){
			e.returnValue = true; 
		}
		if(typeof e.preventDefault != "function"){
			e.preventDefault = function(){
				this.returnValue = false;
			}
		}
		// call our handler
		this[handler](e);
		if(e.returnValue){
			if(useAlert){
				alert(e.toString());
			}else{
				this._setContent(e.toString());
			}
		}
	},

	// fix all remote paths in (hopefully) all cases for example images, remote scripts, links etc.
	// splits up content in different pieces, scripts, title, style, link and whats left becomes .xml
	splitAndFixPaths: function(/*String*/s, url){
		if(!url) { url = "./"; } // point to this page if not set
		if(!s) { return ""; }

		// fix up paths in data
		var titles = []; var scripts = []; var linkStyles = [];
		var styles = []; var remoteScripts = []; var requires = [];

		// khtml is much more picky about dom faults, you can't for example attach a style node under body of document
		// must go into head, as does a title node, so we need to cut out those tags
		// cut out title tags
		var match = [];
		while(match){
			match = s.match(/<title[^>]*>([\s\S]*?)<\/title>/i); // can't match with dot as that 
			if(!match){ break;}					//doesnt match newline in js
			titles.push(match[1]);
			s = s.replace(/<title[^>]*>[\s\S]*?<\/title>/i, "");
		}

		// cut out <style> url(...) </style>, as that bails out in khtml
		var match = [];
		while(match){
			match = s.match(/<style[^>]*>([\s\S]*?)<\/style>/i);
			if(!match){ break; }
			styles.push(dojo.style.fixPathsInCssText(match[1], url));
			s = s.replace(/<style[^>]*?>[\s\S]*?<\/style>/i, "");
		}

		// attributepaths one tag can have multiple paths example:
		// <input src="..." style="url(..)"/> or <a style="url(..)" href="..">
		// strip out the tag and run fix on that.
		// this guarantees that we won't run replace another tag's attribute + it was easier do
		var pos = 0; var pos2 = 0; var stop = 0 ;var str = ""; var fixedPath = "";
		var attr = []; var fix = ""; var tagFix = ""; var tag = ""; var regex = ""; 
		while(pos>-1){
			pos = s.search(/<[a-z][a-z0-9]*[^>]*\s(?:(?:src|href|style)=[^>])+[^>]*>/i);
			if(pos==-1){ break; }
			str += s.substring(0, pos);
			s = s.substring(pos, s.length);
			tag = s.match(/^<[a-z][a-z0-9]*[^>]*>/i)[0];
			s = s.substring(tag.length, s.length);

			// loop through attributes
			pos2 = 0; tagFix = ""; fix = ""; regex = ""; var regexlen = 0;
			while(pos2!=-1){
				// slices up before next attribute check, values from previous loop
				tagFix += tag.substring(0, pos2) + fix;
				tag = tag.substring(pos2+regexlen, tag.length);

				// fix next attribute or bail out when done
				// hopefully this charclass covers most urls
				attr = tag.match(/ (src|href|style)=(['"]?)([\w()\/.,\\'"-:;#=&?\s@]+?)\2/i);
				if(!attr){ break; }

				switch(attr[1].toLowerCase()){
					case "src":// falltrough
					case "href":
						// this hopefully covers most common protocols
						if(attr[3].search(/^(https?|ftps?|file|javascript|mailto|news):/)==-1){ 
							fixedPath = (new dojo.uri.Uri(url, attr[3]).toString());
						} else {
							pos2 = pos2 + attr[3].length;
							continue;
						}
						break;
					case "style":// style
						fixedPath = dojo.style.fixPathsInCssText(attr[3], url);
						break;
					default:
						pos2 = pos2 + attr[3].length;
						continue;
				}

				regex = " " + attr[1] + "=" + attr[2] + attr[3] + attr[2];
				regexlen = regex.length;
				fix = " " + attr[1] + "=" + attr[2] + fixedPath + attr[2];
				pos2 = tag.search(new RegExp(dojo.string.escapeRegExp(regex)));
			}
			str += tagFix + tag;
			pos = 0; // reset for next mainloop
		}
		s = str+s;

		// cut out all script tags, stuff them into scripts array
		match = []; var tmp = [];
		while(match){
			match = s.match(/<script([^>]*)>([\s\S]*?)<\/script>/i);
			if(!match){ break; }
			if(match[1]){
				attr = match[1].match(/src=(['"]?)([^"']*)\1/i);
				if(attr){
					// remove a dojo.js or dojo.js.uncompressed.js from remoteScripts
					var tmp = attr[2].match(/.*\bdojo\b\.js/);
					if(tmp && tmp[0] == dojo.hostenv.getBaseScriptUri()+"dojo.js"){
						dojo.debug("Security note! inhibit:"+attr[2]+" from  beeing loaded again.");
					}else{
						remoteScripts.push(attr[2]);
					}
				}
			}
			if(match[2]){
				// strip out all djConfig variables from script tags nodeValue
				// this is ABSOLUTLY needed as reinitialize djConfig after dojo is initialised
				// makes a dissaster greater than Titanic, update remove writeIncludes() to
				var sc = match[2].replace(/(?:var )?\bdjConfig\b(?:[\s]*=[\s]*\{[^}]+\}|\.[\w]*[\s]*=[\s]*[^;\n]*)?;?|dojo\.hostenv\.writeIncludes\(\s*\);?/g, "");
				if(!sc){ continue; }

				// cut out all dojo.require (...) calls, if we have execute 
				// scripts false widgets dont get there require calls
				tmp = [];
				while(tmp && requires.length<100){
					tmp = sc.match(/dojo\.require(?:After)?(?:If)?\((['"]).*?\1\)\s*;?/);
					if(!tmp){ break;}
					requires.push(tmp[0]);
					sc = sc.replace(tmp[0], "");
				}
				scripts.push(sc);
			}
			s = s.replace(/<script[^>]*>[\s\S]*?<\/script>/i, "");
		}

		// cut out all <link rel="stylesheet" href="..">
		match = [];
		while(match){
			match = s.match(/<link ([^>]*rel=['"]?stylesheet['"]?[^>]*)>/i);
			if(!match){ break; }
			attr = match[1].match(/href=(['"]?)([^'">]*)\1/i);
			if(attr){
				linkStyles.push(attr[2]);
			}
			s = s.replace(new RegExp(match[0]), "");
		}

		return {"xml": s, // Object
			"styles": styles,
			"linkStyles": linkStyles,
			"titles": titles,
			"requires": 	requires,
			"scripts": scripts,
			"remoteScripts": remoteScripts,
			"url": url};
	},

	// private internal function without path regExpCheck and no onLoad calls aftervards
	_setContent: function(xml){
		// remove old children from current content
		this.destroyChildren();

		// remove old stylenodes from HEAD
		if(this._remoteStyles){
			for(var i = 0; i < this._remoteStyles.length; i++){
				if(this._remoteStyles[i] && this._remoteStyles.parentNode){
					this._remoteStyles[i].parentNode.removeChild(this._remoteStyles[i]);
				}
			}
			this._remoteStyles = null;
		}

		var node = this.containerNode || this.domNode;
		try{
			node.innerHTML = xml;
		} catch(e){
			e = "Could'nt load html:"+e;
			this._handleDefaults(e, "onContentError");
		}
	},

	setContent: function(data){
		if(!data){
			// if we do a clean using setContent(""); bypass all parseing, extractContent etc
			this._setContent(data);
		}else{
			// need to run splitAndFixPaths? ie. manually setting content
			 if(!data.xml){
				data = this.splitAndFixPaths(data);
			}
			if(this.extractContent) {
				var matches = data.xml.match(/<body[^>]*>\s*([\s\S]+)\s*<\/body>/im);
				if(matches) { data.xml = matches[1]; }
			}
			// insert styleNodes, from <style>....
			for(var i = 0; i < data.styles.length; i++){
				if(i==0){ 
					this._remoteStyles = []; 
				}
				this._remoteStyles.push(dojo.style.insertCssText(data.styles[i]));
			}
			// insert styleNodes, from <link href="...">
			for(var i = 0; i < data.linkStyles.length; i++){
				if(i==0){ 
					this._remoteStyles = []; 
				}
				this._remoteStyles.push(dojo.style.insertCssFile(data.linkStyles[i]));
			}
			this._setContent(data.xml);

			if(this.executeScripts){
				this._executeScripts(data);
			}

			if(this.parseContent){
				for(var i = 0; i < data.requires.length; i++){
					try{ 
						eval(data.requires[i]);
					} catch(e){
						this._handleDefaults(e, "onContentError", true);
					}
				}
				var node = this.containerNode || this.domNode;
				var parser = new dojo.xml.Parse();
				var frag = parser.parseElement(node, null, true);
				dojo.widget.getParser().createComponents(frag, this);
				this.onResized();
			}
		}
		this.onLoad(); // tell system that we have finished
	},

	// Generate pane content from given java function
	setHandler: function(handler) {
		var fcn = dojo.lang.isFunction(handler) ? handler : window[handler];
		if(!dojo.lang.isFunction(fcn)) {
			// FIXME: needs testing! somebody with java knowledge needs to try this
			this._handleDefaults("Unable to set handler, '" + handler + "' not a function.", "onExecError", true);
			return;
		}
		this.handler = function() {
			return fcn.apply(this, arguments);
		}
	},

	_runHandler: function() {
		if(dojo.lang.isFunction(this.handler)) {
			this.handler(this, this.domNode);
			return false;
		}
		return true;
	},

	_executeScripts: function(data) {
		// do remoteScripts first
		var self = this;
		for(var i = 0; i < data.remoteScripts.length; i++){
			dojo.io.bind({
				"url": data.remoteScripts[i],
				"load":     function(type, evaldObj){/* do nothing */ },
				"error":    function(type, error){
						self._handleDefaults.call(self, type + " running remote script", "onExecError", true);
				},
				"mimetype": "text/javascript",
				"sync":     true
			});
		}

		// do inline scripts
		var repl = null;
		for(var i = 0; i < data.scripts.length; i++){
			// Clean up content: remove inline script  comments
			repl = new RegExp('//.*?$', 'gm');
			data.scripts[i] = data.scripts[i].replace(repl, '\n');
	
			// Clean up content: remove carriage returns
			repl = new RegExp('[\n\r]', 'g');
			data.scripts[i] = data.scripts[i].replace(repl, ' ');

			// Execute commands
			try{
				eval(data.scripts[i]);
			}catch(e){
				this._handleDefaults("Error running inline script: "+e+"\n"+data.scripts[i], "onExecError", true);
			}
		}
	}
});

dojo.widget.tags.addParseTreeHandler("dojo:ContentPane");
