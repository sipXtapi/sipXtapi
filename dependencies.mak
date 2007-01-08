# this is used by sipxbuild to compute a correct build order

.PHONY: sipXtools
sipXtools :
	@echo sipXtools

.PHONY: sipXbuild
sipXbuild :
	@echo sipXbuild

.PHONY: sipXportLib
sipXportLib :
	@echo sipXportLib

.PHONY: sipXtackLib
sipXtackLib : sipXportLib
	@echo sipXtackLib

.PHONY: sipXmediaLib
sipXmediaLib : sipXtackLib
	@echo sipXmediaLib

.PHONY: sipXmediaAdapterLib
sipXmediaAdapterLib : sipXmediaLib
	@echo sipXmediaAdapterLib

.PHONY: sipXcallLib
sipXcallLib : sipXmediaAdapterLib
	@echo sipXcallLib

.PHONY: sipXcommserverLib
sipXcommserverLib : sipXtackLib
	@echo sipXcommserverLib

.PHONY: sipXpbx
sipXpbx : sipXcommserverLib
	@echo sipXpbx

.PHONY: sipXpublisher
sipXpublisher : sipXpbx
	@echo sipXpublisher

.PHONY: sipXregistry
sipXregistry : sipXpbx
	@echo sipXregistry

.PHONY: sipXproxy
sipXproxy : sipXpbx
	@echo sipXproxy

.PHONY: sipXconfig
sipXconfig : sipXpbx
	@echo sipXconfig

.PHONY: sipXvxml
sipXvxml : sipXcallLib sipXpbx sipXmediaAdapterLib
	@echo sipXvxml

