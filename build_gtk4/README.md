# Building Frontier/OPML Editor for Linux with GTK4

This directory contains the build configuration for the Linux port of Frontier/OPML Editor using GTK4.

## Prerequisites

### Required Dependencies

You'll need the following development packages installed:

**Ubuntu/Debian:**
```bash
sudo apt-get install \
    meson \
    ninja-build \
    libgtk-4-dev \
    libcairo2-dev \
    libglib2.0-dev \
    libsqlite3-dev \
    libpcre3-dev \
    build-essential
```

**Fedora/RHEL:**
```bash
sudo dnf install \
    meson \
    ninja-build \
    gtk4-devel \
    cairo-devel \
    glib2-devel \
    sqlite-devel \
    pcre-devel \
    gcc \
    gcc-c++
```

**Arch Linux:**
```bash
sudo pacman -S \
    meson \
    ninja \
    gtk4 \
    cairo \
    glib2 \
    sqlite \
    pcre \
    base-devel
```

### Optional Dependencies

**MySQL Support** (optional):
- Ubuntu/Debian: `sudo apt-get install libmysqlclient-dev`
- Fedora/RHEL: `sudo dnf install mysql-devel`
- Arch: `sudo pacman -S mariadb-libs`

## Building

### Quick Start

```bash
# From the build_gtk4 directory
meson setup builddir
meson compile -C builddir
```

### Configuration Options

You can configure the build with various options:

```bash
# Set build type (debug or release)
meson setup builddir --buildtype=release

# Custom installation prefix
meson setup builddir --prefix=/usr/local

# View all configuration options
meson configure builddir
```

### Building in Debug Mode

For development, build in debug mode with additional checks:

```bash
meson setup builddir --buildtype=debug
meson compile -C builddir
```

## Installation

```bash
# Install to the configured prefix
meson install -C builddir

# Or with sudo if installing system-wide
sudo meson install -C builddir
```

## Running

After building, you can run the application:

```bash
# From build directory
./builddir/opml

# Or if installed
opml
```

## Project Structure

```
build_gtk4/
├── meson.build           # Main build configuration
├── README.md             # This file
└── builddir/             # Build output (created by meson)
```

## Port Status

This is an initial port of Frontier to Linux/GTK4. Current status:

### Completed ✓
- [x] Build system (Meson)
- [x] Main application skeleton
- [x] GTK4 integration
- [x] Basic window management
- [x] Event handling framework
- [x] Platform abstraction headers

### In Progress 🔧
- [ ] Window management system
- [ ] Graphics/drawing (Cairo integration)
- [ ] Menu system
- [ ] Dialog boxes
- [ ] Text editing controls
- [ ] Clipboard operations
- [ ] File operations
- [ ] Printing support

### Platform-Specific Notes

1. **Window System**: Uses GtkNotebook for MDI instead of native MDI
2. **Drawing**: Cairo replaces Win32 GDI
3. **Menus**: GMenu/GAction framework instead of traditional menus
4. **Dialogs**: Native GTK dialogs

## Development

### Adding New Source Files

To add new source files to the build, edit `meson.build`:

```python
linux_sources += files(
    common_src + '/your_new_file.c',
)
```

### Debugging

To debug with gdb:

```bash
meson setup builddir --buildtype=debug
meson compile -C builddir
gdb ./builddir/opml
```

Or use the GTK_DEBUG environment variable:

```bash
GTK_DEBUG=interactive ./builddir/opml
```

### Code Style

Follow the existing code style in the Frontier codebase:
- C99 standard
- BSD/K&R style indentation
- Platform-specific code isolated in LinuxLand.h/FrontierLinuxMain.c

## Testing

Currently, testing is manual. To test basic functionality:

1. Launch the application: `./builddir/opml`
2. Verify the main window appears
3. Test File → About dialog
4. Test File → Quit

## Known Issues

1. Many features not yet implemented (see "In Progress" above)
2. Some platform-specific code still needs adaptation
3. Performance not yet optimized
4. Resource files (icons, etc.) not yet integrated

## Contributing

When contributing to the Linux port:

1. Keep platform-specific code in FrontierLinuxMain.c
2. Use the abstraction layer defined in LinuxLand.h
3. Follow the patterns established in the Windows port
4. Test on multiple distributions if possible
5. Document any new dependencies

## Troubleshooting

### Build Fails with GTK4 Not Found

Make sure you have `libgtk-4-dev` (or equivalent) installed:
```bash
pkg-config --modversion gtk4
```

### Linking Errors

Try cleaning and rebuilding:
```bash
rm -rf builddir
meson setup builddir
meson compile -C builddir
```

### Runtime Errors

Check if all required libraries are present:
```bash
ldd ./builddir/opml
```

## Resources

- [Frontier Repository](https://github.com/tedchoward/Frontier)
- [GTK4 Documentation](https://docs.gtk.org/gtk4/)
- [Meson Build System](https://mesonbuild.com/)
- [Cairo Graphics](https://www.cairographics.org/)

## License

This port follows the same GPL v2 license as the main Frontier project.

## Contact

For questions about the Linux port, please open an issue on the GitHub repository.
