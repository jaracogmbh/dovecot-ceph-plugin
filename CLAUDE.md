# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Dovecot plugin that enables email storage in Ceph RADOS objects. It provides two main plugins:
- **RADOS Storage Plugin (rbox)**: Stores emails as RADOS objects while keeping metadata in filesystem/CephFS
- **RADOS Dictionary Plugin**: Stores Dovecot dictionaries in Ceph OMAP key/value store

The project uses a hybrid storage model where emails are immutable RADOS objects and index/cache data remains in the filesystem for performance.

## Build System

The project uses GNU Autotools (autoconf/automake) for configuration and building:

### Build Commands
```bash
# Generate build system
./autogen.sh

# Configure build (standard installation)
./configure

# Configure with custom dovecot location
./configure --with-dovecot=/path/to/dovecot

# Configure with custom prefix
./configure --prefix=/usr/local

# Build
make

# Install
make install
```

### Build Options
- `--with-dict=yes/no`: Build RADOS dictionary plugin (default: yes)
- `--with-storage=yes/no`: Build RADOS storage plugin (default: yes)
- `--with-tests=yes/no`: Build tests (default: yes)
- `--with-integration-tests=yes/no`: Build integration tests (default: no)

## Dependencies

Required packages:
- `libjansson-devel` (>= 2.9)
- `librados2-devel` (>= 10.2.5)
- `dovecot-devel`
- `spdlog` (optional, for logging)

## Testing

### Unit Tests
```bash
# Run individual test executables from src/tests/
./src/tests/storage-rbox/it_test_storage_rbox
./src/tests/storage-rbox/it_test_read_mail_rbox
./src/tests/librmb/it_test_librmb
```

### Integration Tests
Enable with `--with-integration-tests=yes` during configure.

### Docker Development Environment
```bash
cd docker
docker-compose up
# Access container: docker exec -it dovecot-dev bash
```

## Code Architecture

### Core Components

#### librmb (src/librmb/)
Core library providing RADOS abstractions:
- `rados-cluster-impl`: Ceph cluster connection management
- `rados-storage-impl`: RADOS storage operations
- `rados-mail-impl`: Mail object handling
- `rados-dictionary-impl`: Dictionary operations
- `rados-metadata-*`: Metadata storage implementations

#### Storage Plugin (src/storage-rbox/)
Dovecot storage plugin implementation:
- `rbox-storage.cpp`: Main storage backend
- `rbox-mail.cpp`: Mail object interface
- `rbox-save.cpp`: Mail saving logic
- `rbox-sync.cpp`: Mailbox synchronization
- `rbox-copy.cpp`: Mail copying operations

#### Dictionary Plugin (src/dict-rados/)
Dovecot dictionary plugin for OMAP storage:
- `dict-rados.cpp`: Dictionary implementation
- `libdict-rados-plugin.c`: Plugin interface

#### Tools (src/librmb/tools/rmb/)
Command-line tools for managing RADOS mailboxes:
- `rmb.cpp`: Main RMB tool
- `rmb-commands-impl.cpp`: Command implementations

### Key Interfaces

The code implements Dovecot's plugin interfaces:
- `mail_storage_vfuncs`: Storage operations
- `mail_vfuncs`: Mail object operations  
- `dict_vfuncs`: Dictionary operations

### Configuration

Configuration is handled through:
- Dovecot configuration files
- Ceph configuration files (`ceph.conf`)
- JSON configuration for advanced settings

## Development Guidelines

### Code Style
- C++11 standard
- Line length: 120 characters (see CPPLINT.cfg)
- Use existing code patterns and conventions
- Headers are organized by component

### Testing Strategy
- Unit tests for individual components
- Integration tests for end-to-end functionality
- Manual testing with IMAP clients (Thunderbird, telnet)
- Performance testing with imaptest

### Memory Management
- RAII patterns for resource management
- Proper cleanup of Ceph contexts
- Buffer management for large mail objects

## Important Notes

- This is experimental code, not production-ready
- Mail objects are immutable once stored
- Index data should be on fast storage (SSD/CephFS)
- Mail objects can use erasure coding for efficiency
- The plugin supports both standalone and clustered Dovecot deployments