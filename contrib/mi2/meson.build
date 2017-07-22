libmi2_header_dir = pkgincludedir + '/contrib/mi2'

libmi2_enum_headers = [
  'mi2-error.h',
]

libmi2_glib_enums = gnome.mkenums('mi2-enums',
       h_template: 'mi2-enums.h.in',
       c_template: 'mi2-enums.c.in',
          sources: libmi2_enum_headers,
   install_header: true,
      install_dir: libmi2_header_dir,
)

libmi2_glib_sources = [
  'mi2-breakpoint.c',
  'mi2-breakpoint.h',
  'mi2-client.c',
  'mi2-client.h',
  'mi2-command-message.c',
  'mi2-command-message.h',
  'mi2-console-message.c',
  'mi2-console-message.h',
  'mi2-error.c',
  'mi2-error.h',
  'mi2-event-message.c',
  'mi2-event-message.h',
  'mi2-glib.h',
  'mi2-info-message.c',
  'mi2-info-message.h',
  'mi2-input-stream.c',
  'mi2-input-stream.h',
  'mi2-message.c',
  'mi2-message.h',
  'mi2-output-stream.c',
  'mi2-output-stream.h',
  'mi2-reply-message.c',
  'mi2-reply-message.h',
  'mi2-util.c',
  'mi2-util.h',
   libmi2_glib_enums[0], 
   libmi2_glib_enums[1], 
]

libmi2_deps = [
  libgio_dep,
  libgtk_dep,
]

libmi2_glib = static_library('mi2-glib', libmi2_glib_sources,
  dependencies: libmi2_deps,
  pic: true,
)

libmi2_glib_dep = declare_dependency(
  link_with: libmi2_glib,
  include_directories: include_directories('.'),
)

#TODO: tests
#TODO: vapi, gir
