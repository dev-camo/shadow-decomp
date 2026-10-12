Shadow the Hedgehog  
[![Build Status]][actions] [![Code Progress]][progress] [![Data Progress]][progress]
=============
[<img src="https://decomp.dev/dev-camo/shadow-decomp.svg?w=512&h=256" width="512" height="256">][Progress]
=============

[Build Status]: https://github.com/dev-camo/shadow-decomp/actions/workflows/build.yml/badge.svg
[actions]: https://github.com/dev-camo/shadow-decomp/actions/workflows/build.yml
<!--
decomp.dev progress badges
See https://decomp.dev/api for an API overview.
-->
[Code Progress]: https://decomp.dev/dev-camo/shadow-decomp.svg?mode=shield&measure=code&label=Code
[Data Progress]: https://decomp.dev/dev-camo/shadow-decomp.svg?mode=shield&measure=data&label=Data
[progress]: https://decomp.dev/dev-camo/shadow-decomp

A work-in-progress decompilation of Shadow the Hedgehog.

This repository does **not** contain any game assets or assembly whatsoever. An existing copy of the game is required.

Supported versions:

- `GUPE8P`: Rev 0 (USA)
- `GUPJ8P`: Rev 0 (Japan)
- `GUPP8P`: Rev 0 (Europe / Australia)

AI Use Disclosure
========================

This project uses large language model (LLM) assistance tools to support
decompilation analysis, code development, build troubleshooting, and documentation.

Dependencies
============

Install [uv](https://docs.astral.sh/uv/getting-started/installation/), then run
these commands from the repository root:

```sh
uv sync --locked
```

The project requires Python 3.14 or newer. uv creates `.venv` and installs the
Ninja version recorded in `uv.lock`, obtaining a compatible Python if needed.
Use `uv run --locked` for project commands; activating the environment is optional.
Keep `pyproject.toml` and `uv.lock` together when setting up another checkout.
If `--locked` reports an inconsistent lockfile, resolve the manifest/lockfile mismatch
instead of bypassing the check or upgrading dependencies as part of a normal build.

The build downloads its pinned decomp-toolkit, objdiff CLI, compiler package,
and other native tools into `build/`. These are separate from uv's Python
environment. On Linux/macOS, the build uses wibo to run the Windows compiler tools;
on Windows, use native tooling. Cross-host filesystem watching can affect objdiff
automatic rebuilds, so rebuild explicitly if changes are not detected.

Building
========

- Clone the repository:

  ```sh
  git clone https://github.com/dev-camo/shadow-decomp.git
  ```

- Copy your game's disc image to `orig/{GUPE8P,GUPJ8P,GUPP8P}`.
  - Supported formats: ISO (GCM), RVZ, WIA, WBFS, CISO, NFS, GCZ, TGC
  - After the initial build, the disc image can be deleted to save space.

- Configure:

  ```sh
  uv run --locked python configure.py
  ```

  To use another region, run `uv run --locked python configure.py --version GUPJ8P`
  or `uv run --locked python configure.py --version GUPP8P` before building.
  Running `uv run --locked python configure.py` selects the default `GUPE8P` (USA).

- Build:

  ```sh
  uv run --locked ninja
  ```

Diffing
=======

Once the initial build succeeds, an `objdiff.json` should exist in the project root.

If launching objdiff outside the uv environment on Linux/macOS, configure with
`uv run --locked python configure.py --ninja .venv/bin/ninja` so automatic rebuilds
use the project's Ninja. Include `--version GUPJ8P` or `--version GUPP8P` when
working on those regions. On Windows, use `.venv/Scripts/ninja.exe` for this option.

Download the latest release from [encounter/objdiff](https://github.com/encounter/objdiff). Under project settings, set `Project directory`. The configuration should be loaded automatically.

Select an object from the left sidebar to begin diffing. Changes to the project will rebuild automatically: changes to source files, headers, `configure.py`, `splits.txt` or `symbols.txt`.

![](.github/assets/objdiff.png)

Attribution
===========

This decompilation builds on work published by other decompilation and reverse-engineering
projects. Their sources were used as references for the code, names and layouts listed
below; every reconstructed file is still verified against this game's own executable.

| Project | Used for |
| --- | --- |
| [Resident Evil 4](https://github.com/adonis-singh/re4) (GameCube) | CRI middleware sources used as the basis for most of `src/CRI/`: ADX (ADXF, ADXB format decoders, ADXSJD, ADXT init, header decoding, ADXPD, ADXRNA, ADXAMP, errors), LSC, SJ, SVM, CVFS and the MFCI/GCCI device interfaces, Sofdec MPV/MPS/SFD and UTY helper units and the MWSTM stream glue; the CRI interface and Sofdec layout headers; the Dolphin SDK headers (`include/dolphin`, `include/libc`) and SDK sources in `src/dolphin/`; names for CRI and Dolphin SDK functions |
| [Dolphin SDK 2004 decompilation](https://github.com/doldecomp/dolsdk2004) | Origin of the Dolphin SDK sources and headers (taken via the Resident Evil 4 project) behind `src/dolphin/`, `include/dolphin` and `include/libc`, including the `SDK_REVISION` handling of the Nov 2004 GX/OS builds |
| [The Legend of Zelda: Twilight Princess](https://github.com/zeldaret/tp) | CodeWarrior runtime and MSL C sources and headers (`src/PowerPC_EABI_Support/`): fdlibm math, mem, mem_funcs, direct_io, signal, string, strtoul, strtold, wchar_io, arith, buffer_io, char_io, ctype, file_io, FILE_POS, critical_regions, errno, uart_console_io_gcn and __va_arg, and the GC/2.7 runtime build flags; the MSL alloc.c pool structures and `__pool_free`, and the Gecko_ExceptionPPC fragment registration, used by `src/Runtime.PPCEABI.H/` |
| [Mortal Kombat: Deception](https://github.com/ShulkMaster/mk-deception) (GameCube) | CRI svm.c, lsc.c and adx_fini.c references: function order and names such as `SVM_ExecSvrMain`/`Fs`/`Vsync`/`MwIdle`, `SVM_SetCbBdr`, `LSC_SetLpFlg` and `LSC_Start` |
| [Street Fighter III: 3rd Strike](https://github.com/crowded-street/3s-decomp) | Newer CRI lsc.c reference: source order, error-code numbering and names such as `LSC_ResetEntry` and the LSC stream queries; CRI adx_fs.c function bodies and ADXF/ADXSTM function names; CRI adx_lsc.c function names, order and error codes; CRI adx_tlk2.c (`ADXT_StartFname`, `ADXT_StartAfs`) and ADXT names such as `ADXT_StopWithoutLsc` and `ADXT_GetTimeReal` |
| [Teenage Mutant Ninja Turtles 2: Battle Nexus](https://github.com/xntrz/tmnt2) (PC reimplementation) | CRI adx_fs.c structure: public `ADXF_*` entries wrapping `adxf_*` workers, and names such as `adxf_OpenRange` |
| [The World Ends With You](https://github.com/Yotona/twewy) | CRI adx_crs.c function order and the names `ADXCRS_Enter` and `ADXCRS_Leave`; CRI adx_stmc.c structure (public `ADXSTM_*` entries over `adxstm_*` workers) |
| [Dragon Ball Z: Budokai Tenkaichi 3](https://github.com/z3xox/Tenkaichi3Decomp) | CRI adx_fs.c worker and wrapper naming notes |
| [Xenoblade Chronicles](https://github.com/xbret/xenoblade) | CRI sj_crs.c names `SJCRS_Init` and `SJCRS_Finish` |
| [Sonic Heroes](https://github.com/Jovinull/sonicheroes) | CRI cri_cvfs.c and adx_fini.c comparison references |
| [The Legend of Zelda: The Wind Waker](https://github.com/zeldaret/tww) and [Twilight Princess](https://github.com/zeldaret/tp) | Metrowerks runtime and Dolphin OS references for GCN_mem_alloc.c, abort, signal and heap function names |
| [Pikmin 2](https://github.com/projectPiki/pikmin2) | Metrowerks MSL allocator references (alloc.c function names and sizes); the MSL `__strtold` body used for strtold.c, the ansi_fp.c, printf.c and mbstring.c functions, and the float.c and locale.c data; original sizes of the dead-stripped MSL string.c functions |
| [Metroid Prime](https://github.com/PrimeDecomp/prime) and [Mario Kart: Double Dash!!](https://github.com/doldecomp/mkdd) | Metrowerks MSL allocator comparison references |
| [decomp-toolkit](https://github.com/encounter/decomp-toolkit), [dtk-template](https://github.com/encounter/dtk-template) and [objdiff](https://github.com/encounter/objdiff) | Project tooling, build template and object diffing |
