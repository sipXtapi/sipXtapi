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
	handler: "",			// generate pane content from a java function


	postCreate: function(args, frag, parentComp){
		if ( this.handler != "" ){
			this.setHandler(this.handler);
		}
		if(this.preload){ this.loadContents(); }
	},

	onResized: function(){
		if(this.isVisible()){
			this.loadContents();
		}
		dojo.widget.html.ContentPane.superclass.onResized.call(this);
	},

	show: function(){
		this.loadContents();
		dojo.widget.html.ContentPane.superclass.show.call(this);
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
		if ( this.preload || this.isVisible() ){
			this.loadContents();
		}
	},

	_downloadExternalContent: function(url, useCache) {
		this.setContent("Loading...");

		var self = this;
		dojo.io.bind({
			url: url,
			useCache: useCache,
			mimetype: "text/html",
			handler: function(type, data, e) {
				if(type == "load") {
					self.setContent.call(self, data);
				} else {
					self.setContent.call(self, "Error loading '" + url + "' (" + e.status + " " + e.statusText + ")");
				}
			}
		});
	},

	setContent: function(data){
		// remove old children
		dojo.lang.forEach(this.children, function(child){ child.remove(); });
		this.children=[];

		var node = this.containerNode || this.domNode;
		node.innerHTML = data;
		if(this.executeScripts){
			data = this._executeScripts(data);
		}
		if(this.extractContent) {
			var matches = data.match(/<body[^>]*>\s*([\s\S]+)\s*<\/body>/im);
			if(matches) { data = matches[1]; }
		}
		if(this.parseContent){
			var parser = new dojo.xml.Parse();
			var frag = parser.parseElement(node, null, true);
			dojo.widget.getParser().createComponents(frag, this);
			this.onResized();
		}
	},

	// Generate pane content from given java function
	setHandler: function(handler) {
		var fcn = dojo.lang.isFunction(handler) ? handler : window[handler];
		if(!dojo.lang.isFunction(fcn)) {
			throw new Error("Unable to set handler, '" + handler + "' not a function.");
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

	_executeScripts: function(content) {
		// handle <script src="foo"> first
		var src = new RegExp('<script.*?src=".*?"');
		var repl = new RegExp('<script.*?src="');
		var matches = src.exec(content);
		var semaphore = 0;
	   
		if (matches != null)
		{
			for (i = 0; i < matches.length; i++)
			{
				// get the src of the script
				var scriptSrc = matches[i].replace(repl, '');
				scriptSrc = scriptSrc.substring(0, scriptSrc.length-1);
	
				// this evals remote scripts
				dojo.io.bind({
					url:      scriptSrc,
					load:     function(type, evaldObj) {/* do nothing */ },
					error:    function(type, error) {alert(type); alert(error); /* do nothing */ },
					mimetype: "text/javascript",
					sync:     true
				});
			}   
		}
	
		// Remove the script tags we matched
		repl = new RegExp('<script.*?src=".*?".*?</script>');
		content = content.replace(repl, '');

		// Next, handle inline scripts

		// Clean up content: remove inline script  comments
		repl = new RegExp('//.*?$', 'gm');
		content = content.replace(repl, '\n');

		// Clean up content: remove carraige returns
		repl = new RegExp('[\n\r]', 'g');
		content = content.replace(repl, ' ');
	   
		// Match anything inside <script> tags
		src = new RegExp('<script.*?</script>', 'g');
		matches = content.match(src);

		// For each match that is found...
		if (matches != null)
		{
			for (i = 0; i < matches.length; i++)
			{
				// Remove begin tag
				var repl = new RegExp('<script.*?>', 'gm');
				var script = matches[i].replace(repl, '');

				// Remove end tag
				repl = new RegExp('</script>', 'gm');
				script = script.replace(repl, '');
	
				// Execute commands
				eval(script);
			}   
		}
		return content;
	}
});

dojo.widget.tags.addParseTreeHandler("dojo:ContentPane");
