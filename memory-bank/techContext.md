# Technical Context: Frontier/OPML Editor

## Technologies Used

### Programming Languages

- **C/C++**: Primary implementation languages for the core codebase
- **Custom Scripting Language**: Frontier's own scripting language for automation and extensions
- **HTML/CSS/JavaScript**: Used for web-related features and interface elements

### Frameworks & Libraries

- **IowaRuntime**: Framework for user interface components and rendering
- **PCRE (Perl Compatible Regular Expressions)**: For text pattern matching and manipulation
- **MySQL Client Library**: Optional integration for MySQL database connectivity
- **SQLite**: Embedded SQL database engine integration
- **FSCopyObject**: File system manipulation utilities
- **MoreFiles**: Extended file system functionality

### Storage Systems

- **Object Database**: Proprietary hierarchical object storage system
- **MySQL**: Optional external relational database support
- **SQLite**: Embedded relational database support
- **File System**: Direct file storage and manipulation

### Platform Technologies

#### macOS/Mac OS X
- **Carbon**: Used for older Mac OS X compatibility
- **Cocoa**: Modern macOS API integration
- **Universal Binary Support**: For compatibility with both PowerPC and Intel processors

#### Windows
- **Win32 API**: Core Windows platform integration
- **COM/OLE**: For system integration and interoperability

## Development Setup

### Build Environments

1. **Xcode (Mac)**
   - Supports both Xcode project formats (.xcode and .xcodeproj)
   - Includes development and deployment build configurations
   - Supports Universal Binary compilation for cross-architecture compatibility

2. **Visual Studio (Windows)**
   - Multiple Visual Studio versions supported (6.0, .NET 2003, .NET 2005)
   - Debug and Release build configurations
   - Windows-specific optimizations and features

3. **CodeWarrior Pro**
   - Cross-platform development environment
   - Supports both Mac and Windows targets
   - Requires specific versions (e.g., CodeWarrior Pro 8 with 8.3 updates)

4. **GNU Tools**
   - Support for building with gcc and related tools
   - Primarily for Mac OS X, with potential for expansion to other platforms
   - Makefile-based build system

### Development Requirements

- **macOS Development**:
  - Mac OS X with appropriate SDK (10.4, 10.6 required for certain builds)
  - Xcode or CodeWarrior Pro installed
  - Command line tools for GNU-based builds

- **Windows Development**:
  - Windows with appropriate Visual Studio version
  - Windows SDK components
  - Optional CodeWarrior Pro for cross-platform development

### Project Structure

```
/
├── build_CWPro8/      # CodeWarrior Pro 8 build files
├── build_GNU/         # GNU tools build files
├── build_VC6/         # Visual Studio 6.0 build files
├── build_VC2K3/       # Visual Studio .NET 2003 build files  
├── build_VC2K5/       # Visual Studio .NET 2005 build files
├── build_VC2K8/       # Visual Studio .NET 2008 build files
├── build_Xcode/       # Xcode build files
├── Common/            # Common source code and resources
│   ├── headers/       # Header files
│   ├── source/        # Source code files
│   ├── resources/     # Resource files
│   └── ...            # Various subsystem directories
├── FrontierSDK/       # Software Development Kit
└── resources/         # Additional resources
```

## Technical Constraints

### Platform Constraints

- **macOS Compatibility**: Must support both older (PowerPC) and newer (Intel) Mac systems
- **Windows Compatibility**: Must maintain compatibility with various Windows versions
- **Cross-Platform Code**: Core functionality must work consistently across platforms

### Performance Constraints

- **Memory Management**: Must efficiently manage memory, especially for large databases
- **Processing Speed**: Script execution and database operations must meet performance targets
- **UI Responsiveness**: Interface must remain responsive during intensive operations

### Security Constraints

- **File System Access**: Requires appropriate permissions for file operations
- **Network Operations**: Must handle network security for web-related features
- **Script Execution**: Need to manage security implications of executing user scripts

### Compatibility Constraints

- **Legacy Support**: Must maintain compatibility with existing scripts and databases
- **SDK Compatibility**: Must preserve APIs for third-party extensions and integrations
- **Build Environment Support**: Must continue to support multiple development tools and environments

## Dependencies

### External Dependencies

- **Operating System APIs**: Relies on platform-specific APIs for system integration
- **Database Engines**: Optional dependencies on MySQL client libraries and SQLite
- **Regular Expression Engine**: PCRE library for pattern matching
- **Third-Party Libraries**: Various libraries for specific functionalities

### Internal Dependencies

- **Object Database ← Scripting Engine**: Scripts access and manipulate the database
- **UI System ← Object Database**: Interface displays and edits database content
- **Platform Abstraction → All Components**: Core systems rely on platform abstraction layer
- **Scripting Engine ← UI System**: User interface elements can trigger scripts

## Tool Usage Patterns

### Build Process

1. **Select Build Environment**: Choose appropriate build system based on target platform
2. **Configure Build Settings**: Set debug/release mode and other build parameters
3. **Build Application**: Compile sources and link into executable
4. **Run Tests**: Verify functionality on target platform

### Development Workflow

1. **Cross-Platform Development**: 
   - Implement core functionality in platform-neutral code
   - Add platform-specific implementations as needed
   - Test on each target platform

2. **Feature Implementation**:
   - Design feature architecture
   - Implement in appropriate subsystem
   - Add scripting interface if applicable
   - Update UI components as needed
   - Test across platforms

3. **Debugging Approach**:
   - Use platform-specific debugging tools
   - Log debugging information
   - Test edge cases and error conditions
   - Verify cross-platform behavior

### Deployment Process

1. **Build Release Versions**: Create optimized builds for distribution
2. **Package Application**: Include necessary resources and configuration files
3. **Create Installation Package**: Prepare platform-appropriate installer
4. **Distribution**: Publish to appropriate channels

## Technical Debt & Evolution

The project carries technical debt from its long history:

- **Legacy Code**: Portions written for older systems require maintenance
- **Cross-Platform Complexity**: Supporting multiple platforms increases complexity
- **Multiple Build Systems**: Maintaining various build configurations adds overhead
- **Deprecated APIs**: Some platform APIs used may be deprecated or obsolete

Ongoing technical evolution focuses on:

- Modernizing the codebase while preserving compatibility
- Improving build processes and development workflows
- Addressing technical debt in key areas
- Ensuring compatibility with current operating systems
