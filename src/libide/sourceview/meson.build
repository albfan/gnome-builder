sourceview_headers = [
  'ide-completion-item.h',
  'ide-completion-provider.h',
  'ide-completion-results.h',
  'ide-cursor.h',
  'ide-indenter.h',
  'ide-language.h',
  'ide-source-map.h',
  'ide-source-style-scheme.h',
  'ide-source-view-mode.h',
  'ide-source-view.h',
]

sourceview_sources = [
  'ide-completion-item.c',
  'ide-completion-provider.c',
  'ide-completion-results.c',
  'ide-cursor.c',
  'ide-indenter.c',
  'ide-language.c',
  'ide-source-map.c',
  'ide-source-style-scheme.c',
  'ide-source-view-mode.c',
  'ide-source-view.c',
]

sourceview_private_sources = [
  'ide-completion-words.c',
  'ide-completion-words.h',
  'ide-omni-gutter-renderer.c',
  'ide-omni-gutter-renderer.h',
  'ide-line-change-gutter-renderer.c',
  'ide-line-change-gutter-renderer.h',
  'ide-source-iter.c',
  'ide-source-iter.h',
  'ide-source-view-capture.c',
  'ide-source-view-capture.h',
  'ide-source-view-movements.c',
  'ide-source-view-movements.h',
  'ide-source-view-private.h',
  'ide-source-view-shortcuts.c',
  'ide-text-iter.c',
  'ide-text-iter.h',
  'ide-text-util.c',
  'ide-text-util.h',
  'ide-word-completion-provider.c',
  'ide-word-completion-provider.h',
  'ide-word-completion-results.c',
  'ide-word-completion-results.h',
  'ide-word-completion-item.c',
  'ide-word-completion-item.h',
]

sourceview_enums = [
  'ide-cursor.h',
  'ide-source-view.h',
]

libide_public_headers += files(sourceview_headers)
libide_public_sources += files(sourceview_sources)
libide_private_sources += files(sourceview_private_sources)
libide_enum_headers += files(sourceview_enums)

install_headers(sourceview_headers, subdir: join_paths(libide_header_subdir, 'sourceview'))
