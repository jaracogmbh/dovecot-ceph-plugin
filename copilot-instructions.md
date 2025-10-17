Project-aware Copilot Instructions
=================================

This repository contains the Dovecot Ceph plugins: a RADOS-backed storage plugin (rbox) and a RADOS-backed dictionary plugin. The codebase is C/C++ with Autotools, integrates tightly with Dovecot headers/APIs, uses Ceph librados, and has a GoogleTest-based test suite.

These instructions tell Copilot (and contributors) how to make changes safely and productively in this repo.


What Copilot should know first
------------------------------

- Languages: C++11 (primary), some C for Dovecot plugin glue.
- Build system: Autotools (autogen.sh → configure → make → make install).
- Key external deps: Dovecot devel headers/libs (>= 2.2.21), librados (>= 10.2.5), jansson (>= 2.9).
- Layout:
  - src/librmb: Ceph/RADOS abstraction library used by plugins.
  - src/storage-rbox: Dovecot storage plugin (mails in RADOS, indexes in FS/CephFS).
  - src/dict-rados: Dovecot dictionary plugin (omap K/V store).
  - src/tests: gtest suite (unit + optional integration tests).
- Style: .clang-format uses Google style, 120 cols, no include sorting.
- License: LGPL 2.1; preserve headers; avoid introducing incompatible code.


Build, run, and test (for humans and agents)
--------------------------------------------

- Use the existing Autotools flow. When adding source files, update the relevant Makefile.am and rerun the Autotools steps.
- configure.ac already probes Dovecot features and defines HAVE_* and DOVECOT_PREREQ guards. Prefer feature checks over hard-coding versions.
- Tests live under src/tests with vendored googletest sources. Unit tests run by default; integration tests are opt-in via --with-integration-tests.

Important compile/link flags
----------------------------

- C++: -std=c++11, -fpermissive (for Dovecot headers quirks), PIC for shared libs.
- Compiler warnings are enabled, but some are disabled due to upstream headers (see configure.ac AX_COMPILER_FLAGS_* usage).
- Dovecot includes and libs are wired through DC_DOVECOT macros from m4/dovecot.m4; don’t assume paths.


Coding guidelines (repo-specific)
---------------------------------

- Follow .clang-format. Don’t reformat unrelated code. Keep 120 column limit. Do not sort includes.
- Prefer Dovecot logging and error APIs over std::cerr/printf:
  - i_debug, i_warning, i_error, mail_storage_set_critical, mailbox_set_index_error, etc.
- Use existing debug helpers: wrap function bodies with FUNC_START()/FUNC_END() when appropriate (enabled in debug builds).
- Maintain Dovecot ABI contracts: use provided vfunc tables (e.g., mailbox_vfuncs) and stick to existing patterns in rbox.
- Wrap Dovecot-API-dependent code with the configured feature macros (examples below). Never assume headers/functions exist across all supported Dovecot versions.
- Keep exceptions inside librmb-level code; Dovecot plugin entry points should translate failures into return codes and use Dovecot error reporting.
- Namespace: C++ code under librmb uses the librmb namespace. Dovecot glue code is mostly C/C++ with extern "C" guards as needed.
- Avoid introducing new threading unless strictly required; Dovecot plugins run in-process.


Dovecot and feature guards
--------------------------

- Prefer feature tests already present in configure.ac. Common ones:
  - DOVECOT_PREREQ(2, 3)
  - HAVE_INDEX_POP3_UIDL_H
  - HAVE_ITER_FROM_INDEX_DIR
  - HAVE_MAIL_STORAGE_TRANSACTION_OLD_SIGNATURE
  - HAVE_INDEX_MAIL_INIT_OLD_SIGNATURE
  - HAVE_DICT_SWITCH_IOLOOP, HAVE_DICT_SET_TIMESTAMP, etc.
- New uses of Dovecot APIs should be wrapped like existing patterns in src/storage-rbox and guarded with the corresponding HAVE_* macros. If you add a new check, update configure.ac accordingly.


Ceph/RADOS usage patterns
-------------------------

- Use librmb abstractions (src/librmb) instead of using raw librados directly from plugin code, unless extending librmb itself.
- Connections are opened via RadosStorageImpl::open_connection and configured using RadosDovecotCephCfgImpl. Respect the wait method (WAIT_FOR_SAFE_AND_CB vs WAIT_FOR_COMPLETE_AND_CB) from config.
- Always call rbox_open_rados_connection() from storage code when you need RADOS access; it sets up namespaces and alternate pools correctly.
- Metadata storage: use RadosMetadataStorageImpl via r_storage->ms and avoid ad-hoc omap access in plugin layers.


Editing guidance and safe-change checklist
-----------------------------------------

- Small, focused changes. Avoid API or vfunc signature changes unless necessary; if required, update all call sites and tests.
- When adding files:
  - Place C++ sources/headers in the appropriate subdir.
  - Update the subdir’s Makefile.am to include sources in the correct libtool target.
  - If new Dovecot APIs are used, add the feature probe(s) to configure.ac.
- Error handling: return negative errno for librados/librmb failures (e.g., -ENOENT) and translate to Dovecot error reporting in the caller.
- Don’t break existing behavior around:
  - Mailbox GUID generation and persistence in index headers.
  - Namespace assignment via RadosNamespaceManager.
  - Alt storage semantics (alt_dir) and ceph index delete behavior on INBOX removal.
- Tests: add unit tests under src/tests reflecting new behavior where feasible; gate integration tests behind BUILD_INTEGRATION_TESTS.


Common tasks Copilot can help with
----------------------------------

- Add a new utility in librmb:
  - Implement under src/librmb, export via a header, add to Makefile.am, and write a small gtest in src/tests/librmb.
  - Prefer not to leak exceptions across C APIs; provide result codes.

- Extend rbox storage behavior:
  - Implement in src/storage-rbox, wire through rbox_mailbox_vfuncs if adding a vfunc behavior, and guard with Dovecot feature macros.
  - Use read_plugin_configuration to pull new settings and respect existing environment-driven overrides (plugin_envs with rbox_ceph_client.*).

- Add a Dovecot dictionary feature:
  - Implement under src/dict-rados, keep KV operations via librados (or via a thin librmb helper if reusable), and update Makefile.am.


Examples: version/feature guards
--------------------------------

When using a Dovecot symbol that might not exist everywhere, follow these patterns (described, not literal code):

- Optional includes: wrap them with the HAVE_FOO macro your configure.ac defines (e.g., HAVE_INDEX_POP3_UIDL_H) so the build works across versions.
- Version branching: prefer DOVECOT_PREREQ(2, 3) style guards to select code paths for newer versus older Dovecot releases.
- If you need a new HAVE_* guard, add an AC_MSG_CHECKING/AS_IF or AC_CHECK_* probe in configure.ac and use that macro in the code.


Logging patterns
----------------

- Prefer Dovecot loggers: i_debug/i_warning/i_error.
- For mailbox/index errors use mailbox_set_index_error, mail_storage_set_critical, etc.
- Use FUNC_START()/FUNC_END() macros in debug builds for traceability (see src/storage-rbox/debug-helper.h).


Tests: structure and expectations
---------------------------------

- Unit tests link against librmb and (for plugin-adjacent tests) the storage/dict plugin libs.
- GoogleTest source is vendored under src/tests/googletest; no external dependency is required at build time.
- Add new tests near similar ones, e.g. src/tests/librmb for librmb utilities, storage-mock-rbox for rbox logic.
- Keep tests deterministic and independent of a real Ceph cluster unless marked as integration tests.


Repo quick-reference
--------------------

- m4/dovecot.m4: detects Dovecot location and injects include/lib flags.
- configure.ac: defines feature checks and HAVE_* macros; update this when using new Dovecot symbols.
- src/storage-rbox/rbox-storage.cpp: main storage vfuncs and mailbox lifecycle.
- src/librmb/*: Ceph abstractions; extend here before using raw librados elsewhere.
- src/dict-rados/*: dictionary plugin sources.
- src/tests/*: unit and integration test sources, plus mocks.


When to ask (or open a follow-up)
---------------------------------

- Unclear target Dovecot version or feature guard for a new API.
- Introducing a new config option or on-disk/on-object format change.
- Behavioral changes around mailbox GUIDs, namespace mapping, or alt storage.


Copyright and headers
---------------------

- All new source files must include the existing license header and comply with LGPL 2.1.
- Keep attribution consistent with existing files.


Acceptance criteria for changes
-------------------------------

- Build succeeds with Autotools on a system with dependencies installed.
- Lint/formatting: .clang-format respected; no unrelated reformatting.
- Tests: existing tests still pass; new behavior covered by tests where applicable.
- Backward-compatible across supported Dovecot versions via feature guards.


Notes for tooling
-----------------

- Do not replace Autotools with CMake or Meson.
- Do not introduce new external runtime dependencies without discussion.
- Keep commits cohesive and reference the area changed (librmb, storage-rbox, dict-rados, tests).
