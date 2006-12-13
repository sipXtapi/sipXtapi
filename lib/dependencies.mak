# this is used by sipxbuild to compute a correct build order

.PHONY: cgicc
cgicc :
	@echo $@

.PHONY: cppunit
cppunit :
	@echo $@

.PHONY: java-ibm
java-ibm :
	@echo $@

.PHONY: java-sun
java-sun :
	@echo $@

.PHONY: mod_cplusplus
mod_cplusplus :
	@echo $@

.PHONY: pcre
pcre :
	@echo $@

.PHONY: ruby
ruby :
	@echo $@

.PHONY: ruby-gems
ruby-gems : ruby
	@echo $@

.PHONY: ruby-postgres
ruby-postgres : ruby-gems
	@echo $@

.PHONY: ruby-rails
ruby-rails : ruby-gems
	@echo $@

.PHONY: w3c-libwww
w3c-libwww :
	@echo $@

.PHONY: xerces-c
xerces-c :
	@echo $@
