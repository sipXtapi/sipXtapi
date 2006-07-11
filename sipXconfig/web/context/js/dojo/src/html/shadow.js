/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

dojo.provide("dojo.html.shadow");

dojo.require("dojo.lang");
dojo.require("dojo.uri");

if(dojo.render.html.ie){
	dojo.style.insertCssFile(dojo.uri.dojoUri("src/html/shadowIE.css"));
}else{
	dojo.style.insertCssFile(dojo.uri.dojoUri("src/html/shadow.css"));
}

dojo.html.shadow = function(node) {
	this.init(node);
}

dojo.lang.extend(dojo.html.shadow, {
	shadowPng: dojo.uri.dojoUri("src/html/images/shadow"),

	init: function(node){
		this.node=node;

		var pieces = [ "T", "TL", "TR", "L", "R", "BL", "BR", "B" ];
		var urlBase = this.shadowPng;
		
		dojo.lang.forEach(pieces,
			function(piece) {
				var child = document.createElement("div");
				child.className="dojoShadow"+piece;
				node.appendChild(child);
				
				// to get the pathnames right we have to generate the CSS :-(
				if(dojo.render.html.ie){
					child.innerHTML = "<div></div>";
					var url = urlBase + piece + ".png";
					sizing=(dojo.lang.inArray(['T','L','R','B'],piece)?"scale":"");
					child.style.filter=
						"progid:DXImageTransform.Microsoft.AlphaImageLoader(src='"+url+"'"+
						(sizing?", sizingMethod='"+sizing+"'":"") + ")";
				}
			}
		);
/*		
		var t= "top: -8px;";
		var l= "left: -8px;";
		var b= dojo.render.html.ie ? "bottom: -6px;" : "bottom: -8px;";
		var r= dojo.render.html.ie ? "right: -6px;" : "right: -8px;";
		this._makePiece("UL", t + l + "width: 8px; height: 8px;");
		this._makePiece("UR", t + r + "width: 8px; height: 8px;");
		this._makePiece("BL", b + l + "width: 8px; height: 8px;");
		this._makePiece("BR", b + r + "width: 8px; height: 8px;");

		this._makePiece("T", t + "left: 0px; width: 100%; height: 8px;", "scale");
		this._makePiece("B", b + "left: 0px; width: 100%; height: 8px;", "scale");
		this._makePiece("L", "top: 0px;" + l + "width: 8px; height: 100%;", "scale");
		this._makePiece("R", "top: 0px;" + r + "width: 8px; height: 100%;", "scale");
*/
	},

	_makePiece: function(name, cssText, sizing){
		var img;
		var url = this.shadowPng + name.toUpperCase() + ".png";
		if(dojo.render.html.ie){
			img=document.createElement("div");
			img.style.cssText="position: absolute;" + cssText +
				"filter:progid:DXImageTransform.Microsoft.AlphaImageLoader(src='"+url+"'"+
				(sizing?", sizingMethod='"+sizing+"'":"") + ")";
		}else{
			img=document.createElement("img");
			img.src=url;
			img.style.cssText="position: absolute;" + cssText;
		}
		this.node.appendChild(img);
	},

	size: function(width, height){
		dojo.deprecated("size", "stop calling this function, it's no longer necessary");
	}
});

