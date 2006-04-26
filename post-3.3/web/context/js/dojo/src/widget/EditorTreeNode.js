/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/


dojo.provide("dojo.widget.EditorTreeNode");

dojo.require("dojo.event.*");
dojo.require("dojo.fx.html");
dojo.require("dojo.io.*");

// make it a tag
dojo.widget.tags.addParseTreeHandler("dojo:EditorTreeNode");


// # //////////

dojo.widget.EditorTreeNode = function() {
	dojo.widget.HtmlWidget.call(this);

	this.actionsDisabled = [];
}

dojo.inherits(dojo.widget.EditorTreeNode, dojo.widget.HtmlWidget);

dojo.lang.extend(dojo.widget.EditorTreeNode, {
	widgetType: "EditorTreeNode",

	loadStates: {
		UNCHECKED: "UNCHECKED",
    	LOADING: "LOADING",
    	LOADED: "LOADED"
	},


	actions: {
		MOVE: "MOVE",
    	REMOVE: "REMOVE",
    	EDIT: "EDIT",
    	ADDCHILD: "ADDCHILD"
	},

	isContainer: true,


	templateString: ('<div class="dojoTreeNode"> '
		+ '<span treeNode="${this.widgetId}" class="dojoTreeNodeLabel" dojoAttachPoint="labelNode"> '
		+ '		<span dojoAttachPoint="titleNode" dojoAttachEvent="onClick: onTitleClick" class="dojoTreeNodeLabelTitle">${this.title}</span> '
		+ '</span> '
		+ '<span class="dojoTreeNodeAfterLabel" dojoAttachPoint="afterLabelNode">${this.afterLabel}</span> '
		+ '<div dojoAttachPoint="containerNode"></div> '
		+ '</div>').replace(/(>|<)\s+/g, '$1'), // strip whitespaces between nodes


	childIconSrc: null,
	childIconFolderSrc: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/closed.gif"), // for under root parent item child icon,
	childIconDocumentSrc: dojo.uri.dojoUri("src/widget/templates/images/EditorTree/document.gif"), // for under root parent item child icon,

	childIcon: null,
	isTreeNode: true,

	objectId: "", // the widget represents an object

	afterLabel: "",
	afterLabelNode: null, // node to the left of labelNode

	// an icon left from childIcon: imgs[-2].
	// if +/- for folders, blank for leaves
	expandIcon: null,

	title: "",
	href: "", // node may have href for navigation purposes
	isFolder: false,

	labelNode: null, // the item label
	titleNode: null, // the item title
	imgs: null, // an array of icons imgs


	tree: null,

	depth: 0,

	isExpanded: false,

	state: null,  // after creation will change to loadStates: "loaded/loading/unchecked"
	domNodeInitialized: false,  // domnode is initialized with icons etc


	isFirstNode: function() {
		return this.getPreviousSibling() ? false : true;
	},

	isLastNode: function() {
		return this.getNextSibling() ? false : true;
	},

	actionIsDisabled: function(action) {
		var _this = this;

		return (action == this.actions.ADDCHILD && !this.isFolder) || dojo.lang.inArray(_this.actionsDisabled, action);
	},

	getInfo: function() {
		// No title here (title may be widget)
		var info = {
			widgetId: this.widgetId,
			objectId: this.objectId,
			index: this.getParentIndex(),
			isFolder: this.isFolder
		}

		return info;
	},

	initialize: function(args, frag){
		this.state = this.loadStates.UNCHECKED;

		for(var i=0; i<this.actionsDisabled.length; i++) {
			this.actionsDisabled[i] = this.actionsDisabled[i].toUpperCase();
		}

	},


	/**
	 * Change visible node depth by appending/prepending with blankImgs
	 * @param depthDiff Integer positive => move right, negative => move left
	*/
	adjustDepth: function(depthDiff) {

		for(var i=0; i<this.children.length; i++) {
			this.children[i].adjustDepth(depthDiff);
		}

		this.depth += depthDiff;

		if (depthDiff>0) {
			for(var i=0; i<depthDiff; i++) {
				var img = this.tree.makeBlankImg();
				this.imgs.unshift(img);
				//dojo.debugShallow(this.domNode);
				this.domNode.insertBefore(this.imgs[0], this.domNode.firstChild);

			}
		}
		if (depthDiff<0) {
			for(var i=0; i<-depthDiff;i++) {
				this.imgs.shift();
				this.domNode.removeChild(this.domNode.firstChild);
			}
		}

	},


	markLoading: function() {
		this.expandIcon.src = this.tree.expandIconSrcLoading;


	},


	setFolder: function() {
		dojo.event.connect(this.expandIcon, 'onclick', this, 'onTreeClick');
		this.isFolder = true;
	},


	buildNode: function(tree, depth){

		this.tree = tree;
		this.depth = depth;


		//
		// add the tree icons
		//

		this.imgs = [];

		for(var i=0; i<this.depth+1; i++){

			var img = this.tree.makeBlankImg();

			this.domNode.insertBefore(img, this.labelNode);

			this.imgs.push(img);
		}


		this.expandIcon = this.imgs[this.imgs.length-1];


		this.childIcon = this.tree.makeBlankImg();

		// add to images before the title
		this.imgs.push(this.childIcon);

		dojo.dom.insertBefore(this.childIcon, this.titleNode);

		// node with children(from source html) becomes folder on build stage.
		if (this.children.length || this.isFolder) {
			this.setFolder();
		}

		dojo.event.connect(this.childIcon, 'onclick', this, 'onIconClick');


		//
		// create the child rows
		//


		for(var i=0; i<this.children.length; i++){
			this.children[i].parent = this;

			var node = this.children[i].buildNode(this.tree, this.depth+1);

			this.containerNode.appendChild(node);
		}


		if (this.children.length) {
			this.state = this.loadStates.LOADED;
		}

		if (this.isExpanded) {
			this.expand();
		}else { // leads to double icon updates
			this.updateIcons();
		}

		this.domNodeInitialized = true;

		dojo.event.topic.publish(this.tree.eventNames.nodeCreate, { source: this } );

		return this.domNode;
	},

	onTreeClick: function(e){
		dojo.event.topic.publish(this.tree.eventNames.treeClick, { source: this, event: e });
	},

	onIconClick: function(e){
		dojo.event.topic.publish(this.tree.eventNames.iconClick, { source: this, event: e });
	},

	onTitleClick: function(e){
		dojo.event.topic.publish(this.tree.eventNames.titleClick, { source: this, event: e });
	},

	markSelected: function() {
		dojo.html.addClass(this.titleNode, 'dojoTreeNodeLabelSelected');
	},


	unMarkSelected: function() {
		//dojo.debug('unmark')
		dojo.html.removeClass(this.titleNode, 'dojoTreeNodeLabelSelected');
	},

	/* et the grid under the expand icon */
	updateExpandIcon: function() {


		if (this.tree.showGrid){
			if (this.depth){
				this.setGridImage(-2, this.isLastNode() ? this.tree.gridIconSrcL : this.tree.gridIconSrcT);
			}else{
				if (this.isFirstNode()){
					this.setGridImage(-2, this.isLastNode() ? this.tree.gridIconSrcX : this.tree.gridIconSrcY);
				}else{
					this.setGridImage(-2, this.isLastNode() ? this.tree.gridIconSrcL : this.tree.gridIconSrcT);
				}
			}
		}else{
			this.setGridImage(-2, this.tree.blankIconSrc);
		}

	},

	/* set the grid under the child icon */
	updateChildIcon: function() {

		if ((this.depth || this.tree.showRootGrid) && this.tree.showGrid){
			this.setGridImage(-1, (this.children.length && this.isExpanded) ? this.tree.gridIconSrcP : this.tree.gridIconSrcC);
		}else{
			if (this.tree.showGrid && !this.tree.showRootGrid){
				this.setGridImage(-1, (this.children.length && this.isExpanded) ? this.tree.gridIconSrcZ : this.tree.blankIconSrc);
			}else{
				this.setGridImage(-1, this.tree.blankIconSrc);
			}
		}


	},

	updateParentIcons: function() {
		var parent = this.parent;

		for(var i=0; i<this.depth; i++){

			var idx = this.imgs.length-(3+i);

			this.setGridImage(
				idx,
				(this.tree.showGrid && !parent.isLastNode()) ? this.tree.gridIconSrcV : this.tree.blankIconSrc
			);

			parent = parent.parent;
		}
	},

	updateIcons: function(){


		dojo.profile.start("updateIcons")

		//dojo.debug("Update icons for "+this.widgetId)
		//dojo.debug(this.isFolder)

		this.imgs[0].style.display = this.tree.showRootGrid ? 'inline' : 'none';


		//
		// set the expand icon
		//

		if (this.isFolder){

			this.expandIcon.src = this.isExpanded ? this.tree.expandIconSrcMinus : this.tree.expandIconSrcPlus;

		} else {
			this.expandIcon.src = this.tree.blankIconSrc;
		}

		//
		// set the child icon
		//
		this.buildChildIcon();

		this.updateExpandIcon();
		this.updateChildIcon();
		this.updateParentIcons();


		//alert(dojo.profile.toSource());
		dojo.profile.stop("updateIcons")

	},

	buildChildIcon: function() {
		/* no child icon */
		if (this.childIconSrc == "none") {
			this.childIcon.style.display = 'none';
			return;
		}

		/* assign default icon if not set */
		this.childIcon.src = this.childIconSrc
			|| (this.childIconSrc = this.isFolder ? this.childIconFolderSrc : this.childIconDocumentSrc);


		this.childIcon.style.display = 'inline';
	},

	setGridImage: function(idx, src){

		if (idx < 0){
			idx = this.imgs.length + idx;
		}

		//if (idx >= this.imgs.length-2) return;
		this.imgs[idx].style.backgroundImage = 'url(' + src + ')';
	},


	updateIconTree: function(){
		this.tree.updateIconTree.call(this);
	},



	expand: function(){

		//if (this.title=="Item 1.1") dojo.debug("expand IsExpanded:"+this.isExpanded);


		if (this.children.length) {
			this.showChildren();
		}

		this.isExpanded = true;

		this.updateIcons();
	},

	collapse: function(){
		this.hideChildren();
		this.isExpanded = false;
		this.updateIcons();
	},

	hideChildren: function(){
		dojo.style.hide(this.containerNode);

		/* if dnd is in action, recalculate changed coordinates */
		if(dojo.dnd.dragManager.dragObjects.length) {
			dojo.dnd.dragManager.cacheTargetLocations();
		}
	},

	showChildren: function(){
		dojo.style.show(this.containerNode);

		/* if dnd is in action, recalculate changed coordinates */
		if(dojo.dnd.dragManager.dragObjects.length) {
			dojo.dnd.dragManager.cacheTargetLocations();
		}
	},


	addChild: function(){
		return this.tree.addChild.apply(this, arguments);
	},

	removeChild: function(){
		return this.tree.removeChild.apply(this, arguments);
	},


	/* Edit current node : change properties and update contents */
	edit: function(props) {
		dojo.lang.mixin(this, props);
		if (props.title) {
			this.titleNode.innerHTML = this.title;
		}

		if (props.afterLabel) {
			this.afterLabelNode.innerHTML = this.afterLabel;
		}

		if (props.childIconSrc) {
			this.buildChildIcon();
		}


	},

	toString: function() {
		return "["+this.widgetType+" Tree:"+this.tree+" ID:"+this.widgetId+"]";

	}

});




