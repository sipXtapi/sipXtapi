/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/

/**
 * TreeDrag* specialized on managing subtree drags
 * It selects nodes and visualises what's going on,
 * but delegates real actions upon tree to the controller
 *
 * This code is considered a part of controller
*/

dojo.provide("dojo.dnd.TreeDragAndDrop");
dojo.provide("dojo.dnd.TreeDragSource");
dojo.provide("dojo.dnd.TreeDropTarget");

dojo.require("dojo.dnd.HtmlDragAndDrop");
dojo.require("dojo.lang.func");
dojo.require("dojo.lang.array");
dojo.require("dojo.lang.extras");

dojo.dnd.TreeDragSource = function(node, syncController, type, treeNode){
	this.controller = syncController;
	this.treeNode = treeNode;

	dojo.dnd.HtmlDragSource.call(this, node, type);
}

dojo.inherits(dojo.dnd.TreeDragSource, dojo.dnd.HtmlDragSource);

dojo.lang.extend(dojo.dnd.TreeDragSource, {
	onDragStart: function(){
		/* extend adds functions to prototype */
		var dragObject = dojo.dnd.HtmlDragSource.prototype.onDragStart.call(this);
		//dojo.debugShallow(dragObject)

		dragObject.treeNode = this.treeNode;

		dragObject.onDragStart = dojo.lang.hitch(dragObject, function(e) {

			/* save selection */
			this.savedSelectedNode = this.treeNode.tree.selector.selectedNode;
			if (this.savedSelectedNode) {
				this.savedSelectedNode.unMarkSelected();
			}

			var result = dojo.dnd.HtmlDragObject.prototype.onDragStart.apply(this, arguments);

			/* remove background grid from cloned object */
			dojo.lang.forEach(
				this.dragClone.getElementsByTagName('img'),
				function(elem) { elem.style.backgroundImage='' }
			);

			return result;


		});

		dragObject.onDragEnd = function(e) {

			/* restore selection */
			if (this.savedSelectedNode) {
				this.savedSelectedNode.markSelected();
			}
			//dojo.debug(e.dragStatus);

			return dojo.dnd.HtmlDragObject.prototype.onDragEnd.apply(this, arguments);
		}
		//dojo.debug(dragObject.domNode.outerHTML)


		return dragObject;
	},

	onDragEnd: function(e){


		 var res = dojo.dnd.HtmlDragSource.prototype.onDragEnd.call(this, e);


		 return res;
	}
});

// .......................................

dojo.dnd.TreeDropTarget = function(node, syncController, type, treeNode){

	this.treeNode = treeNode;
	this.controller = syncController; // I will sync-ly process drops

	dojo.dnd.HtmlDropTarget.apply(this, [node, type]);

}

dojo.inherits(dojo.dnd.TreeDropTarget, dojo.dnd.HtmlDropTarget);

dojo.lang.extend(dojo.dnd.TreeDropTarget, {

	autoExpandDelay: 1500,
	autoExpandTimer: null,

	/**
	 * Check if I can drop sourceTreeNode here
	 * only tree node targets are implemented ATM
	*/
	onDragOver: function(e){

		var accepts = dojo.dnd.HtmlDropTarget.prototype.onDragOver.apply(this, arguments);

		//dojo.debug("TreeDropTarget.onDragOver accepts:"+accepts)

		if (accepts && this.treeNode.isFolder && !this.treeNode.isExpanded) {
			this.setAutoExpandTimer();
		}

		return accepts;
	},


	setAutoExpandTimer: function() {
		// set up autoexpand timer
		var _this = this;
		var autoExpand = function () {
			if (dojo.dnd.dragManager.currentDropTarget === _this) {
				_this.controller.expand(_this.treeNode);
			}
		}

		this.autoExpandTimer = dojo.lang.setTimeout(autoExpand, _this.autoExpandDelay);
	},


	accepts: function(dragObjects) {

		var accepts = dojo.dnd.HtmlDropTarget.prototype.accepts.apply(this, arguments);

		if (!accepts) return false;

		var sourceTreeNode = dragObjects[0].treeNode;

		if (dojo.lang.isUndefined(sourceTreeNode) || !sourceTreeNode || !sourceTreeNode.isTreeNode) {
			dojo.raise("Source is not TreeNode or not found");
		}

		return true;
	},


	onDragOut: function(e) {
		this.clearAutoExpandTimer();
	},


	clearAutoExpandTimer: function() {
		if (this.autoExpandTimer) {
			clearTimeout(this.autoExpandTimer);
			delete this.autoExpandTimer;
			this.autoExpandTimer = null;
		}
	},

	onDragMove: function(e, dragObjects){
	}


});


dojo.dnd.TreeDropOntoTarget = function(node, syncController, type, treeNode){
	dojo.dnd.TreeDropTarget.apply(this, arguments);
}


dojo.inherits(dojo.dnd.TreeDropOntoTarget, dojo.dnd.TreeDropTarget);


dojo.lang.extend(dojo.dnd.TreeDropOntoTarget, {


	accepts: function(dragObjects) {
		//dojo.debug("TreeDropOntoTarget.accepts")

		var accepts = dojo.dnd.TreeDropTarget.prototype.accepts.apply(this, arguments);

		if (!accepts) return false;

		// checked by parent call
		var sourceTreeNode = dragObjects[0].treeNode;

		// can't drop parent to child etc
		accepts = this.controller.canChangeParent(sourceTreeNode, this.treeNode);

		if (!accepts) return false;

		// can't drag to same parent in onto mode
		if (sourceTreeNode.parent === this.treeNode) {
			return false;
		}



		//dojo.debug("Check2 "+acceptable)


		return true;

	},

	onDragOver: function(e) {

		// perform common DnD actions and learn whether the drop acceptable
		var accepts = dojo.dnd.TreeDropTarget.prototype.onDragOver.apply(this, arguments);

		//dojo.debug("TreeDropOntoTarget.onDragOver accepts:"+accepts)

		if (!accepts) return false;

		// mark current node being dragged into
		this.treeNode.markSelected();


		return true;

	},

	// called for all targets
	onDragOut: function(e) {

		dojo.dnd.TreeDropTarget.prototype.onDragOut.apply(this, arguments);

		this.treeNode.unMarkSelected();
	},


	// called for accepted objects only
	onDrop: function(e) {

		this.onDragOut(e);

		var sourceTreeNode = e.dragObject.treeNode;

		return this.controller.processDrop(sourceTreeNode, this.treeNode, 0);
	}


});


// .......................................


dojo.dnd.TreeDropBetweenTarget = function(node, syncController, type, treeNode){
	dojo.dnd.TreeDropTarget.apply(this, arguments);
}


dojo.inherits(dojo.dnd.TreeDropBetweenTarget, dojo.dnd.TreeDropTarget);


dojo.lang.extend(dojo.dnd.TreeDropBetweenTarget, {

	indicatorPosition: null,

	indicatorStyle: "2px black solid",
	indicatorVisible: false,


	showIndicator: function(position) {

		// do not change style too often, cause of blinking possible
		if (this.indicatorPosition == position && this.indicatorVisible) {
			return;
		}

		//dojo.debug(position)

		this.hideIndicator();

		this.indicatorPosition = position;
		this.indicatorVisible = true;

		if (position == "before") {
			this.treeNode.labelNode.style.borderTop = this.indicatorStyle;
		} else if (position == "after") {
			this.treeNode.labelNode.style.borderBottom = this.indicatorStyle;
		}


	},

	hideIndicator: function() {
		this.indicatorVisible = false;
		this.treeNode.labelNode.style.borderBottom="";
		this.treeNode.labelNode.style.borderTop="";
	},



	// is the target possibly ok ?
	// This function is run on dragOver, but drop possibility is also determined by position over node
	// that's why acceptsWithPosition is called
	// doesnt take index into account ( can change while moving mouse w/o changing target )
	accepts: function(dragObjects) {
		//dojo.debug("TreeDropBetweenTarget.accepts")

		var accepts = dojo.dnd.TreeDropTarget.prototype.accepts.apply(this, arguments);

		if (!accepts) return false;

		// checked by parent call
		var sourceTreeNode = dragObjects[0].treeNode;

		// can't drop parent to child etc
		accepts = this.controller.canChangeParent(sourceTreeNode, this.treeNode.parent);

		if (!accepts) return false;


		//dojo.debug("Check2 "+acceptable)


		return true;

	},


	acceptsWithPosition: function(sourceNode, position) {
		return !this.isAdjacentNode(sourceNode, position);
	},


	isAdjacentNode: function(sourceNode, position) {

		if (sourceNode === this.treeNode) return true;
		if (sourceNode.getNextSibling() === this.treeNode && position=="before") return true;
		if (sourceNode.getPreviousSibling() === this.treeNode && position=="after") return true;

		return false;
	},


	/**
	 * Check if I can drop sourceTreeNode here
	 * only tree node targets are implemented ATM
	 *
	 * returns true for nodes different from current node even if they are siblings.
	*/
	onDragOver: function(e){

		// perform common DnD actions and learn whether the drop acceptable
		var accepts = dojo.dnd.TreeDropTarget.prototype.onDragOver.apply(this, arguments);

		if (!accepts) return false;

		return true;

	},

	getPosition: function(e) {
		if (dojo.html.gravity(this.treeNode.labelNode, e) & dojo.html.gravity.NORTH) {
			var position = "before";
		} else {
			var position = "after";
		}

		return position;
	},


	// only called if drop acceptable
	onDragMove: function(e, dragObjects){

		//dojo.debugShallow(dragObjects);

		var position = this.getPosition(e);

		var sourceTreeNode = dragObjects[0].treeNode;

		if (this.acceptsWithPosition(sourceTreeNode, position)) {
			this.showIndicator(position);
		}

	},

	onDragOut: function(e) {
		//dojo.debug("Out");

		dojo.dnd.TreeDropTarget.prototype.onDragOut.apply(this, arguments);

		this.hideIndicator();

		//return dojo.dnd.HtmlDropTarget.prototype.onDragOut.call(this, e);
	},


	getTargetParentIndex: function(sourceTreeNode, position) {

		var index = position == "before" ? this.treeNode.getParentIndex() : this.treeNode.getParentIndex()+1;
		if (this.treeNode.parent === sourceTreeNode.parent
		  && this.treeNode.getParentIndex() > sourceTreeNode.getParentIndex()) {
		  	index--;  // dragging a node is different for simple changeParent bacause of before-after issues
		}

		return index;
	},


	onDrop: function(e){
		this.onDragOut(e);

		//dojo.debug('drop');

		var sourceTreeNode = e.dragObject.treeNode;

		if (!dojo.lang.isObject(sourceTreeNode)) {
			dojo.raise("TreeNode not found in dragObject")
		}


		if (!this.acceptsWithPosition(sourceTreeNode, this.indicatorPosition)) {
			return false;
		}

		// I don't check that trees are same! Target/source system deals with it

		//tree.changeParentRemote(sourceTreeNode, targetTreeNode);
		var index = this.getTargetParentIndex(sourceTreeNode, this.indicatorPosition);

		//dojo.debug('drop2');

		return this.controller.processDrop(sourceTreeNode, this.treeNode.parent, index);

	},

	toString: function() {
		return "[TreeDropBetweenTarget "+this.treeNode+"]";
	}
});
