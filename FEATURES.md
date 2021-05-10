# Feature list

<sub>**win** = *Windows* / 
**mac** = *MacOS* / 
**ios** = *iOS* / 
**and** = *Android* / 
**x11** = *Linux/Unix/BSD (X11)* / 
**wln** = *Linux (Wayland)*</sub>

<sub>![OK](_img/badges/feat_done.png) : done<br>
![BUG](_img/badges/feat_issues.png) : minor issue(s)<br>
![TEST](_img/badges/feat_not_tested.png) : not fully tested<br>
![NOT](_img/badges/feat_not_impl.png) : not implemented yet</sub>

|         hardware includes        |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *hardware/cpu_instruction_set.h* | CPU instuction set ID/family ('cpu_specs.h')| ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *hardware/cpu_specs.h*           | CPU info/specs/features reader              | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *hardware/cpu_vendor.h*          | CPU vendor (enum/labels) (for 'cpu_specs.h')| ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *hardware/cpuid_property_loca...*| CPUID reg. property ID (for 'cpu_specs.h')  | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *hardware/cpuid_registers.h*     | List of CPUID registers (for 'cpu_specs.h') | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *hardware/display_monitor.h*     | Monitor info/size/DPI + display modes       | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![TEST](_img/badges/feat_not_tested.png) | ![TEST](_img/badges/feat_not_tested.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *hardware/process_affinity.h*    | Process/thread affinity with CPU core(s)    | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |

|            io includes           |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *io/csv_log_formatter.h*         | Log formatter: CSV table format             | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *io/file_handle.h*               | Managed C file handle (RAII)                | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *io/file_system_io.h*            | File-system operations + metadata reader    | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *io/file_system_locations.h*     | Standard OS-specific location finder        | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![TEST](_img/badges/feat_not_tested.png) | ![TEST](_img/badges/feat_not_tested.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |

|          logic includes          |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *logic/math.h*                   | Math algorithms: GCD/pow2/near-equal/...    | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *logic/search.h*                 | Search algorithms: binary/jump/exp/interp.  | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *logic/sort.h*                   | Sort algorithms: linear/heap/quick-sort     | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *logic/sort_order.h*             | Data order enum (for 'search.h'/'sort.h')   | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *logic/strings.h*                | String utils: trim/pad/find/assign/...      | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |

|          memory includes         |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| list TODO                        |                                             |     |     |     |     |     |     |

|         pattern includes         |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| list TODO                        |                                             |     |     |     |     |     |     |

|          system includes         |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *system/cpu_arch.h*              | CPU arch. detection functions/constants     | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *system/date_time.h*             | Date/time string formatting                 | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *system/export.h*                | Dynamic lib import/export attributes        | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *system/force_inline.h*          | Portable '__forceinline' attribute          | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *system/logger.h*                | Simple log writer/formatter/options         | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *system/operating_system.h*      | Op. system detection function/constants     | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *system/preprocessor_tools.h*    | Macros: duplicate, enum serializer/flags... | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *system/trace.h*                 | Debug trace utilities                       | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *system/api/intrinsics.h*        | CPU intrinsics API (MMX/SSE/AVX/NEON/...)   | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![BUG](_img/badges/feat_issues.png) | ![BUG](_img/badges/feat_issues.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *system/api/android_app.h*       | Android app state container                 | ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png)
| *system/api/windows_app.h*       | Windows app instance container              | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png)
| *system/api/windows_api.h*       | Windows API, with version + options         | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png)
| *system/api/windows_base_types.h*| Minimum Windows API: types defs only        | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png)
| *system/api/windows_common_co...*| Windows common controls: API + manifest     | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png)
| *system/api/windows_version.h*   | Windows minimum system version defs         | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png) ![NOT](_img/badges/feat_not_impl.png)

|          thread includes         |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *thread/ordered_lock.h*          | Concurrency sync primitive with FIFO order  | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *thread/semaphore.h*             | Sync primitive with counter (wait/notify)   | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *thread/spin_lock.h*             | Active/polling concurrency sync primitive   | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *thread/recursive_spin_lock.h*   | Spin-lock with recursive thread ownership   | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *thread/thread_pool.h*           | Fixed-size pool of threads (async tasks)    | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *thread/thread_priority.h*       | Set thread scheduler priority/policy        | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |

|           time includes          |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *time/rate.h*                    | Rational frequency container + utils        | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *time/rate_factory.h*            | Standard rate creator (video SMPTE/audio)   | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *time/stopwatch.h*               | Stop-watch to measure elapsed time/duration | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *time/system_clocks.h*           | High-res. OS-specific clocks/timers         | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *time/timer.h*                   | High-precision dual timer (clock sync)      | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |

|          video includes          |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *video/api/message_box.h*        | Alert message-box: standard/custom buttons  | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![TEST](_img/badges/feat_not_tested.png) | ![TEST](_img/badges/feat_not_tested.png) | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) |
| *video/api/window.h*             | System window manager + builder             | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) |
| *video/api/window_handle.h*      | Window/resource/menu handle types           | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *video/api/window_input.h*       | Window input enums/utils (key/mouse/hw/...) | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) |
| *video/api/window_keycodes.h*    | OS-specific virtual-keycode bindings        | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) |
| *video/api/window_resource.h*    | Resource builders: icon/cursor/brush/...    | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) | ![NOT](_img/badges/feat_not_impl.png) |
| *video/api/opengl.h*             | OpenGL API includes with bindings           | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
| *video/api/vulkan.h*             | Vulkan API includes                         | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![NOT](_img/badges/feat_not_impl.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) | ![OK](_img/badges/feat_done.png) |
