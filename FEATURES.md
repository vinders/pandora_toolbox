# Feature list

<sub>**win** = *Windows* / 
**mac** = *MacOS* / 
**ios** = *iOS* / 
**and** = *Android* / 
**x11** = *Linux/Unix/BSD (X11)* / 
**wln** = *Linux (Wayland)*</sub>

<sub>![OK](_img/badges/feat_done.svg) : done -- 
![BUG](_img/badges/feat_issues.svg) : minor issue(s) -- 
![TEST](_img/badges/feat_not_tested.svg) : not fully tested -- 
![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>

|         hardware includes        |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *hardware/cpu_instruction_set.h* | CPU instuction set ID/family ('cpu_specs.h')|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *hardware/cpu_specs.h*           | CPU info/specs/features reader              |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *hardware/cpu_vendor.h*          | CPU vendor (enum/labels) (for 'cpu_specs.h')|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *hardware/cpuid_property_loca...*| CPUID reg. property ID (for 'cpu_specs.h')  |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *hardware/cpuid_registers.h*     | List of CPUID registers (for 'cpu_specs.h') |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *hardware/display_monitor.h*     | Monitor info/size/DPI + display modes       |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![TEST](_img/badges/feat_not_tested.svg)|![TEST](_img/badges/feat_not_tested.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *hardware/process_affinity.h*    | Process/thread affinity with CPU core(s)    |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|

|            io includes           |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *io/csv_log_formatter.h*         | Log formatter: CSV table format             |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *io/file_handle.h*               | Managed C file handle (RAII)                |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *io/file_system_io.h*            | File-system operations + metadata reader    |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *io/file_system_locations.h*     | Standard OS-specific location finder        |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![TEST](_img/badges/feat_not_tested.svg)|![TEST](_img/badges/feat_not_tested.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|

|          logic includes          |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *logic/math.h*                   | Math algorithms: GCD/pow2/near-equal/...    |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *logic/search.h*                 | Search algorithms: binary/jump/exp/interp.  |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *logic/sort.h*                   | Sort algorithms: linear/heap/quick-sort     |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *logic/sort_order.h*             | Data order enum (for 'search.h'/'sort.h')   |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *logic/strings.h*                | String utils: trim/pad/find/assign/...      |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|

|          memory includes         |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| list TODO                        |                                             |     |     |     |     |     |     |

|         pattern includes         |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| list TODO                        |                                             |     |     |     |     |     |     |

|          system includes         |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *system/cpu_arch.h*              | CPU arch. detection functions/constants     |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *system/date_time.h*             | Date/time string formatting                 |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *system/export.h*                | Dynamic lib import/export attributes        |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *system/force_inline.h*          | Portable '__forceinline' attribute          |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *system/logger.h*                | Simple log writer/formatter/options         |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *system/operating_system.h*      | Op. system detection function/constants     |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *system/preprocessor_tools.h*    | Macros: duplicate, enum serializer/flags... |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *system/trace.h*                 | Debug trace utilities                       |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *system/api/intrinsics.h*        | CPU intrinsics API (MMX/SSE/AVX/NEON/...)   |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![BUG](_img/badges/feat_issues.svg)|![BUG](_img/badges/feat_issues.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *system/api/android_app.h*       | Android app state container                 |![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|
| *system/api/windows_app.h*       | Windows app instance container              |![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|
| *system/api/windows_api.h*       | Windows API, with version + options         |![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|
| *system/api/windows_base_types.h*| Minimum Windows API: types defs only        |![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|
| *system/api/windows_common_co...*| Windows common controls: API + manifest     |![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|
| *system/api/windows_version.h*   | Windows minimum system version defs         |![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|![NOT](_img/badges/feat_not_impl.svg) : not implemented yet</sub>|

|          thread includes         |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *thread/ordered_lock.h*          | Concurrency sync primitive with FIFO order  |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *thread/semaphore.h*             | Sync primitive with counter (wait/notify)   |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *thread/spin_lock.h*             | Active/polling concurrency sync primitive   |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *thread/recursive_spin_lock.h*   | Spin-lock with recursive thread ownership   |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *thread/thread_pool.h*           | Fixed-size pool of threads (async tasks)    |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *thread/thread_priority.h*       | Set thread scheduler priority/policy        |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|

|           time includes          |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *time/rate.h*                    | Rational frequency container + utils        |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *time/rate_factory.h*            | Standard rate creator (video SMPTE/audio)   |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *time/stopwatch.h*               | Stop-watch to measure elapsed time/duration |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *time/system_clocks.h*           | High-res. OS-specific clocks/timers         |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *time/timer.h*                   | High-precision dual timer (clock sync)      |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|

|          video includes          |                 description                 | win | mac | ios | and | x11 | wln |
|----------------------------------|---------------------------------------------|-----|-----|-----|-----|-----|-----|
| *video/api/message_box.h*        | Alert message-box: standard/custom buttons  |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![TEST](_img/badges/feat_not_tested.svg)|![TEST](_img/badges/feat_not_tested.svg)|![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg)|
| *video/api/window.h*             | System window manager + builder             |![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|
| *video/api/window_handle.h*      | Window/resource/menu handle types           |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *video/api/window_input.h*       | Window input enums/utils (key/mouse/hw/...) |![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|
| *video/api/window_keycodes.h*    | OS-specific virtual-keycode bindings        |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|
| *video/api/window_resource.h*    | Resource builders: icon/cursor/brush/...    |![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|![NOT](_img/badges/feat_not_impl.svg)|
| *video/api/opengl.h*             | OpenGL API includes with bindings           |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
| *video/api/vulkan.h*             | Vulkan API includes                         |![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![NOT](_img/badges/feat_not_impl.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|![OK](_img/badges/feat_done.svg)|
