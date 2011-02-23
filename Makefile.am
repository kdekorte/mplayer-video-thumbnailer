## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src 

nailerdocdir = ${prefix}/share/doc/nailer-${VERSION}
nailerdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

thunardir = ${prefix}/share/thumbnailers
thunar_in_files = video-thumbnailer.desktop.in
thunar_DATA = video-thumbnailer.desktop

if GCONF_SCHEMAS_INSTALL
schemasdir =  @GCONF_SCHEMA_FILE_DIR@
schemas_in_files = nailer.schemas.in
schemas_DATA = nailer.schemas
endif

EXTRA_DIST = $(nailerdoc_DATA) $(schemas_DATA) $(thunar_DATA) nailer.schemas.in video-thumbnailer.desktop.in nailer.spec.in


install-schemas: $(schemas_DATA)
	GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source` gconftool-2 --makefile-install-rule $(schemas_DATA)

if GCONF_SCHEMAS_INSTALL
install-data-hook: install-schemas
endif
	
# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

