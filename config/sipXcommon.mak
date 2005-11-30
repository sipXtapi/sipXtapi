# -*-makefile-*-

ConfigureArgs=$(shell sed \
	-e '/^ *\$$ .*\/configure/!d' \
	-e 's/^ *\$$ .*\/configure *//' \
	$(top_builddir)/config.log \
	)

# localize-sipX.sh operates as a filter to localize files.
# It can also take argument pairs "-e" "s/xxx/yyy/" for additional editing.
#
# To make a usable command, we must invoke sh explicitly to run
# localize-sipX.sh, because localize-sipX.sh is created from
# localize-sipX.sh.in, and will not have execute permission.
LocalizeSipXconfig = sh $(top_builddir)/config/localize-sipX.sh
