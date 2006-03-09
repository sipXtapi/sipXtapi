/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

/**
 * Tree model does all the drawing, visual node management etc.
 * Throws events about clicks on it, so someone may catch them and process
 * Tree knows nothing about DnD stuff, covered in TreeDragAndDrop and (if enabled) attached by controller
*/
dojo.provide("dojo.widget.EditorTree");

dojo.require("dojo.event.*");
dojo.require("dojo.fx.html");
dojo.require("dojo.io.*");
dojo.require("dojo.widget.HtmlWidget");
dojo.require("dojo.widget.EditorTreeNode");
dojo.require("dojo.widget.EditorTreeSelector");
dojo.require("dojo.widget.EditorTreeController");



// make it a tag
dojo.widget.tags.addParseTreeHandler("dojo:EditorTree");


dojo.widget.EditorTree = function() {
	dojo.widget.HtmlWidget.call(this);

	this.eventNames = {
		// new node built.. Well, just built
		nodeCreate: "",
		// expand icon clicked
		treeClick: "",
		// node icon clicked
		iconClick: "",
		// node title clicked
		titleClick: ""
	};

	this.tree = this;
	this.acceptDropSources = [];
	this.actionsDisabled = [];

}
dojo.inherits(dojo.widget.EditorTree, dojo.widget.HtmlWidget);

/* extend DOES NOT copy recursively */
dojo.lang.extend(dojo.widget.EditorTree, {
	widgetType: "EditorTree",

	isContainer: true,

	acceptDropSources: "",

	templateCssPath: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/EditorTree.css"),

	templateString: '<div class="dojoTree"></div>',

	/* Model events */
	eventNames: null,
	eventNaming: "default",

	isExpanded: true, // consider this "root node" to be always expanded

	isTree: true,

	objectId: "",


	//
	// these icons control the grid and expando buttons for the whole tree
	//

	blankIconSrc: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_blank.gif"),

	gridIconSrcT: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_grid_t.gif"), // for non-last child grid
	gridIconSrcL: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_grid_l.gif"), // for last child grid
	gridIconSrcV: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_grid_v.gif"), // vertical line
	gridIconSrcP: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_grid_p.gif"), // for under parent item child icons
	gridIconSrcC: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_grid_c.gif"), // for under child item child icons
	gridIconSrcX: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_grid_x.gif"), // grid for sole root item
	gridIconSrcY: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_grid_y.gif"), // grid for last rrot item
	gridIconSrcZ: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_grid_z.gif"), // for under root parent item child icon

	expandIconSrcPlus: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_expand_plus.gif"),
	expandIconSrcMinus: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_expand_minus.gif"),
	expandIconSrcLoading: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/treenode_loading.gif"),


	iconWidth: 18,
	iconHeight: 18,


	//
	// tree options
	//

	showGrid: true,
	showRootGrid: true,

	selector: null,


	actionIsDisabled: function(action) {
		var _this = this;
		return dojo.lang.inArray(_this.actionsDisabled, action)
	},


	actions: {
    	ADDCHILD: "ADDCHILD"
	},


	getInfo: function() {
		var info = {
			widgetId: this.widgetId,
			objectId: this.objectId
		}

		return info;
	},

	initialize: function(args, frag){

		var _this = this;


		//this.actionsDisabled = this.actionsDisabled.split(",");
		for(var i=0; i<this.actionsDisabled.length; i++) {
			this.actionsDisabled[i] = this.actionsDisabled[i].toUpperCase();
		}



		if (this.eventNaming == "default") { // IE || FF

			for (eventName in this.eventNames) {
				this.eventNames[eventName] = this.widgetId+"/"+eventName;
			}
		}


		if (args['controller']) {
			var controller = dojo.widget.manager.getWidgetById(args['controller']);

			controller.subscribeTree(this); // controller listens to my events
		}

		if (args['selector']) {
			this.selector = dojo.widget.manager.getWidgetById(args['selector']);
		} else {
			this.selector = new dojo.widget.createWidget("EditorTreeSelector");
		}

		this.containerNode = this.domNode;

	},



	postCreate: function() {
		this.buildTree();
	},

	buildTree: function() {

		dojo.html.disableSelection(this.domNode);

		for(var i=0; i<this.children.length; i++){
			this.children[i].parent = this; // root nodes have tree as parent

			var node = this.children[i].buildNode(this, 0);


			this.domNode.appendChild(node);
		}



		if (!this.showRootGrid){
			for(var i=0; i<this.children.length; i++){
				this.children[i].expand();
			}
		}


	},



	/**
	 * Move child to newParent as last child
	 * redraw tree and update icons
	*/
	changeParent: function(child, newParent, index) {
//		dojo.debug("Move "+child+" to "+newParent+" index "+index)

		//dojo.debug(dojo.widget.manager.getWidgetById('1.3').containerNode.style.display);

		var destIdx = index;
/*
		if (child.parent === newParent && child.getParentIndex()<=index) {
			dojo.debug("shift dest index")
			destIdx--; // shift index cause child.length is less by 1 after removal
		}
	*/

		/* do actual parent change here. Write remove child first */
		child.parent.removeChild(child);

		newParent.addChild(child, destIdx);

	},

	// FIXME: removeChild may leak because events do not get cleaned on removal like:
	// dojo.event.browser.clean( parent.removeChild( child ) );
	// From the other hand, if I clean events I have to reinsert'em later..
	// maybe supply "clean" parameter ?
	removeChild: function(child) {

		var parent = child.parent;

		var children = parent.children;

		for(var i=0; i<children.length; i++){
			if(children[i] === child){
				children.splice(i, 1);
				break;
			}
		}

		dojo.dom.removeNode(child.domNode);


		parent.updateIconTree();

		return child;

	},



	// not called for initial tree building. See buildNode instead.
	// builds child html node if needed
	// index is "last node" by default
	addChild: function(child, index){


		dojo.profile.start("AddChild");

		if (dojo.lang.isUndefined(index)) {
			index = this.children.length;
		}

		if (!child.isTreeNode){
			dojo.raise("You can only add EditorTreeNode widgets to a "+this.widgetType+" widget!");
			return;
		}

		// usually it is impossible to change "isFolder" state, but if anyone wants to add a child to leaf,
		// it is possible program-way.
		if (this.isTreeNode){
			if (!this.isFolder) { // just became a folder.
				this.setFolder();
			}
		}

		// adjust tree
		var _this = this;
		dojo.lang.forEach(child.getDescendants(), function(elem) { elem.tree = _this.tree; });

		// fix parent
		child.parent = this;


		// no dynamic loading for those who are parents already
		if (this.isTreeNode) {
			this.state = this.loadStates.LOADED;
		}

		// if node exists - adjust its depth, otherwise build it
		if (child.domNodeInitialized) {
			//dojo.debug(this.widgetType)
			var d = this.isTreeNode ? this.depth : -1;
			//dojo.debug('Depth is '+this.depth);
			child.adjustDepth( d - child.depth + 1 );
		} else {
			child.depth = this.isTreeNode ? this.depth+1 : 0;
			child.buildNode(child.tree, child.depth);
		}


		//dojo.debug(child.domNode.outerHTML)

		if (index < this.children.length) {

			//dojo.debug('insert '+index)
			//dojo.debugShallow(child);

			// insert
			dojo.dom.insertBefore(child.domNode, this.children[index].domNode);
		} else {
			this.containerNode.appendChild(child.domNode);
		}

		/*
		if (index == this.children.length && this.children.length == 0) {
			this.containerNode.style.display = 'block';
		}
		*/

		this.children.splice(index, 0, child);

		dojo.profile.end("AddChild");

		dojo.profile.start("updateIconTree");

		//this.updateIconTree();
		//dojo.debug("Update my icons: "+child)
		child.updateIcons();
		if (child.isFirstNode() && child.getNextSibling()) {
			//dojo.debug("Update expand for"+child.getNextSibling())
			child.getNextSibling().updateExpandIcon();
		}
		if (child.isLastNode() && child.getPreviousSibling()) {
			//dojo.debug("Update expand for"+child.getPreviousSibling())
			child.getPreviousSibling().updateExpandIcon();
		}


		dojo.profile.end("updateIconTree");


	},

	makeBlankImg: function() {
		var img = document.createElement('img');

		img.style.width = this.iconWidth + 'px';
		img.style.height = this.iconHeight + 'px';
		img.src = this.blankIconSrc;
		img.style.verticalAlign = 'middle';

		return img;
	},


	updateIconTree: function(){

		//dojo.debug("Update icons for "+this)
		if (!this.isTree) {
			this.updateIcons();
		}

		for(var i=0; i<this.children.length; i++){
			this.children[i].updateIconTree();
		}

	},

	toString: function() {
		return "["+this.widgetType+" ID:"+this.widgetId+"]"
	}





});
