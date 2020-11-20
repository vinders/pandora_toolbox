# Changelog
All notable changes will be documented in this file (generated).

## 0.48.1.207
2020-11-19 - 0e2640d7f296932c306ffee2d1a5c3a3a1b728ec
### features
vinders (5):
* cwork: custom cmake framework: support custom/shader/resource files + test-only dependencies
* cwork: custom cmake framework: add option: minimum supported windows version (Win32 API)
* cwork/system: dynamic library import/export definitions
* video: openGL/vulkan API includes
* video: shader generation script (assemble modules + remove comments/white-spaces + vulkan spir-v compilation)
### fixes
vinders (6):
* video: shader generation script: add D3D11 fxc shader compilation
* video: shader generation script: add D3D12 dxc shader compilation
* system: macros & code generators: add constexpr enum serializer
* io: file IO/management/metadata: improve absolute path detection
* cwork: custom cmake framework: replace global include dirs by target include dirs (to avoid file name conflicts)
* cwork: custom cmake framework: allow different scopes when linking extern libs

---

## 0.43.2.128
2020-10-21 - c22d1e7bb91530b3584974bbb3b0a10e49288a7e
### features
vinders (12):
* thread: thread scheduling priority setter
* logic: general math algorithms: GCD, power of 2, near equality
* logic: string manipulation: length, pad, trim...
* logic: interval search algorithms (for sorted arrays)
* logic: sorting algorithms (heap-sort, quick-sort, bubble, insert, binary-insert)
* logic: search/sort algorithm benchmark tool
* memory: endianness conversion utils
* memory: memory register component (multi-size access)
* memory: fixed-size circular queue (stack alloc)
* memory: fixed-size vector template (stack alloc)
* memory: fixed-size string (stack alloc)
* memory: preallocated memory pool (stack/heap)
### fixes
vinders (2):
* hardware: CPU cores detection: support modern AMD
* scripts: version release script (version/changelog/tag + merge): push changelog on develop

---

## 0.31.0.103
2020-10-11 - ea4aba33ade990f10db821e3b72b606a90832d82
### features
vinders (9):
* system: macros & code generators: enum serialization with buffer
* system: date/time formatting + reading
* io: CSV log output formatter
* io: file system IO/creation/removal + entry metadata/type/access/content reading + path management
* io: file system known location finder
* hardware: process / thread CPU affinity tools
* hardware: CPUID register reader (x86/ARM) & CPU info file reader (Linux)
* hardware: CPU arch: cores detection
* hardware: CPU specifications detection (x86/ARM)
### fixes
vinders (2):
* system: logger + basic formatter: enum serialization with buffer (no alloc)
* system: logger + basic formatter: add current date/time

---

## 0.22.1.59
2020-10-04 - 0cc828d8a1f6187fb040ffec8690212627b5dc86
### features
vinders (8):
* pattern: custom type traits (copy/move/compare/iterator) + copy/move enablers
* pattern: global singleton + locked singleton patterns
* pattern: optional value pattern (pointer-like buffer, stack alloc)
* pattern: locked object pattern (safe lifetime lock + simple ref lock)
* pattern: pre/post operation guard pattern
* pattern: iterator/iterable pattern + declaration helpers
* pattern: RAII contract utility (for delegate,observer...) + contract registrar
* pattern: observer + delegate patterns (direct/deferrable)
### fixes
vinders (1):
* cwork: custom cmake framework: refactor parent/internal solution system

---

## 0.14.2.47
2020-10-02 - f9c113579c80d6f2881096c7d395aa4102648c5d
### features
vinders (8):
* thread: spin-lock + recursive spin-lock (real-time mutex using polling)
* thread: semaphore utility (producer/consumer pattern)
* thread: ordered lock (mutex with lock order)
* thread: thread pool utility (async task management)
* time: basic rational (frequency...) + rate factory (smpte, audio rates)
* time: high precision & monotonic native system clocks
* time: high-resolution stopwatch utility
* time: synchronization timer (refresh/poll sync, frame limiter...)
### fixes
vinders (2):
* cwork: git submodule utility: optionally permissive
* cwork: custom cmake framework: regroup parent/child modules in common _libs directory

---

## 0.6.1.31
2020-10-01 - 83b7c88476372748865fdb82a186ffe7531659f5
### features
vinders (6):
* cwork: custom cmake framework
* system: debug trace utility
* system: preprocessor: macros & code generators (enum serializer, flag ops, foreach, code duplicator...)
* system: CPU intrinsics API includes
* system: CPU arch + operating system detection
* system: logger utility + basic log formatter
### fixes
vinders (1):
* cwork: cmake modules: git utilities

---