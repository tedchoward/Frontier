project('frontier', 'c',
  version: '10.2.0',
  default_options: ['c_std=gnu99', 'warning_level=2']
)

# GTK4 and dependencies
gtk4_dep = dependency('gtk4', version: '>= 4.0')
cairo_dep = dependency('cairo')
glib_dep = dependency('glib-2.0', version: '>= 2.66')
sqlite3_dep = dependency('sqlite3')
pcre_dep = dependency('libpcre')
thread_dep = dependency('threads')

# MySQL is optional
mysql_dep = dependency('mysqlclient', required: false)

# Source directories
common_src = '../Common/source'
common_headers = '../Common/headers'

# Include directories
inc_dirs = include_directories(
  common_headers,
  '../Common/IowaRuntime',
  '../Common/PCRE',
  '../Common/sqlite3'
)

# Platform-specific defines
add_project_arguments(
  '-D__linux__',
  '-DFRONTIERLINUX',
  '-DPIKE',
  language: 'c'
)

if mysql_dep.found()
  add_project_arguments('-DMYSQL_SUPPORT', language: 'c')
endif

# Core source files (platform-independent)
core_sources = files(
  common_src + '/about.c',
  common_src + '/assert.c',
  common_src + '/base64.c',
  common_src + '/bitmaps.c',
  common_src + '/browserverbs.c',
  common_src + '/cancoon.c',
  common_src + '/cancoonpopup.c',
  common_src + '/cancoonverbs.c',
  common_src + '/cancoonwindow.c',
  common_src + '/claybrowserexpand.c',
  common_src + '/claybrowserstruc.c',
  common_src + '/claybrowservalidate.c',
  common_src + '/claycallbacks.c',
  common_src + '/claylinelayout.c',
  common_src + '/command.c',
  common_src + '/config.c',
  common_src + '/cursor.c',
  common_src + '/db.c',
  common_src + '/dbstats.c',
  common_src + '/dbverbs.c',
  common_src + '/dialogs.c',
  common_src + '/error.c',
  common_src + '/file.c',
  common_src + '/filealias.c',
  common_src + '/filedialog.c',
  common_src + '/fileloop.c',
  common_src + '/filepath.c',
  common_src + '/fileverbs.c',
  common_src + '/findinfile.c',
  common_src + '/font.c',
  common_src + '/frontierconfig.c',
  common_src + '/frontierdebug.c',
  common_src + '/frontierstart.c',
  common_src + '/frontierwindows.c',
  common_src + '/icon.c',
  common_src + '/kb.c',
  common_src + '/lang.c',
  common_src + '/langcallbacks.c',
  common_src + '/langcard.c',
  common_src + '/langcrypt.c',
  common_src + '/langdate.c',
  common_src + '/langdialog.c',
  common_src + '/langerror.c',
  common_src + '/langerrorwindow.c',
  common_src + '/langevaluate.c',
  common_src + '/langexternal.c',
  common_src + '/langhash.c',
  common_src + '/langhtml.c',
  common_src + '/langipc.c',
  common_src + '/langlist.c',
  common_src + '/langmath.c',
  common_src + '/langmodeless.c',
  common_src + '/langops.c',
  common_src + '/langpack.c',
  common_src + '/langregexp.c',
  common_src + '/langscan.c',
  common_src + '/langsqlite.c',
  common_src + '/langstartup.c',
  common_src + '/langsystem7.c',
  common_src + '/langsystypes.c',
  common_src + '/langtmpstack.c',
  common_src + '/langtrace.c',
  common_src + '/langtree.c',
  common_src + '/langvalue.c',
  common_src + '/langverbs.c',
  common_src + '/langxml.c',
  common_src + '/md5.c',
  common_src + '/memory.c',
  common_src + '/menu.c',
  common_src + '/menubar.c',
  common_src + '/menueditor.c',
  common_src + '/menufind.c',
  common_src + '/menupack.c',
  common_src + '/menuresize.c',
  common_src + '/menuverbs.c',
  common_src + '/meprograms.c',
  common_src + '/miniwindow.c',
  common_src + '/mouse.c',
  common_src + '/mutex.c',
  common_src + '/notify.c',
  common_src + '/odbengine.c',
  common_src + '/op.c',
  common_src + '/opbuttons.c',
  common_src + '/opdisplay.c',
  common_src + '/opdraggingmove.c',
  common_src + '/opedit.c',
  common_src + '/opexpand.c',
  common_src + '/ophoist.c',
  common_src + '/opicons.c',
  common_src + '/opinit.c',
  common_src + '/oplangtext.c',
  common_src + '/oplineheight.c',
  common_src + '/oplist.c',
  common_src + '/opops.c',
  common_src + '/oppack.c',
  common_src + '/oppopup.c',
  common_src + '/opprint.c',
  common_src + '/oprefcon.c',
  common_src + '/ops.c',
  common_src + '/opscreenmap.c',
  common_src + '/opscrollbar.c',
  common_src + '/opstructure.c',
  common_src + '/opvalidate.c',
  common_src + '/opverbs.c',
  common_src + '/opvisit.c',
  common_src + '/opxml.c',
  common_src + '/pict.c',
  common_src + '/pictverbs.c',
  common_src + '/popup.c',
  common_src + '/process.c',
  common_src + '/quickdraw.c',
  common_src + '/resources.c',
  common_src + '/scrap.c',
  common_src + '/scripts.c',
  common_src + '/scrollbar.c',
  common_src + '/search.c',
  common_src + '/sha1dgst.c',
  common_src + '/shell.c',
  common_src + '/shellactivate.c',
  common_src + '/shellblocker.c',
  common_src + '/shellbuttons.c',
  common_src + '/shellcallbacks.c',
  common_src + '/shellfile.c',
  common_src + '/shellhooks.c',
  common_src + '/shelljuggler.c',
  common_src + '/shellkb.c',
  common_src + '/shellmenu.c',
  common_src + '/shellmouse.c',
  common_src + '/shellops.c',
  common_src + '/shellprint.c',
  common_src + '/shellscrap.c',
  common_src + '/shellscroll.c',
  common_src + '/shellsysverbs.c',
  common_src + '/shellundo.c',
  common_src + '/shellupdate.c',
  common_src + '/shellverbs.c',
  common_src + '/shellwindow.c',
  common_src + '/shellwindowmenu.c',
  common_src + '/shellwindowverbs.c',
  common_src + '/strings.c',
  common_src + '/stringverbs.c',
  common_src + '/sysshellcall.c',
  common_src + '/tablecallbacks.c',
  common_src + '/tablecompare.c',
  common_src + '/tabledisplay.c',
  common_src + '/tableedit.c',
  common_src + '/tableexternal.c',
  common_src + '/tablefind.c',
  common_src + '/tableformats.c',
  common_src + '/tablelayout.c',
  common_src + '/tablenewvalue.c',
  common_src + '/tableops.c',
  common_src + '/tablepack.c',
  common_src + '/tablepopup.c',
  common_src + '/tableprint.c',
  common_src + '/tablerunbutton.c',
  common_src + '/tablescrap.c',
  common_src + '/tablestructure.c',
  common_src + '/tablevalidate.c',
)

# MySQL support (optional)
if mysql_dep.found()
  core_sources += files(common_src + '/langmysql.c')
endif

# Linux-specific sources
linux_sources = files(
  common_src + '/FrontierLinuxMain.c',
  common_src + '/LinuxPlatform.c',
)

# IowaRuntime library sources
iowa_sources = files(
  '../Common/IowaRuntime/iowagraphics.c',
  '../Common/IowaRuntime/iowapictures.c',
  '../Common/IowaRuntime/iowastrings.c',
)

# PCRE library sources (if not using system PCRE)
pcre_sources = files(
  '../Common/PCRE/pcre.c',
  '../Common/PCRE/study.c',
)

# Build executable
executable('opml',
  core_sources,
  linux_sources,
  iowa_sources,
  dependencies: [
    gtk4_dep,
    cairo_dep,
    glib_dep,
    sqlite3_dep,
    pcre_dep,
    thread_dep,
    mysql_dep
  ],
  include_directories: inc_dirs,
  install: true
)

# Installation
install_data(
  '../resources/opml.root',
  install_dir: get_option('datadir') / 'opml'
)
