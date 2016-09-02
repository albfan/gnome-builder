if ENABLE_SYSPROF_PLUGIN

DISTCLEANFILES =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST = $(plugin_DATA)

plugindir = $(libdir)/gnome-builder/plugins
plugin_LTLIBRARIES = libdebugger-plugin.la
dist_plugin_DATA = debugger.plugin

libdebugger_plugin_la_SOURCES = \
	gbp-debugger-breakpoints.c \
	gbp-debugger-breakpoints.h \
	gbp-debugger-editor-view-addin.c \
	gbp-debugger-editor-view-addin.h \
	gbp-debugger-gutter-renderer.c \
	gbp-debugger-gutter-renderer.h \
	gbp-debugger-perspective.c \
	gbp-debugger-perspective.h \
	gbp-debugger-plugin.c \
	gbp-debugger-service.c \
	gbp-debugger-service.h \
	gbp-debugger-workbench-addin.c \
	gbp-debugger-workbench-addin.h \
	$(NULL)

nodist_libdebugger_plugin_la_SOURCES = \
	gbp-debugger-resources.c \
	gbp-debugger-resources.h

libdebugger_plugin_la_CFLAGS = $(PLUGIN_CFLAGS) $(SYSPROF_CFLAGS)
libdebugger_plugin_la_LIBADD = $(PLUGIN_LIBS) $(SYSPROF_LIBS)
libdebugger_plugin_la_LDFLAGS = $(PLUGIN_LDFLAGS) $(SYSPROF_LDFLAGS)

glib_resources_c = gbp-debugger-resources.c
glib_resources_h = gbp-debugger-resources.h
glib_resources_xml = debugger.gresource.xml
glib_resources_namespace = gbp_debugger
include $(top_srcdir)/build/autotools/Makefile.am.gresources

include $(top_srcdir)/plugins/Makefile.plugin

endif

-include $(top_srcdir)/git.mk
