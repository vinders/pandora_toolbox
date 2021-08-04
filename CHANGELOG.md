# Changelog
All notable changes will be documented in this file (generated).

## 0.100.5.707
2021-08-04 - 89d8a0e02078a6e1bfa82fe1fbb0f42398c92c6e
### features
vinders (20):
* video: window resource: menu/sub-menu/text-item/checkbox/radio-item creation + state/check change support
* video: component format (color/depth/stencil): portable bindings for D3D/OpenGL/Vulkan
* video: Direct3D11 rendering device/context
* video: Direct3D11 swap-chain creation (framebuffers)
* video: Direct3D11 swap-chain/framebuffer: management/resizing + buffer swapping
* video: Direct3D11 rendering device/context: render-target activation + rasterizer state creation/settings
* video: Direct3D11 rendering device/context: filter/sampler builder (linear/anisotropic/compared)
* video: Direct3D11 shader builder/compiler/binding + input layouts
* video: rendering viewport (region for 3D projection + near/far clipping)
* memory: light string/wstring implementations (to avoid STL overhead)
* system: portable data alignment macro: __align_type(size,def)
* video: Direct3D11 / OpenGL4 / Vulkan enum definitions (same labels)
* video: Direct3D11 depth/stencil buffer and view + depth/stencil states
* video: Direct3D11 constant data buffer (for shaders)
* video: Direct3D11 static/dynamic vertex/index/constant buffers
* video: Direct3D11 vertex buffer drawing (base/indexed/instances)
* video: Direct3D11 camera screen projection (ratio, fov, near/far-planes)
* video: Direct3D11 rendering device/context: blend state builder (grouped/separate/per-target)
* video: Direct3D11 1D/2D/3D/target texture buffers + params
* video: Direct3D11 texture data reader (staged) + writer (static/dynamic/staged) + copy/copyRegion
### fixes
vinders (11):
* video: window manager + builder for Windows: fix window fullscreen restore with clipped cursor
* video: Direct3D11: remove outdated multisampling mode
* pattern: advanced type traits: fix move detection for new MSVC compiler
* video: window manager + builder: remove scrollbar support (not very useful -> improve speed + reduce weight)
* video: drop OpenGLES support + add Vulkan support
* video: refactor Direct3D11 renderer/swap-chain/state factory: improve efficiency, reduce weight, make interface usable for vulkan/openGL
* video: Direct3D11 viewport: reimplement viewport per API
* cwork: custom cmake framework: fix child solution system: always include child libs
* video: Direct3D11 render state arrays: avoid auto-release when reading with at(index)
* video: Direct3D11 texture containers: allow creation with external data (created with loaders such as DDSTextureLoader)
* video: modal message-box: add event flush method

---

## 0.80.2.502
2021-05-27 - 316f77782f0e0bb156ec8e332bceb276450cd5ec
### features
vinders (18):
* video: modal message box (Win32/Cocoa impl for windows/macOS)
* video: modal message box (UIKit impl for iOS)
* video: modal message box (java/JNI impl for android)
* video: modal message box (X11 impl for linux/unix/bsd)
* video: modal message box: allow custom labels for buttons
* cwork: custom cmake framework: add resource management (images/icons/fonts) + adapt android manifest/gradle/app-tree for resources
* cwork: custom cmake framework: add "console" build mode + set "executable" builds in GUI mode
* video: window virtual-keycode mappings (windows/macOS/iOS/android)
* video: window resource builder (icon/cursor/brush): Win32 impl (windows)
* video: window input events & handlers + event data helpers (window/keyboard/mouse/tch-scr)
* video: window manager + builder (Win32 impl for windows)
* video: window manager + builder for Windows: add mouse capture + raw input mode
* video: screen-saver: disable/restore + ping activity (reset idle timer)
* io: serialization value tree (multi-type value/array/object) + key-value serializer/deserializer interface
* io: INI config format serializer/deserializer
* io: JSON format serializer/deserializer
* io: encoder/decoder/converter/detection (UTF-8/UTF-16BE/UTF-16LE)
* io: INI/JSON deserializers: support unicode characters (<backslash> x<hex*1-4>, u<hex*4>, U<hex*8>)
### fixes
vinders (5):
* video: window manager + builder for Windows: refactor + fix fullscreen issues
* video: window manager + builder for Windows: support parent window close + fix size calculations
* video: window manager + builder for Windows: fix client area when adding scrollbars
* video: window manager + builder for Windows: remove useless mutex to improve efficiency
* video: screen-saver disable/restore/ping: use window handle as argument + fix handle type on Wayland

---

## 0.62.1.352
2021-04-24 - 40c1a8290e3244e1c58415bf3653303422657d17
### features
vinders (14):
* hardware: display monitor manager (adapter info, DPI, area, get/set display mode)
* hardware: add display monitors to hardware analyzer
* hardware: display monitor manager (Win32 impl for windows)
* hardware: display monitor manager (X11 impl for linux/bsd/unix)
* hardware: display monitor manager (Cocoa impl for macOS)
* hardware: display monitor manager (UIKit impl for iOS)
* cwork: custom cmake framework: add options: C++20, X11/Wayland
* cwork: custom cmake framework: add linux-wayland protocol file generator
* cwork: custom cmake framework: add possibility for modules to add source files to project
* hardware: display monitor manager (Wayland impl for Linux)
* system: android API: global container for application state
* hardware: display monitor manager (JNI impl for android)
* cwork: custom cmake framework: generate manifest for android apps
* cwork: custom cmake framework: generate gradle builder for android apps
### fixes
vinders (3):
* io: file time reader: fix GCC/clang time conversion
* hardware: display monitor: don't list duplicate display modes + use milliHertz for rate
* cwork: custom cmake framework: build android apps in shared libs (for JVM)

---

## 0.48.1.207
2020-11-19 - 0e2640d7f296932c306ffee2d1a5c3a3a1b728ec
### features
vinders (17):
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
* cwork: custom cmake framework: support custom/shader/resource files + test-only dependencies
* cwork: custom cmake framework: add option: minimum supported windows version (Win32 API)
* cwork/system: dynamic library import/export definitions
* video: openGL API includes/bindings
* video: shader generation script (assemble modules + remove comments/white-spaces + vulkan spir-v compilation)
### fixes
vinders (8):
* hardware: CPU cores detection: support modern AMD
* scripts: version release script (version/changelog/tag + merge): push changelog on develop
* video: shader generation script: add D3D11 fxc shader compilation
* video: shader generation script: add D3D12 dxc shader compilation
* system: macros & code generators: add constexpr enum serializer
* io: file IO/management/metadata: improve absolute path detection
* cwork: custom cmake framework: replace global include dirs by target include dirs (to avoid file name conflicts)
* cwork: custom cmake framework: allow different scopes when linking extern libs

---

## 0.31.0.103
2020-10-11 - ea4aba33ade990f10db821e3b72b606a90832d82
### features
vinders (17):
* pattern: custom type traits (copy/move/compare/iterator) + copy/move enablers
* pattern: global singleton + locked singleton patterns
* pattern: optional value pattern (pointer-like buffer, stack alloc)
* pattern: locked object pattern (safe lifetime lock + simple ref lock)
* pattern: pre/post operation guard pattern
* pattern: iterator/iterable pattern + declaration helpers
* pattern: RAII contract utility (for delegate,observer...) + contract registrar
* pattern: observer + delegate patterns (direct/deferrable)
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
vinders (3):
* cwork: custom cmake framework: refactor parent/internal solution system
* system: logger + basic formatter: enum serialization with buffer (no alloc)
* system: logger + basic formatter: add current date/time

---

## 0.14.2.47
2020-10-02 - f9c113579c80d6f2881096c7d395aa4102648c5d
### features
vinders (14):
* cwork: custom cmake framework
* system: debug trace utility
* system: preprocessor: macros & code generators (enum serializer, flag ops, foreach, code duplicator...)
* system: CPU intrinsics API includes
* system: CPU arch + operating system detection
* system: logger utility + basic log formatter
* thread: spin-lock + recursive spin-lock (real-time mutex using polling)
* thread: semaphore utility (producer/consumer pattern)
* thread: ordered lock (mutex with lock order)
* thread: thread pool utility (async task management)
* time: basic rational (frequency...) + rate factory (smpte, audio rates)
* time: high precision & monotonic native system clocks
* time: high-resolution stopwatch utility
* time: synchronization timer (refresh/poll sync, frame limiter...)
### fixes
vinders (3):
* cwork: cmake modules: git utilities
* cwork: git submodule utility: optionally permissive
* cwork: custom cmake framework: regroup parent/child modules in common _libs directory

---