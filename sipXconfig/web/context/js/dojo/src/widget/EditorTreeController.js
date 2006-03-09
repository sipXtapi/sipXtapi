/*
	Copyright (c) 2004-2005, The Dojo Foundation
	All Rights Reserved.

	Licensed under the Academic Free License version 2.1 or above OR the
	modified BSD license. For more information on Dojo licensing, see:

		http://dojotoolkit.org/community/licensing.shtml
*/


dojo.provide("dojo.widget.EditorTreeController");

dojo.require("dojo.event.*");
dojo.require("dojo.dnd.TreeDragAndDrop");
dojo.require("dojo.json")
dojo.require("dojo.io.*");


dojo.widget.tags.addParseTreeHandler("dojo:EditorTreeController");


dojo.widget.EditorTreeController = function() {
	dojo.widget.HtmlWidget.call(this);

	this.eventNames = {
		select : "",
		deselect : "",
		collapse: "",
		expand: "",
		dblselect: "", // select already selected node.. Edit or whatever

		move: "", // a child was moved from one place to another
		remove: ""
	};


	this.dragSources = {};

	this.dropTargets = {};
}

dojo.inherits(dojo.widget.EditorTreeController, dojo.widget.HtmlWidget);


dojo.lang.extend(dojo.widget.EditorTreeController, {
	widgetType: "EditorTreeController",

	RPCUrl: "local",

	errorHandler: null, // function pointer on error handler

	eventNaming: "default",

	initialize: function(args, frag){

		if (this.eventNaming == "default") { // IE || FF
			for (eventName in this.eventNames) {
				this.eventNames[eventName] = this.widgetId+"/"+eventName;
			}
		}

	},


	DNDMode: "off",


	/**
	 * Common RPC error handler (dies)
	*/
	RPCErrorHandler: function(type, obj) {
		alert( "RPC Error: " + (obj.message||"no message"));
	},


	/**
	 * Binds controller to all tree events
	*/
	subscribeTree: function(tree) {

		if (!tree.eventNames.nodeCreate) dojo.raise("Can't subscribe controller to empty nodeCreate")
		if (!tree.eventNames.treeClick) dojo.raise("Can't subscribe controller to empty treeClick")
		if (!tree.eventNames.iconClick) dojo.raise("Can't subscribe controller to empty iconClick")
		if (!tree.eventNames.titleClick) dojo.raise("Can't subscribe controller to empty titleClick")

		dojo.event.topic.subscribe(tree.eventNames.nodeCreate, this, "onNodeCreate");
		dojo.event.topic.subscribe(tree.eventNames.treeClick, this, "onTreeClick");
		dojo.event.topic.subscribe(tree.eventNames.iconClick, this, "onIconClick");
		dojo.event.topic.subscribe(tree.eventNames.titleClick, this, "onTitleClick");
	},


	/**
	 * Checks whether it is ok to change parent of child to newParent
	 * May incur type checks etc
	 *
	 * It should check only hierarchical possibility w/o index, etc
	 * because in onDragOver event for Between DND mode we can't calculate index at once on onDragOVer.
	 * index changes as client moves mouse up-down over the node
	 */
	canChangeParent: function(child, newParent){

		if (child.actionIsDisabled(child.actions.MOVE)) {
			return false;
		}

		if (child.parent !== newParent && newParent.actionIsDisabled(newParent.actions.ADDCHILD)) {
			return false;
		}

		// Can't move parent under child. check whether new parent is child of "child".
		var node = newParent;
		while(node.isTreeNode) {
			//dojo.debugShallow(node.title)


			if (node === child) {
				// parent of newParent is child
				return false;
			}
			node = node.parent;
		}


		// check for newParent being a folder (if node)
		if (newParent.isTreeNode && !newParent.isFolder) {
			return false;
		}


		return true;
	},



	saveExpandedIndices: function(node, field) {
		var obj = {};

		for(var i=0; i<node.children.length; i++) {
			if (node.children[i].isExpanded) {
				var key = dojo.lang.isUndefined(field) ? i : node.children[i][field];
				obj[key] = this.saveExpandedIndices(node.children[i], field);
			}
		}

		return obj;
	},


	restoreExpandedIndices: function(node, savedIndices, field) {
		var _this = this;



		var handler = function(node, savedIndices) {
			this.node = node; //.children[i];
			this.savedIndices = savedIndices; //[i];
			// recursively read next savedIndices level and apply to opened node
			this.process = function() {
				//dojo.debug("Callback applied for "+this.node);
				_this.restoreExpandedIndices(this.node, this.savedIndices, field);
			};
		}


		for(var i=0; i<node.children.length; i++) {
			var child = node.children[i];

			var found = false;
			var key = -1;

			dojo.debug("Check "+child)
			// process field set case
			if (dojo.lang.isUndefined(field) && savedIndices[i]) {
				found = true;
				key = i;
			}

			// process case when field is not set
			if (field) {
				for(var key in savedIndices) {
					dojo.debug("Compare "+key+" "+child[field])
					if (key == child[field]) {
						found = true;
						break;
					}
				}
			}

			// if we found anything - expand it
			if (found) {
				dojo.debug("Found at "+key)
				var h = new handler(child, savedIndices[key]);
				this.expand(child, h, h.process);
			} else if (child.isExpanded) { // not found, so collapse
				dojo.debug("Collapsing all descendants "+node.children[i])
				dojo.lang.forEach(child.getDescendants(), function(elem) { _this.collapse(elem); });
				//this.collapse(node.children[i]);
			}

		}


	},


	getRPCUrl: function(action) {

		if (this.RPCUrl == "local") { // for demo and tests. May lead to widgetId collisions
			var dir = document.location.href.substr(0, document.location.href.lastIndexOf('/'));
			var localUrl = dir+"/"+action;
			//dojo.debug(localUrl);
			return localUrl;
		}


		if (!this.RPCUrl) {
			dojo.raise("Empty RPCUrl: can't load");
		}

		return this.RPCUrl + ( this.RPCUrl.indexOf("?") > -1 ? "&" : "?") + "action="+action;
	},


	/**
	 * Make request to server about moving children.
	 *
	 * Request returns "true" if move succeeded,
	 * object with error field if failed
	 *
	 * I can't leave DragObject floating until async request returns, need to return false/true
	 * so making it sync way...
	 *
	 * Also, "loading" icon is not shown until function finishes execution, so no indication for remote request.
	*/
	changeParentRemote: function(child, newParent, index){



		//if (newParent.isTreeNode) newParent.markLoading();

		var params = {
			// where from
			child: child.getInfo(),
			childTree: child.tree.getInfo(),
			// where to
			newParent: newParent.getInfo(),
			newParentTree: newParent.tree.getInfo()
		};

		var success;

		//dojo.debug("changeParentRemote request")

		dojo.io.bind({
			url: this.getRPCUrl('changeParent'),
			/* I hitch to get this.loadOkHandler */
			load: dojo.lang.hitch(this,
				function(type, obj) {
					success = this.changeParentProcessResponse(type, obj, child, newParent, index) ;
				}
			),
			error: this.RPCErrorHandler,
			mimetype: "text/json",
			preventCache: true,
			sync: true,
			content: { data: dojo.json.serialize(params) }
		});

		//dojo.debug("changeParentRemote request done")

		return success;
	},

	changeParentProcessResponse: function(type, result, child, newParent, index) {


		if (!dojo.lang.isUndefined(result.error)) {
			this.RPCErrorHandler(result.error);
			return false;
		}

		child.tree.changeParent(child, newParent, index);
		this.updateDND(child);

		return true;
	},

	/**
	 * return true on success, false on failure
	*/
	changeParent: function(child, newParent, index) {

		/* move sourceTreeNode to new parent */
		if (!this.canChangeParent(child, newParent)) {
			return false;
		}

		/* load nodes into newParent in sync mode, if needed, first */
		if (newParent.isTreeNode && newParent.state == newParent.loadStates.UNCHECKED) {
			this.loadRemote(newParent, true);
		}

		var oldParent = child.parent;
		var oldParentIndex = child.getParentIndex();


		var result = this.changeParentRemote(child, newParent, index);

		if (!result) return result;



		/* publish many events here about structural changes */
		dojo.event.topic.publish(this.eventNames.move,
			{
				oldParent: oldParent, oldParentIndex: oldParentIndex,
				newParent: child.parent, newParentIndex: index,
				child: child
			}
		);

		this.expand(newParent);

		return result;
	},





	/**
	 * Add all loaded nodes from array obj as node children and expand it
	*/
	loadProcessResponse: function(type, node, result, callback, callObj) {

		if (!dojo.lang.isUndefined(result.error)) {
			this.RPCErrorHandler(result.error);
			return false;
		}

		var newChildren = result;

		if (!dojo.lang.isArray(newChildren)) {
			dojo.raise('Not array loaded: '+newChildren);
		}

		for(var i=0; i<newChildren.length; i++) {
			// looks like dojo.widget.manager needs no special "add" command
			newChildren[i] = dojo.widget.createWidget(node.widgetType, newChildren[i]);
			node.addChild(newChildren[i])
		}


		//node.addAllChildren(newChildren);

		node.state = node.loadStates.LOADED;

		if (dojo.lang.isFunction(callback)) {
			callback.apply(dojo.lang.isUndefined(callObj) ? this : callObj, [node, newChildren]);
		}
		//this.expand(node);
	},


	/**
	 * Load children of the node from server
	 * Synchroneous loading doesn't break control flow
	 * I need sync mode for DnD
	*/
	loadRemote: function(node, sync, callback, callObj){
		node.markLoading();


		var params = {
			node: node.getInfo(),
			tree: node.tree.getInfo()
		};

		var requestUrl = this.getRPCUrl('getChildren');
		//dojo.debug(requestUrl)

		dojo.io.bind({
			url: requestUrl,
			/* I hitch to get this.loadOkHandler */
			load: dojo.lang.hitch(this,
				function(type, result) {
					this.loadProcessResponse(type, node, result, callback, callObj) ;
				}
			),
			error: this.RPCErrorHandler,
			mimetype: "text/json",
			preventCache: true,
			sync: sync,
			content: { data: dojo.json.serialize(params) }
		});


	},

	onTreeClick: function(message){

		//dojo.debug("EXPAND")

		var node = message.source;

		if (node.isExpanded){
			this.collapse(node);
		} else {
			this.expand(node);
		}
	},


	onIconClick: function(message){
		this.onTitleClick(message);
	},

	onTitleClick: function(message){
		var node = message.source;
		var e = message.event;

		if (node.tree.selector.selectedNode === node){
			dojo.event.topic.publish(this.eventNames.dblselect, { source: node });
			return;
		}


		/* deselect old node */
		if (node.tree.selector.selectedNode) {
			this.deselect(node.tree.selector.selectedNode);
		}

		this.select(node);


	},


	/**
	 * Process drag'n'drop -> drop
	 * NOT event-driven, because its result is important (success/false)
	 * in event system subscriber can't return a result into _current function control-flow_
	 * @return true on success, false on failure
	*/
	processDrop: function(sourceNode, parentNode, index){
		if (dojo.lang.isUndefined(index)) {
			var index = 0;
		}

		//dojo.debug('drop index '+index)
		return this.changeParent(sourceNode, parentNode, index)

	},


	onNodeCreate: function(message) {
		this.registerDNDNode(message.source);
	},

	select: function(node){

		node.markSelected();

		node.tree.selector.selectedNode = node;


		dojo.event.topic.publish(this.eventNames.select, {source: node} );
	},

	deselect: function(node){
		node.unMarkSelected();

		node.tree.selector.selectedNode = null;
		dojo.event.topic.publish(this.eventNames.deselect, {source: node} );
	},

	expand: function(node, callObj, callFunc, sync) {

		if (node.isExpanded) {
			// callback is anyway applied
			if (callFunc) callFunc.apply(callObj, [node]);
			return;
		}

		if (node.state == node.loadStates.UNCHECKED) {
			this.loadRemote(node, sync,
				function(node, newChildren) {
					this.expand(node, callObj, callFunc, sync);
				}
			);
		} else {
			node.expand();

			// callback after expansion
			if (callFunc) callFunc.apply(callObj, [node])

			dojo.event.topic.publish(this.eventNames.expand, {source: node} );
		}
	},

	collapse: function(node) {
		if (!node.isExpanded) return;

		node.collapse();
		dojo.event.topic.publish(this.eventNames.collapse, {source: node} );
	},


	/**
	 * Controller(node model) creates DNDNodes because it passes itself to node for synchroneous drops processing
	 * I can't process DnD with events cause an event can't return result success/false
	*/
	registerDNDNode: function(node) {

		if (this.DNDMode=="off") return;

		/* I drag label, not domNode, because large domNodes are very slow to copy and large to drag */

		var source = null;
		var target = null;

		if (!node.actionIsDisabled(node.actions.MOVE)) {
			//dojo.debug("reg source")
			var source = new dojo.dnd.TreeDragSource(node.labelNode, this, node.tree.widgetId, node);
			this.dragSources[node.widgetId] = source;
		}

		if (this.DNDMode=="onto") {
			var target = new dojo.dnd.TreeDropOntoTarget(node.labelNode, this, node.tree.acceptDropSources, node);
		} else if (this.DNDMode=="between") {
			var target = new dojo.dnd.TreeDropBetweenTarget(node.labelNode, this, node.tree.acceptDropSources, node);
		}

		this.dropTargets[node.widgetId] = target;

	},


	unregisterDNDNode: function(node) {

		if (this.dragSources[node.widgetId]) {
			dojo.dnd.dragManager.unregisterDragSource(this.dragSources[node.widgetId]);
			delete this.dragSources[node.widgetId];
		}

		if (this.dropTargets[node.widgetId]) {
			dojo.dnd.dragManager.unregisterDropTarget(this.dropTargets[node.widgetId]);
			delete this.dropTargets[node.widgetId];
		}
	},

	// update types for DND right accept
	// E.g when move from one tree to another tree
	updateDND: function(node) {

		this.unregisterDNDNode(node);

		this.registerDNDNode(node);

		for(var i=0; i<node.children.length; i++) {
			this.updateDND(node.children[i]);
		}

	},




	// -----------------------------------------------------------------------------
	//                             Remove nodes stuff
	// -----------------------------------------------------------------------------



	canRemoveNode: function(node) {
		return true;
	},

	removeNode: function(node, callback, callObj) {
		if (!this.canRemoveNode(node)) {
			return false;
		}

		return this.removeNodeRemote(node, callback, callObj);

	},


	removeNodeRemote: function(node, callback, callObj) {

		var params = {
			node: node.getInfo(),
			tree: node.tree.getInfo()
		}

		dojo.io.bind({
				url: this.getRPCUrl('removeNode'),
				/* I hitch to get this.loadOkHandler */
				load: dojo.lang.hitch(this, function(type, obj) {
					this.removeNodeProcessResponse(type, node, obj, callback, callObj) }
				),
				error: this.RPCErrorHandler,
				mimetype: "text/json",
				preventCache: true,
				content: {data: dojo.json.serialize(params) }
		});

	},

	removeNodeProcessResponse: function(type, node, result, callback, callObj) {
		if (!dojo.lang.isUndefined(result.error)) {
			this.RPCErrorHandler(result.error);
			return false;
		}

		if (!result) return false;

		if (result == true) {
			/* change parent succeeded */
			this.doRemoveNode(node, node);
			if (callback) {
				// provide context manually e.g with dojo.lang.hitch.
				callback.apply(dojo.lang.isUndefined(callObj) ? this : callObj, [node]);
			}

			return true;
		} else if (dojo.lang.isObject(result)) {
			dojo.raise(result.error);
		} else {
			dojo.raise("Invalid response "+obj)
		}


	},


	/* This function does the real job only w/o any checks */
	doRemoveNode: function(node) {
		/* publish many events here about structural changes */

		if (node.tree.selector.selectedNode === node) {
			this.deselect(node);
		}

		this.unregisterDNDNode(node);

		removed_node = node.tree.removeChild(node);

		// nodes AFTER
		dojo.event.topic.publish(this.eventNames.remove,
			{ node: removed_node }
		);

	},


	// -----------------------------------------------------------------------------
	//                             Create node stuff
	// -----------------------------------------------------------------------------




	canCreateNode: function(parent, index, data) {
		if (!parent.isFolder) return false;

		return true;
	},


	/* send data to server and add child from server */
	/* data may contain an almost ready child, or anything else, suggested to server */
	/* server responds with child data to be inserted */
	createNode: function(parent, index, data, callback, callObj) {
		if (!this.canCreateNode(parent, index, data)) {
			return false;
		}


		/* load nodes into newParent in sync mode, if needed, first */
		if (parent.state == parent.loadStates.UNCHECKED) {
			this.loadRemote(parent, true);
		}


		return this.createNodeRemote(parent, index, data, callback, callObj);

	},


	createNodeRemote: function(parent, index, data, callback, callObj) {

			var params = {
				treeId: parent.tree.widgetId,
				parentId: parent.widgetId,
				index: index,
				data: dojo.json.serialize(data)
			}

			dojo.io.bind({
					url: this.getRPCUrl('createNode'),
					/* I hitch to get this.loadOkHandler */
					load: dojo.lang.hitch(this, function(type, obj) {
						this.createNodeProcessResponse(type, obj, parent, index, callback, callObj) }
					),
					error: this.RPCErrorHandler,
					mimetype: "text/json",
					preventCache: true,
					content: params
			});

	},

	createNodeProcessResponse: function(type, result, parent, index, callback, callObj) {

		if (!dojo.lang.isUndefined(result.error)) {
			this.RPCErrorHandler(result.error);
			return false;
		}

		if (!parent.isTreeNode) {
			dojo.raise("Can only add children to EditorTreeNode")
		}

		if (!dojo.lang.isObject(result)) {
			dojo.raise("Invalid result "+result)
		}

		this.doCreateNode(parent, index, result);

		if (callback) {
			// provide context manually e.g with dojo.lang.hitch.
			callback.apply(dojo.lang.isUndefined(callObj) ? this : callObj, [parent, index, result]);
		}

	},


	doCreateNode: function(parent, index, child) {

		var newChild = dojo.widget.createWidget("EditorTreeNode", child);

		parent.addChild(newChild, index);

		this.expand(parent);
	}


});
