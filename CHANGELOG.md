# Changelog
All notable changes will be documented in this file (generated).

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
* cwork: custom cmake framework: refactr parent/internal solution system

---

## 0.14.2.47
2020-10-02 - f9c113579c80d6f2881096c7d395aa4102648c5d
### features
vinders (4):
* time: basic rational (frequency...) + rate factory (smpte, audio rates)
* time: high precision & monotonic native system clocks
* time: high-resolution stopwatch utility
* time: synchronization timer (refresh/poll sync, frame limiter...)
### fixes
vinders (2):
* cwork: git submodule utility: optionally permissive
* cwork: custom cmake framework: regroup parent/child modules in common _libs directory

---

## 0.10.0.37
2020-10-02 - 0d671633ddbd7612e802283c179bbaed2d6a76b9
### features
vinders (4):
* thread: spin-lock + recursive spin-lock (real-time mutex using polling)
* thread: semaphore utility (producer/consumer pattern)
* thread: ordered lock (mutex with lock order)
* thread: thread pool utility (async task management)

---

## 0.6.0.31
2020-10-01 - 83b7c88476372748865fdb82a186ffe7531659f5
### features
vinders (5):
* system: debug trace utility
* system: preprocessor: macros & code generators (enum serializer, flag ops, foreach, code duplicator...)
* system: CPU intrinsics API includes
* system: CPU arch + operating system detection
* system: logger utility + basic log formatter

---

## 0.1.1.16
2020-09-30 - 4f187f4f5341dfd3d1c0ae7f79313025da6c441a
### features
vinders (1):
* cwork: custom cmake framework
### fixes
vinders (1):
* cwork: cmake modules: git utilities

---