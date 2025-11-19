# Linux/GTK4 Port Documentation

## Overview

This document tracks the Linux port of Frontier/OPML Editor using GTK4 as the windowing toolkit. The port follows the established pattern of the Windows port, adapting Win32 API calls to GTK4 equivalents while maintaining the same core functionality.

## Port Architecture

### Design Philosophy

The Linux port maintains the same architectural approach as other platform ports:

1. **Platform-specific entry point**: `FrontierLinuxMain.c` contains GTK4-specific initialization and event handling
2. **Platform abstraction layer**: `LinuxLand.h` provides type definitions and function declarations that map GTK4 concepts to Frontier's internal APIs
3. **Shared core logic**: All business logic in `Common/source/` remains unchanged and platform-independent
4. **Build system**: Meson build system for modern, fast builds with proper dependency management

### Key Architectural Decisions

#### Window Management
- **MDI Implementation**: Using `GtkNotebook` instead of traditional MDI
  - Each document/window becomes a notebook page
  - Tabs provide easy navigation between open documents
  - More native to modern Linux desktop environments
  
#### Graphics System
- **Cairo Integration**: Replacing Win32 GDI with Cairo
  - `HDC` (Device Context) maps to `cairo_t*`
  - Drawing operations translated to Cairo API calls
  - Cairo provides cross-platform graphics with good performance

#### Event System
- **GTK Signal System**: Converting Win32 messages to GTK signals
  - Win32 message pump → GTK main loop
  - Window procedures → GTK signal handlers
  - Event translation layer in FrontierLinuxMain.c

#### Menu System
- **GMenu/GAction Framework**: Modern GTK menu approach
  - Declarative menu definition
  - Action-based instead of message-based
  - Better integration with desktop environment

## Files Created

### Build System
- `build_gtk4/meson.build` - Main build configuration
  - Defines dependencies (GTK4, Cairo, GLib, SQLite, PCRE)
  - Lists all source files
  - Configures compilation flags
  - Sets up installation rules

- `build_gtk4/README.md` - Build instructions and documentation
  - Prerequisites for different distributions
  - Build commands
  - Troubleshooting guide
  - Port status tracking

### Platform Layer
- `Common/headers/LinuxLand.h` - Platform abstraction header
  - Type definitions (`HWND`, `HDC`, `HMENU`, etc.)
  - Message constants (`WM_*`)
  - Virtual key codes
  - Function declarations for compatibility layer

- `Common/source/FrontierLinuxMain.c` - Main entry point
  - GTK application initialization
  - Main window creation
  - Event handling functions
  - Action callbacks
  - Integration with Frontier core

## Implementation Status

### Phase 1: Foundation ✓ COMPLETE
- [x] Created build system with Meson
- [x] Set up GTK4 dependencies
- [x] Created platform abstraction headers
- [x] Implemented basic application structure
- [x] Created main entry point
- [x] Integrated with Frontier core initialization

### Phase 2: Core Window System ✓ COMPLETE
- [x] Implement window creation wrapper functions
- [x] Map GTK widgets to HWND handles
- [x] Create window registry system
- [x] Implement MDI container with GtkNotebook
- [x] Port window management functions
- [x] Handle window activation/deactivation
- [x] Implement Cairo drawing context wrappers (GetDC/ReleaseDC)
- [x] Implement menu management functions
- [x] Implement message box dialogs
- [x] Implement clipboard operations
- [x] Implement timer functions
- [x] Implement file dialogs
- [ ] Handle image/bitmap operations
- [ ] Implement double-buffering for smooth updates

### Phase 4: UI Components
- [ ] Port menu creation and management
- [ ] Implement menu event handling
- [ ] Create toolbar (if used)
- [ ] Port scrollbar functionality
- [ ] Implement status bar updates
- [ ] Port text editing controls (GtkTextView)

### Phase 5: Advanced Features
- [ ] Implement file open/save dialogs
- [ ] Port clipboard operations
- [ ] Implement printing support
- [ ] Add drag-and-drop functionality
- [ ] System integration (file associations, etc.)

### Phase 6: Integration & Testing
- [ ] Test with actual Frontier scripts
- [ ] Verify database operations
- [ ] Test networking features
- [ ] Performance profiling and optimization
- [ ] Cross-distribution testing
- [ ] Documentation updates

## Technical Mapping

### Type Mappings

| Windows Type | GTK4 Type | Notes |
|--------------|-----------|-------|
| `HWND` | `GtkWidget*` | Window handle |
| `HDC` | `cairo_t*` | Drawing context |
| `HMENU` | `GMenu*` | Menu handle |
| `HICON` | `GdkPixbuf*` | Icon/image |
| `HCURSOR` | `GdkCursor*` | Cursor |
| `BOOL` | `gboolean` | Boolean type |

### Message Mappings

| Win32 Message | GTK4 Equivalent | Handler |
|---------------|-----------------|---------|
| `WM_PAINT` | `draw` signal | `WinProcessPaintEvent` |
| `WM_COMMAND` | Action activation | `WinProcessCommandEvent` |
| `WM_LBUTTONDOWN` | `button-press-event` | `WinProcessMouseEvent` |
| `WM_KEYDOWN` | `key-press-event` | `WinProcessKeyboardEvent` |
| `WM_SIZE` | `size-allocate` | Direct handling |
| `WM_CLOSE` | `close-request` | `on_window_delete_event` |

### Function Mappings (To Be Implemented)

| Win32 Function | GTK4 Equivalent | Status |
|----------------|-----------------|--------|
| `CreateWindow` | `gtk_window_new` + setup | Pending |
| `ShowWindow` | `gtk_widget_show` | Pending |
| `InvalidateRect` | `gtk_widget_queue_draw` | Pending |
| `GetDC` | Create `cairo_t` from widget | Pending |
| `ReleaseDC` | `cairo_destroy` | Pending |
| `MessageBox` | `GtkMessageDialog` | Pending |
| `SetTimer` | `g_timeout_add` | Pending |

## Build Instructions

See `build_gtk4/README.md` for detailed build instructions.

Quick start:
```bash
cd build_gtk4
meson setup builddir
meson compile -C builddir
./builddir/opml
```

## Dependencies

### Required
- GTK4 >= 4.0
- Cairo
- GLib >= 2.66
- SQLite3
- PCRE (Perl Compatible Regular Expressions)

### Optional
- MySQL client library (for MySQL integration)

## Known Challenges

### High Priority
1. **MDI Adaptation**: GtkNotebook doesn't perfectly replicate Win32 MDI behavior
   - Need to track active "child window" concept
   - Window menu management differs
   - Z-order handling works differently

2. **Drawing Performance**: Cairo vs. GDI performance characteristics
   - May need caching strategies
   - Investigate hardware acceleration options

3. **Event Translation**: Some Win32 messages don't have direct GTK equivalents
   - Will need custom event synthesis
   - Modifier key handling differs

### Medium Priority
1. **Thread Safety**: GTK4 requires UI updates on main thread
   - May need GLib idle callbacks
   - Thread synchronization for database access

2. **Font Handling**: Pango text engine differs from Win32
   - Font metrics may differ slightly
   - Need to ensure consistent text layout

3. **Coordinate Systems**: Origin and direction differences
   - GTK uses top-left origin (same as Win32)
   - But some operations may need translation

### Low Priority
1. **Resource Files**: Icons, strings, etc. need Linux equivalents
2. **Clipboard Formats**: Format translation between platforms
3. **Printing**: GTK print framework differs from Win32

## Testing Strategy

1. **Build Testing**: Verify compilation on multiple distributions
   - Ubuntu/Debian
   - Fedora/RHEL
   - Arch Linux

2. **Functional Testing**: Test each major subsystem
   - Window management
   - Menu operations
   - File operations
   - Database access
   - Script execution

3. **Integration Testing**: Compare behavior with Windows version
   - Open same files
   - Execute same scripts
   - Verify same results

4. **Performance Testing**: Profile critical operations
   - Window updates
   - Database queries
   - Script execution
   - Drawing operations

## Next Steps

### Immediate (Next Session)
1. Implement window creation and management wrapper functions
2. Create window handle registry system
3. Set up basic event connection for child windows
4. Test basic window creation and display

### Short Term
1. Implement Cairo drawing wrapper functions
2. Port critical drawing operations
3. Implement menu system
4. Create basic dialogs

### Medium Term
1. Port all UI components
2. Implement clipboard support
3. Add file dialog integration
4. Complete event handling

### Long Term
1. Performance optimization
2. Cross-distribution testing
3. Documentation
4. Package creation (deb, rpm, etc.)

## Resources

- [GTK4 Documentation](https://docs.gtk.org/gtk4/)
- [Cairo Manual](https://www.cairographics.org/manual/)
- [Meson Build System](https://mesonbuild.com/)
- [Frontier Repository](https://github.com/tedchoward/Frontier)

## Notes

This port represents a significant undertaking, as it involves:
- Learning GTK4's architecture and idioms
- Translating thousands of lines of Windows-specific code
- Maintaining compatibility with Frontier's core logic
- Ensuring performance parity with existing ports

However, the benefits include:
- Native Linux support for Frontier/OPML Editor
- Modern GTK4 UI with good desktop integration
- Cross-platform consistency (alongside Mac and Windows)
- Foundation for potential future ports (BSD, etc.)
