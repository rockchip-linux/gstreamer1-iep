# Note: plugindir is set in configure

GST_API_VERSION = 1.0
##############################################################################
# TODO: change libgstiepdeinterlace.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstiepdeinterlace.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgstiepdeinterlace_la_SOURCES = gstiepdeinterlace.c

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstiepdeinterlace_la_CFLAGS = $(GST_CFLAGS) \
            $(GST_BASE_CFLAGS) \
            $(GST_PLUGINS_BASE_CFLAGS)
libgstiepdeinterlace_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) \
            -lgstvideo-$(GST_API_VERSION) \
            $(GST_BASE_LIBS) $(GST_LIBS)
libgstiepdeinterlace_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstiepdeinterlace_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstiepdeinterlace.h
