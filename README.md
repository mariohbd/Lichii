# Fritchii custom: Ocarina of Time build

An unofficial build of [Fritchii](https://github.com/Frizti/Fritchii) **v5.3** (Frizti's fork of
[Lichii](https://github.com/zortos293/Lichii)), with extra performance and correctness work for
**The Legend of Zelda: Ocarina of Time** on Switch 2 (`0400486024DEC000`).

It was put together on a Ryzen 5 5600X, RTX 3060 Ti 8 GB and a 3440x1440 monitor. It is
**experimental**: it compiles and starts, but it has had little play time. It is not affiliated
with Nintendo, Lichii or Fritchii. No game files, keys or firmware are included: use only a copy of
the game and system files you obtained legally.

**[Download the latest release](https://github.com/mariohbd/Lichii/releases/latest)**

The Fritchii README is in [README-FRITCHII.md](README-FRITCHII.md).

---

## Base

Everything in Fritchii v5.3 is kept, including its shader precompilation ("Preparing shaders" on the
first boot, about 120,000 pipelines). This build changes the kept-SPIR-V stamp (`NX2KEPT5`), so a
cache prepared by the official Fritchii is prepared again once in the background. Give this build
its own `user/cache` folder if you also run the official one, or each will keep redoing the other's
preparation.

## Switches

Every change below that has a switch is read from `nx2_switches.txt` next to `lichii.exe` (one
`NAME=VALUE` per line, restart after editing). Most are **on by default** and `=0` turns them off.
Four are **off by default** because they are riskier: `EDEN_NX2_OOT_BANK_SKIP`,
`EDEN_NX2_VB_BIND_SKIP`, `EDEN_NX2_PRESENT_QUEUE` and `EDEN_NX2_WAITABLE_TIMER` (`=1` turns them on).
`EDEN_NX2_RSMAN_VRAM=0` turns off the whole VRAM series at once.

If you see graphics problems, try these first: `EDEN_NX2_RSMAN_VRAM=0`,
`EDEN_NX2_SKIP_EMPTY_DRAWS=0`, `EDEN_NX2_COLOR_MASK_TRIM=0`, `EDEN_NX2_JIT_SUPERBLOCKS=0`.

Suggested on NVIDIA (in `recommended-config/nx2_switches-nvidia.txt`): `EDEN_NX2_EXP16=8192` and
`EDEN_NX2_ASYNC_DISCARD=0` (waits for a pipeline instead of skipping its draws, which removes the
few seconds of artifacts after an area change on a warm cache).

## What this build adds

### Texture cache

| Change | Source | Switch |
| --- | --- | --- |
| Keep the TIC and TSC lookup maps in flat hash maps | 883b98d1e8 | always on |
| Unswizzle and swizzle subrects through a per-GOB 16-byte unit table | 97f90e38d6 + d25767e86f | `EDEN_NX2_UNSWIZZLE_TABLE=0` turns it off |
| Skip alias sync when no alias changed | 4b04fd7409 | `EDEN_NX2_ALIAS_SYNC_SKIP=0` turns it off |
| Don't upload an image again with what it already holds | 9ac7fb8a4e | `EDEN_NX2_TC_SKIP_SAME_UPLOAD=0` turns it off |
| Place 8-64 MiB images in a pool of 256 MiB blocks | [Lichii #922](https://github.com/zortos293/Lichii/pull/922) / 9935eddc96 + 74c3712301 | `EDEN_NX2_MEDIUM_IMAGE_POOL=0` turns it off |
| Cap the EDEN_NX2_TC_*_MIB overrides at the device budget | 834b1ca136 | always on |
| Keep shadow maps at native resolution | [PantACRO4life](https://github.com/PantACRO4life/Lichii) 9601db93ab | `EDEN_NX2_SHADOW_NATIVE=0` turns it off |
| Bind the null image for depth-compare samples of uncomparable images | [PantACRO4life](https://github.com/PantACRO4life/Lichii) b10d65d730 | `EDEN_NX2_NULL_DEPTH_CMP=0` turns it off |

### Buffer cache / memory

| Change | Source | Switch |
| --- | --- | --- |
| Skip the native bank check for Ocarina of Time, opt-in | [Lichii #909](https://github.com/zortos293/Lichii/pull/909) | `EDEN_NX2_OOT_BANK_SKIP=1` (off by default) |
| Skip unchanged words while binding clean buffers | c45c53269b | `EDEN_NX2_BC_SKIP_CLEAN_WORDS=0` turns it off |
| Reuse the last memory layout interval during draw setup | 5028f38ac2 | `EDEN_NX2_LAYOUT_REUSE=0` turns it off |
| Let HostMemory::Protect share the placeholder lock | 64b8cb1d88 + 9fdc95cb71 | `EDEN_NX2_PROTECT_SHARED_LOCK=0` turns it off |
| Arm read traps only on pages not armed yet | d181de503c, [Lichii #264](https://github.com/zortos293/Lichii/pull/264) | `EDEN_NX2_READ_TRAP_ONCE=0` turns it off |
| Skip vertex buffer binds still bound in the command buffer | [Lichii #955](https://github.com/zortos293/Lichii/pull/955) | `EDEN_NX2_VB_BIND_SKIP=1` (off by default) |
| Keep the resolved index buffer | [Lichii #954](https://github.com/zortos293/Lichii/pull/954) | `EDEN_NX2_IB_KEEP=0` turns it off |
| Give each DeviceMemoryManager its own translation generations | 78a0c46373, [Lichii #984](https://github.com/zortos293/Lichii/pull/984) | always on |
| Reserve the last page table level from the range's last byte | [Lichii #957](https://github.com/zortos293/Lichii/pull/957) | always on |
| Finish thread worker task cleanup before publishing completion | [Lichii #939](https://github.com/zortos293/Lichii/pull/939) | always on |
| Flush adjacent device pages of a GPU range in one FlushRegion | [1Shad0w](https://github.com/1Shad0w/Lichii) e007a05460 | `EDEN_NX2_FLUSH_COALESCE=0` turns it off |

### Shaders / pipelines

| Change | Source | Switch |
| --- | --- | --- |
| Size the last descriptor payload once per pipeline | bd00ef8105 | `EDEN_NX2_DESC_SIZE_ONCE=0` turns it off |
| Skip empty direct draws before pipeline, buffer and descriptor work | 4b8759aa2d | `EDEN_NX2_SKIP_EMPTY_DRAWS=0` turns it off |
| Read texture descriptors without flushing during shader translation | 591ceee8b1, [Lichii #970](https://github.com/zortos293/Lichii/pull/970) | always on |
| Shadow high registers of IMAD with a uniform multiplier | c149054446, #279 | always on |
| Gather each tap with its own offset when per-tap offsets stay dynamic | c11ea2f0d4, #433 | `EDEN_NX2_GATHER_PER_TAP=0` turns it off |
| Route native global memory helpers with one switch | [Eglantine](https://github.com/Eglantine-Community/Eglantine) 9a362357a2 | `EDEN_NX2_SPIRV_GMEM_SWITCH=0` turns it off |
| Don't write render target components the fragment shader does not output | 79c151694a / [A-Man-With-A-Plan-dev](https://github.com/A-Man-With-A-Plan-dev/Eglantine) 8e1b50ca4c, [Lichii #353](https://github.com/zortos293/Lichii/pull/353) | `EDEN_NX2_COLOR_MASK_TRIM=0` turns it off |
| Compile pointer-slot compute rebuilds on a worker instead of waiting in the dispatch | e61ecab335 + pr969, [Lichii #995](https://github.com/zortos293/Lichii/pull/995) / #969 | `EDEN_NX2_COMPUTE_REBUILD_WORKER=0` turns it off |
| Keep unused graphics pipelines until the disk cache size is exceeded | pr994, [Lichii #994](https://github.com/zortos293/Lichii/pull/994) | `EDEN_NX2_PIPELINE_KEEP_BUDGET=0` turns it off |
| Spread run-time shader builds over frames | 4553f1a100 / [A-Man-With-A-Plan-dev](https://github.com/A-Man-With-A-Plan-dev/Eglantine) c9b8bb5e52 + e330d49488 | `EDEN_NX2_SHADER_BUILD_BUDGET=<ms>` (default 24, 0 off) |

### CPU / timing / Vulkan

| Change | Source | Switch |
| --- | --- | --- |
| Operation lists as vectors | b51a0cddd6 + e08d94c10b | always on |
| Present and audio renderer threads in MMCSS "Games" | a6777b6f1a | `EDEN_NX2_MMCSS=0` turns it off |
| Submit while the front waits or the GPU is idle | 3216f3ea99 | `EDEN_NX2_SUBMIT_WHILE_WAITED=0` turns it off |
| Don't scale the guest counter for CPU Boost/Overclock in Switch 2 titles | eb7eecbe57 | `EDEN_NX2_NO_BOOST_SCALE=0` turns it off |
| Memory priorities and pageable device memory | ee7617f691 | `EDEN_NX2_MEM_PRIORITY=0` turns it off |
| Drop due events whose type is gone instead of spinning on them | 782d8f48e4 | always on |
| Wake the timer thread only for events it would otherwise be late for | ce5966d758 | `EDEN_NX2_TIMING_SLEEP=0` turns it off |
| Keep fence-flagged payload releases synchronized at low accuracy under EXP33 | pr874 1a0a91dc4f | `EDEN_NX2_QUERY_LOW_ACC_SYNC` (only with `EDEN_NX2_EXP33=1`) |

### Ocarina / DLSS / video

| Change | Source | Switch |
| --- | --- | --- |
| Enlarge the Turing macro RAM 0x3000->0x8000 words so entries past 0x1000 execute | 7969a392c2 / #439 | always on |
| Accept DLSS presets E and F in EDEN_NX2_DLSS_PRESET and log DLSS memory | 72067ce4e1 | `EDEN_NX2_DLSS_PRESET=<preset>` |
| Keep NVIDIA DLSS on when Streamline warns the VRAM budget is used up | pr941 0a66c512bb | always on |
| Keep Ocarina's glows and particles out of DLSS history, bias mask as R16F | 82494c4820 + 74a2d20c90 + 4352c06bbf / #635 | `EDEN_NX2_DLSS_BIAS_MASK=0` turns it off |
| Return present release fences instead of blocking until the vblank | 7c5c9d1bb7 / upstream #695 by @Alleexxi | `EDEN_NX2_PRESENT_QUEUE=1` (off by default) |
| Drop late movie frames so the picture keeps up with the sound | [Eglantine](https://github.com/Eglantine-Community/Eglantine) 1f52ac7e2e | `EDEN_NX2_VDEC_PACE=0` turns it off |
| Fix the Castle Town guard house freeze and its device-memory blowup | pr976 51e0fd3f42 + db0c346e03 | `EDEN_NX2_OOT_GUARDHOUSE_FIX=0` turns it off |
| Decompress Ocarina of Time's zstd on the host, streamed blocks included | 033b26f1a4 / [Lichii #869](https://github.com/zortos293/Lichii/pull/869) | `EDEN_NX2_HOST_ZSTD=0` turns it off |
| Decode NUP zstd members only as far as reads need | 2601083ae8 | always on |
| Finish NUP zstd members only at frame end and bound spill decoders | 7d292cbe9a | always on |
| Move Ocarina's HUD to the 21:9 screen edges with the 3440x1440 ultrawide mod | 0e43dc96ae + f136edfd5f + 9429009c27 | `EDEN_NX2_OOT_HUD_EDGES=0` turns it off |
| NVIDIA DLSS mode setting (Quality / Balanced / Performance / Ultra Performance) | 4329fa17ed + f2f226f7a3 + 3ce682b588, [Lichii #903](https://github.com/zortos293/Lichii/pull/903) | Setting in Graphics |

### CPU JIT

| Change | Source | Switch |
| --- | --- | --- |
| A64 superblocks, conditional branches become side exits | ecaaed9711 | `EDEN_NX2_JIT_SUPERBLOCKS=0` turns it off |
| Fused zero/bit tests in side exits | 82d103f394 | `EDEN_NX2_JIT_SUPERBLOCKS=0` turns it off |

### Frame pacing

| Change | Source | Switch |
| --- | --- | --- |
| Precise 300 us tail after the high resolution timer sleep | [Lichii #1002](https://github.com/zortos293/Lichii/pull/1002) | `EDEN_NX2_WAITABLE_TIMER=1` (off by default) |
| Frame pacing target override | [Lichii #1002](https://github.com/zortos293/Lichii/pull/1002) | `EDEN_NX2_TARGET_FPS=<fps>` (unset: off) |

### VRAM (RsMan-Dev series)

| Change | Source | Switch |
| --- | --- | --- |
| Upload new render targets only when the clear does not overwrite them | RsMan-Dev c724c2e9c9 + 4c5fccc93c, [Lichii #267](https://github.com/zortos293/Lichii/pull/267) | `EDEN_NX2_RSMAN_VRAM_RT_DEFER=0` turns it off |
| Use one image for a depth target and its square texture | RsMan-Dev fb86321f6f, [Lichii #267](https://github.com/zortos293/Lichii/pull/267) | `EDEN_NX2_RSMAN_VRAM_SQUARE_DEPTH=0` turns it off |
| Skip the upload of a render target placed over GPU contents | RsMan-Dev 0ad215969a, [Lichii #267](https://github.com/zortos293/Lichii/pull/267) | `EDEN_NX2_RSMAN_VRAM_OVER_GPU=0` turns it off |
| Make the unscaled image of a new render target only when needed | RsMan-Dev 941bb41427, [Lichii #267](https://github.com/zortos293/Lichii/pull/267) | `EDEN_NX2_RSMAN_VRAM_LAZY_ORIGINAL=0` turns it off |
| Free the unscaled image once the scaled one holds the contents | RsMan-Dev 27d7b06551, [Lichii #267](https://github.com/zortos293/Lichii/pull/267) | `EDEN_NX2_RSMAN_VRAM_FREE_ORIGINAL=0` turns it off |

## Ultrawide 21:9

Ultrawide comes from Ivan0xFF's mod, **[craftycorvid/oot-mod](https://github.com/craftycorvid/oot-mod)**
(MIT). With its 3440x1440 version enabled, this build also moves the hearts, magic and item buttons
to the screen edges ([Lichii #831](https://github.com/zortos293/Lichii/pull/831)). Import the mod's
zip under **Properties → Add-Ons**, then set **Graphics → Aspect Ratio → Stretch to Window**.

## Build

clang-cl 22, ThinLTO, Lichii's PGO profile and `-march=znver3`: it needs a **Ryzen 5000 or newer**
(Zen 3+) CPU. Script: `build-clang-v51.bat` style, `cmake -G Ninja` with `LICHII_BUILD_TAG=fritchii-v5.3-custom-pgo`.

## Credits

GPL-3.0. All credit goes to Frizti (Fritchii), zortos293 and the Lichii contributors, the Eden and
yuzu developers, the Eglantine community, A-Man-With-A-Plan-dev, PantACRO4life, 1Shad0w, RsMan-Dev
and the authors of each PR listed above. The ports were made with Claude Code.
