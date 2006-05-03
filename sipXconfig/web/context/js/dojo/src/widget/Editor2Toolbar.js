/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.widget.Editor2Toolbar");
dojo.provide("dojo.widget.html.Editor2Toolbar");

dojo.require("dojo.widget.*");
dojo.require("dojo.event.*");
dojo.require("dojo.widget.RichText");

dojo.widget.defineWidget(
	"dojo.widget.html.Editor2Toolbar",
	dojo.widget.HtmlWidget,
	{
		commandList: [ "bold", "italic", "underline", "subscript", "superscript",
			"fontname", "fontsize", "forecolor", "hilitecolor", "justifycenter",
			"justifyfull", "justifyleft", "justifyright", "cut", "copy", "paste",
			"delete", "undo", "redo", "createlink", "unlink", "removeformat",
			"inserthorizontalrule", "insertimage", "insertorderedlist",
			"insertunorderedlist", "indent", "outdent", "formatblock", "strikethrough", 
			"inserthtml", "blockdirltr", "blockdirrtl", "dirltr", "dirrtl",
			"inlinedirltr", "inlinedirrtl", "inserttable", "insertcell",
			"insertcol", "insertrow", "deletecells", "deletecols", "deleterows",
			"mergecells", "splitcell"
		],

		templatePath: dojo.uri.dojoUri("src/widget/templates/HtmlEditorToolbar.html"),
		templateCssPath: dojo.uri.dojoUri("src/widget/templates/HtmlEditorToolbar.css"),

		// DOM Nodes
		wikiWordButton: null,
		styleDropdownButton: null,
		styleDropdownContainer: null,
		copyButton: null,
		boldButton: null,
		italicButton: null,
		underlineButton: null,
		justifyleftButton: null,
		justifyfullButton: null,
		justifyrightButton: null,
		pasteButton: null,
		undoButton: null,
		redoButton: null,
		linkButton: null,
		insertunorderedlistButton: null,
		insertorderedlistButton: null,
		forecolorButton: null,
		hilitecolorButton: null,
		formatSelectBox: null,

		buttonClick: function(){ dojo.debug("buttonClick"); },

		buttonMouseOver: function(e){  },
		buttonMouseOut: function(e){  },

		// event signals
		wikiWordClick: function(){ dojo.debug("wikiWordButtonClick"); },
		styleDropdownClick: function(){
			dojo.debug("styleDropdownClick:", this.styleDropdownContainer);
			dojo.style.toggleShowing(this.styleDropdownContainer);
		},

		closeAllDropDowns: function(){
		},

		copyClick: function(){ this.exec("copy"); },
		boldClick: function(){ this.exec("bold"); },
		italicClick: function(){ this.exec("italic"); },
		underlineClick: function(){ this.exec("underline"); },
		justifyleftClick: function(){ this.exec("justifyleft"); },
		justifyfullClick: function(){ this.exec("justifyfull"); },
		justifyrightClick: function(){ this.exec("justifyright"); },
		pasteClick: function(){ this.exec("paste"); },
		undoClick: function(){ this.exec("undo"); },
		redoClick: function(){ this.exec("redo"); },
		linkClick: function(){ 
			this.exec(	"createlink", 
						prompt("Please enter the URL of the link:", "http://"));
		},
		insertunorderedlistClick: function(){ this.exec("insertunorderedlist"); },
		insertorderedlistClick: function(){ this.exec("insertorderedlist"); },

		formatSelectClick: function(){ 
			// dojo.debug(this.formatSelectBox.value);
			var sv = this.formatSelectBox.value.toLowerCase();
			this.exec("formatblock", sv);
			/*
			if(sv == "blockquote"){
				this.exec("italic");
			}
			*/
		},

		normalTextClick: function(){ this.exec("formatblock", "p"); },
		h1TextClick: function(){ this.exec("formatblock", "h1"); },
		h2TextClick: function(){ this.exec("formatblock", "h2"); },
		h3TextClick: function(){ this.exec("formatblock", "h3"); },
		h4TextClick: function(){ this.exec("formatblock", "h4"); },

		forecolorClick: function(){
			//TODO
		},

		hilitecolorClick: function(){
			//TODO
		},

		indentClick: function(){ this.exec("indent"); },
		outdentClick: function(){ this.exec("outdent"); },

		// stub that an observer can connect to
		exec: function(what, arg){ },

		hideUnusableButtons: function(){
			dojo.lang.forEach(this.commandList,
				function(cmd){
					if(this[cmd+"Button"]){
						var cb = this[cmd+"Button"];
						if(!dojo.widget.html.RichText.prototype.queryCommandAvailable(cmd)){
							cb.style.display = "none";
						}
					}
				},
				this);
		}
	},
	"html",
	function(){
		dojo.event.connect(this, "fillInTemplate", this, "hideUnusableButtons");
	}
);
