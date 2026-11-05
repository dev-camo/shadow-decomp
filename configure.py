#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import re
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    build_include_path,
    calculate_progress,
    generate_build,
    is_windows,
    normalize_configure_args,
    prepare_build_sha_manifest,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "GUPE8P",  # 0
    "GUPJ8P",  # 1
    "GUPP8P",  # 2
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--warn",
    dest="warn",
    type=str,
    choices=["all", "off", "error"],
    help="how to handle warnings",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
config.configure_args = normalize_configure_args(sys.argv[1:], parser, args.mode)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.ninja_path = args.ninja
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-2"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.4"
config.objdiff_tag = "v3.8.3"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "1.0.3"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.build_check_sha_path = prepare_build_sha_manifest(config)
build_include = build_include_path(config)
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I {build_include}",
    f"--defsym BUILD_VERSION={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = [config.check_sha_path]

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
    "-i include",
    f"-i {build_include}",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Warning flags
if args.warn == "all":
    cflags_base.append("-W all")
elif args.warn == "off":
    cflags_base.append("-W off")
elif args.warn == "error":
    cflags_base.append("-W error")

# Game and CRI middleware flags: the base flags plus the C library headers in include/libc.
cflags_game = [
    *cflags_base,
    "-i include/libc",
]

# The MSL C, math and C++ library headers. The C++ units built with cflags_game_cpp or cflags_csd,
# Game/fn_8041116C.cpp and Game/fn_80420724.cpp (for <cmath>) and CRI/dct_ac.c (for MSL's <math.h>)
# search them in place of include/libc; other C game code and the other fn_8040 C++ helpers
# (cflags_game) keep the C declarations in include/libc.
msl_includes = [
    "-i src/PowerPC_EABI_Support",
    "-i src/PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Include",
    "-i src/PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Include",
    "-i src/PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Include",
    "-i src/PowerPC_EABI_Support/MSL/MSL_C++/MSL_Common/Include",
    "-i src/PowerPC_EABI_Support/Runtime/Inc",
]

# Game C++: the original carries MWCC RTTI type infos for its classes and is optimized for size
# (saving register pairs with stmw/lmw). Only functions marked inline are inlined, and only those
# whose bodies call no other inline function (level=1): a constructor that just stores three floats
# is inlined into a static initializer, while weak_ptr's converting constructor, whose body calls
# weak_count's inline constructor, is never inlined and leaves a local copy in each unit that uses
# it. Inlining is deferred: functions are emitted in reverse definition order, and the RTTI names
# are pooled before the function literals.
cflags_game_cpp = [
    *cflags_base,
    *msl_includes,
    "-O4,s",
    "-use_lmw_stmw on",
    "-str reuse,readonly",
    "-RTTI on",
    "-inline noauto,level=1,deferred",
]

# Effect commands have C++ exception tables, RTTI in .data and float literals in .rodata.
# Matching workaround: vector command constructors address each literal independently;
# data pooling instead introduces a shared constant-base register. Revisit when the original
# effect unit's compiler settings are recovered.
cflags_effect = [
    *cflags_game_cpp,
    "-Cpp_exceptions on",
    "-sdata 0",
    "-sdata2 0",
    "-pool off",
]

# RW3 GlobalParam addresses RTTI and its vtable through a shared data base.
# Keep default pooling with the effect exception and section settings.
cflags_effect_rw3 = [
    *cflags_game_cpp,
    "-Cpp_exceptions on",
    "-sdata 0",
    "-sdata2 0",
]

# Sega's GameCube AX sound library (gcax*.c): optimized for size with stmw/lmw, read-only strings,
# and deferred inlining of only the functions marked inline.
cflags_game_gcax = [
    *cflags_game,
    "-O4,s",
    "-use_lmw_stmw on",
    "-str reuse,readonly",
    "-inline noauto,deferred",
]

# Chao::CSD UI library (game C++ without RTTI): small data off (globals in .bss, constants in
# .rodata), read-only strings (literals in .rodata), size-optimized with stmw/lmw, deferred inlining
# (functions in reverse definition order) one level deep (recursive inline functions are expanded
# once).
cflags_csd = [
    *cflags_base,
    *msl_includes,
    "-O4,s",
    "-use_lmw_stmw on",
    "-str reuse,readonly",
    "-inline noauto,deferred,level=1",
    "-sdata 0",
    "-sdata2 0",
]

# Metrowerks library flags (the runtime units include the MSL and runtime headers)
cflags_runtime = [
    *cflags_base,
    "-i src/PowerPC_EABI_Support",
    "-i src/PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Include",
    "-i src/PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Include",
    "-i src/PowerPC_EABI_Support/Runtime/Inc",
    "-i include/libc",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-inline auto",
]

# REL flags
cflags_rel = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
]

config.linker_version = "GC/2.7"


# Dolphin SDK flags: Nintendo's prebuilt SDK libraries (GC/1.2.5n, -O4,p, unsigned char, default
# small data). Headers in include/dolphin and include/libc; private SDK headers in src/dolphin.
cflags_sdk = [
    "-nodefaults",
    "-proc gekko",
    "-fp hard",
    "-Cpp_exceptions off",
    "-enum int",
    "-char unsigned",
    "-warn pragmas",
    "-requireprotos",
    '-pragma "cats off"',
    "-O4,p",
    "-inline auto",
    "-I-",
    "-i include",
    "-i include/libc",
    "-i src/dolphin",
    "-D__GEKKO__",
    "-DSDK_REVISION=0",
]
# The GX and OS libraries in this game are the Nov 10 2004 builds (SDK_REVISION 2 in the
# dolsdk2004 decompilation), so the os and gx libraries below are built with cflags_sdk_rev2.
# OSArena.c's OSAllocFromArenaHi is in the os object, but the game never calls it and the
# linker drops it. The other SDK libraries with version strings are the Apr 2004 builds
# (Apr 5, VI Apr 7), which revision 0 corresponds to: its OS build string is Apr 5 2004
# 04:13:58, from the same build run.
# Revision 1 is the May 21 2004 patch.
cflags_sdk_rev2 = [flag for flag in cflags_sdk if flag != "-DSDK_REVISION=0"] + ["-DSDK_REVISION=2"]


# CodeWarrior runtime and MSL C library (PowerPC_EABI_Support) as built for this game with GC/2.7:
# signed char, 4-byte function alignment, MSL inlines and deferred inlining.
cflags_eabi = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-common off",
    "-char signed",
    "-func_align 4",
    "-DMSL_USE_INLINES=1",
    "-inline deferred,auto",
    "-i src/PowerPC_EABI_Support",
    "-i src/PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Include",
    "-i src/PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Include",
    "-i src/PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Include",
    "-i src/PowerPC_EABI_Support/MSL/MSL_C++/MSL_Common/Include",
    "-i src/PowerPC_EABI_Support/Runtime/Inc",
    "-i src/PowerPC_EABI_Support/MetroTRK",
    "-D__GEKKO__",
]


# Helper function for Dolphin libraries
def DolphinLib(
    lib_name: str, objects: List[Object], cflags: List[str] = cflags_sdk
) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags,
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "objects": objects,
    }


Matching = True  # Object matches and should be linked
NonMatching = False  # Object does not match and should not be linked
Equivalent = (
    config.non_matching
)  # Object should be linked when configured with --non-matching


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    {
        # MetroTRK debugger stub (TRK_MINNOW_DOLPHIN), built without small data or string pooling
        "lib": "MetroTRK",
        "mw_version": config.linker_version,
        "cflags": [*cflags_runtime, "-str reuse,nopool,readonly", "-inline deferred", "-sdata 0", "-sdata2 0"],
        "objects": [
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/mainloop.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/nubevent.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/nubinit.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/msg.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/msgbuf.c"),
            # Matching workaround: the library is built with -sdata 0, but the original serpoll
            # keeps its 4-byte gTRKInputPendingPtr pointer in .sbss; at -sdata 0 it lands in
            # .bss. Revisit when the original per-unit small-data threshold is known.
            Object(
                Matching,
                "PowerPC_EABI_Support/MetroTRK/serpoll.c",
                extra_cflags=["-sdata 8"],
            ),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/usr_put.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/dispatch.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/msghndlr.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/support.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/mutex_TRK.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/notify.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/flush_cache.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/mem_TRK.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/targimpl.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/mpc_7xx_603e.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/dolphin_trk.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/main_TRK.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/dolphin_trk_glue.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/targcont.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/target_options.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/mslsupp.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/UDP_Stubs.c"),
            # Matching workaround: as serpoll.c above (the original keeps the 4-byte
            # gIsInitialized in .sbss; at -sdata 0 it is addressed through .bss with lis/addi
            # and the code differs).
            Object(
                Matching,
                "PowerPC_EABI_Support/MetroTRK/ddh/main.c",
                extra_cflags=["-sdata 8"],
            ),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/CircleBuffer.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/MWTrace.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/MWCriticalSection_gc.cpp"),
        ],
    },
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": config.linker_version,
        "cflags": cflags_runtime,
        "objects": [
            Object(
                NonMatching,
                "Runtime.PPCEABI.H/register_fragment.c",
                # The target unit owns an EABI extab/extabindex record.
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "Runtime.PPCEABI.H/bad_exception.c"),
            Object(
                Matching,
                "Runtime.PPCEABI.H/bad_exception_dtor.c",
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "Runtime.PPCEABI.H/GCN_Mem_Alloc.c"),
            Object(Matching, "Runtime.PPCEABI.H/calloc.c"),
            Object(NonMatching, "Runtime.PPCEABI.H/allocator_core.c"),
            Object(Matching, "Runtime.PPCEABI.H/allocator_release.c"),
            Object(Matching, "Runtime.PPCEABI.H/abort_exit.c"),
            Object(Matching, "Runtime.PPCEABI.H/global_destructor_chain.c"),
            # All four operators match; pooled what() strings and weak destructor unwind records differ.
            Object(
                NonMatching,
                "Runtime.PPCEABI.H/fn_803A0208.cpp",
                cflags=cflags_eabi,
                # The native operators retain throw/specification unwind records.
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "Runtime.PPCEABI.H/__init_cpp_exceptions.cpp"),
        ],
    },
    {
        "lib": "Game",
        "mw_version": config.linker_version,
        "cflags": cflags_game,
        "objects": (
            [
                Object(Matching, "Game/gcaxSndMemory.c", cflags=cflags_game_gcax),
                Object(Matching, "Game/fn_8043B25C.c", cflags=cflags_game_gcax),
                Object(NonMatching, "Game/fn_8043BD80.c", cflags=cflags_game_gcax),
                Object(Matching, "Game/fn_8043D7B4.c", cflags=cflags_game_gcax),
                Object(Matching, "Game/fn_8043E2B4.c", cflags=cflags_game_gcax),
                Object(Matching, "Game/fn_8043FB58.c", cflags=cflags_game_gcax),
                # Matching workaround: -inline noauto on fn_80440410.c and fn_8044BAB4.c. With the
                # library default fn_80440D58..fn_80440E80 and fn_8044C56C..fn_8044CD4C grow (e.g.
                # fn_80440E80 0x198 -> 0x404, fn_8044C5FC 0x158 -> 0x1F8); the other units from
                # fn_80442B98.c through fn_80451A6C.c build to the same code and data with or
                # without the flag. Revisit when the original unit flags are known.
                Object(Matching, "rw/fn_80440410.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_80442B98.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80443044.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80443A74.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804447B0.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804448B8.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80445CF4.c", cflags=cflags_game),
                # Two functions differ. The hierarchy create function fn_804472AC: 7 of its 91
                # words, and it saves r26-r31 (its two relocations name _savegpr_26/_restgpr_26
                # instead of _savegpr_27/_restgpr_27) because maxInterpKeyFrameSize and the new
                # hierarchy get separate registers where the original shares r31. The hierarchy
                # attach function fn_804474D8: 5 of its 41 words (the original copies the node
                # index's zero into the node offset and keeps the index in r31).
                Object(NonMatching, "rw/fn_80446A04.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80447598.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804485BC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80448CB0.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8044920C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8044BAB4.c", cflags=[*cflags_game, "-inline noauto"]),
                # NonMatching, USA raw differing words per function (branch displacements included;
                # data unchanged): fn_8044CDC4, fn_8044CEE0, fn_8044CF68, fn_8044D554, fn_8044D6E8,
                # fn_8044D924 and fn_8044DC4C exact. fn_8044DA3C 3: the lis of the skin globals'
                # address is scheduled before the copy of the object pointer and takes r5 instead of
                # r3. fn_8044DD1C 11: the original adds the run's offset to the run table before the
                # first run's (ours adds the two offsets first), and the run counter and the first
                # run's offset take r24 and r25 the other way round. fn_8044DE74 35: the same
                # address order and register colouring in its two inlined copies of fn_8044DD1C, and
                # the matrix index and the normals flag of the rigid-skin loop take r17 and r18 the
                # other way round. fn_8044E408 23: register colouring of the weight offset and shift
                # counters, of two bit-mask temporaries and of the index loop, and two loop offsets
                # start as a copy of the zeroed counter instead of li 0 (which also moves the
                # zeroing of the weight total). fn_8044D344 111 (508 of 528 bytes): in the loop over
                # the bones without hierarchy matrices the original keeps a separate incremented
                # pointer for each of the three boneIds[i] reads and loads the 0 for
                # boneMatrix.flags in every iteration; ours shares one pointer and keeps the 0 in a
                # register.
                Object(NonMatching, "rw/fn_8044CDC4.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8044E6BC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80450D6C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80451A6C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80451F48.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80453018.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804535C8.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80453980.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80454AA4.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80455D0C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80458CF8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8045A760.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8045AF28.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8045B6F0.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8045BF30.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8045CA38.c", cflags=cflags_game),
                # Matching workaround: -inline noauto. At the default (-inline auto) fn_8045F2A0.o
                # has a .text of 0x758 bytes instead of the original 0x748 (measured). Revisit
                # when the unit's real flags are found.
                Object(Matching, "rw/fn_8045F2A0.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_8045F9E8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80460974.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804626F8.c", cflags=cflags_game),
                # Matching workaround: -inline noauto. At the default (-inline auto) fn_8046288C.o
                # has a .text of 0xB58 bytes instead of the original 0xAD0 (measured). Revisit
                # when the unit's real flags are found.
                Object(Matching, "rw/fn_8046288C.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_8046335C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80463D28.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804641CC.c", cflags=cflags_game),
                # fn_804649C0 and the data match. fn_80464814 differs in 28 of 107 instruction
                # words, all register numbers (opcodes, relocations and stack offsets are the
                # original's): the two component sizes, start and end take r29/r31/r30 instead
                # of r31/r30/r29, and r5-r8 are permuted among short-lived values (the two GQR6
                # settings, the morph target array, the position type and two table addresses).
                Object(NonMatching, "rw/fn_80464814.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80464A94.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8046528C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80465CDC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80466B58.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8046B110.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8046D8C8.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8046EE8C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8046FC1C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80470474.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80470AC4.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80473E9C.c", cflags=cflags_game),
                # fn_80474C7C and fn_80476830 expand their own recursive call one level deeper than
                # the original (49 and 39 words; 0x294 and 0x210 bytes against 0x224 and 0x1DC).
                # fn_80474FF8, fn_80475260 and fn_80476510 differ in register numbers only (16, 10
                # and 30 words). fn_804754F4 differs in 121 words and is 0x3E0 bytes against the
                # original 0x3D4: the flatness test's arithmetic is ordered differently, and the
                # halving loads bezier[0].y and bezier[1] again where the original reuses the
                # flatness test's loads (19 float loads from bezier against 16).
                Object(NonMatching, "rw/fn_80474BEC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80476ABC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80477344.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047832C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80479040.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80479598.c", cflags=cflags_game),
                # The line reader (fn_8047A518) is 79 words against the original's 78 (316 bytes
                # instead of 312). Counting differing words at the same position (and, in
                # parentheses, rows that differ once the instruction streams are aligned): 76 of 78
                # (18), because the extra word at 0x08 shifts every later word. maxLen is copied to
                # r7 because the buffer argument move is scheduled before maxLen - 1, and buffer and
                # file take r31 and r29 where the original has r29 and r31.
                Object(NonMatching, "rw/fn_80479A50.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047A7C8.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047B720.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8047CCF8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8047DB10.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8047DC90.c", cflags=cflags_game),
                # The orthonormalizer (fn_8047E298) retains 21 register-word differences: the
                # floating-point registers of the axis dot products that pick the pivot pair.
                Object(NonMatching, "rw/fn_8047E100.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8047FA28.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8048053C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80480C6C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804815F0.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8048222C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80482984.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80482BC4.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80483444.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8048349C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80483554.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804836EC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80483AFC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804842C8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80485BCC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80486888.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80487914.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80489B2C.c", cflags=cflags_game),
                # USA raw differing words: fn_8048A178 16 (eight first-block instructions in
                # another order: lis, the y shift, a store, two lfd, the compare, fsubs and fmuls;
                # registers and operands identical), fn_8048A790 40 (non-volatile register
                # assignment r20-r31 only; its inlined copies in fn_8048A96C match).
                Object(NonMatching, "rw/fn_8048A178.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8048ADE0.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8048B6AC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8048D7C0.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8048D85C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049043C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80491AB4.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804939DC.c", cflags=cflags_game),
                # fn_80496408 differs in 7 words: its error strings are addressed from a .data
                # base 0x74 bytes below the original's. The original loads 0x8056E4F8 (fn_80495428's
                # jump table, 0x74 bytes after fn_804951B8's at 0x8056E484) as its base. That suggests
                # the original file's .data starts there and fn_804951B8 belonged to another file
                # (inferred, not established).
                Object(NonMatching, "rw/fn_804951B8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80497198.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80497FB4.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049952C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80499B24.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049A78C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049A82C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049B0CC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049B14C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049B1F8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049B8BC.c", cflags=cflags_game),
                # fn_8049D934 differs in 10 words: in the duplicate check of the clusters of interest
                # the index, its 12-byte offset and the hoisted count take r7/r5/r6 instead of
                # r5/r6/r7. All other functions and the data match.
                Object(NonMatching, "rw/fn_8049D408.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049EA2C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049F2A4.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049F3F8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049F8AC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049FAFC.c", cflags=cflags_game),
                # Matching workaround: the original calls fn_804A2F24 from fn_804A457C; the default
                # limit inlines it, an inline_max_size of 350 to 480 keeps the call. Revisit when the
                # unit's real inline settings, or a source shape that keeps the call at the default
                # limit, are found.
                Object(Matching, "rw/fn_804A2C90.c", cflags=[*cflags_game, "-pragma \"inline_max_size(400)\""]),
                # Matching workaround: -inline noauto; at the default setting .text is 0x1738 bytes,
                # with the flag 0x1710 like the original's. Revisit when the unit's real inline
                # settings are found.
                Object(NonMatching, "rw/fn_804A55CC.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_804A6CDC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804A869C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804A90F8.c", cflags=cflags_game),
                Object(Matching, "Game/SpaceGadget.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/BackDrop.cpp", cflags=cflags_game_cpp),
                # All 35 functions have identical instructions. The unit's own data and its order
                # are the original's; the accessor of the switch counter is emitted after the
                # template instances that follow ApplyState here, directly after ApplyState in the
                # original.
                Object(NonMatching, "Game/CandleStick.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ChaoHolder.cpp", cflags=cflags_game_cpp),
                # 35 of 36 functions are identical in every word and relocation. The hit callback
                # (0x80130364) differs in 19 words: the float-to-int conversion of the level and
                # the int-to-float conversion of the index get separate 8-byte stack slots (the
                # original reuses one; frame 0xB0 instead of 0xA0), and the level and the
                # difference of the two table entries are held in f3 and f2 instead of f2 and f3.
                # The object also has a 32-byte local copy of std::swap (called by the weak
                # boost::function1 swap that the callback assignment instantiates), which the
                # original unit does not have.
                Object(NonMatching, "Game/CircusBall.cpp", cflags=cflags_game_cpp),
                # 23 of 29 functions are identical in every word and relocation (target and section
                # offset). The other six differ only in where data sits in the unit's sections. The
                # original has an unreferenced 0x40-byte table of floats (0, 0, 0, 0 and three rows of
                # 0, 0, 0, 1) in front of the TNumEdit names, which this build does not reproduce, so
                # the strings sit 0x40 lower in .rodata: 3 immediate words differ in 0x801307D8 and 6
                # in the constructor, and relocation offsets in the destructor and the allocator's
                # allocate. The TNumEdit and CharaColliBase type records that the headers add put
                # 0x30 bytes in front of .data and .sdata, which moves relocation offsets in the
                # constructor, the destructor and the functor manager. The registry accessor's guard
                # byte is the second byte of .sbss in the original and follows the pointer here (two
                # relocation offsets). The original's .rodata ends with the string
                # "string constructor: n > max_size\n", which this object does not have. This object
                # has a 32-byte local copy of std::swap (called by the weak boost::function1 swap)
                # and the sp_counted_base vtable and RTTI, which the original unit does not have.
                Object(NonMatching, "Game/CircusPole.cpp", cflags=cflags_game_cpp),
                # 80 of the 85 functions have identical instructions. The preload hook differs in 5
                # words (the registers of the table pointer and the stage number), SetupPath in 2
                # (the order of two argument moves), ResetFromOwner in 7 (the original calls the
                # smart pointer's operator-> out of line for the yaw of the character info) and
                # UpdateBody in 6 (the order of two argument moves and the register of the stage
                # pointer); the conversion fn_80131654 has no
                # function of that name in the object. The original builds the copies of the
                # follower pointer passed to fn_801CE9F4/fn_801CEA54 with fn_80131654 and destroys
                # them with fn_80009844, so their parameter is a shared pointer of another, unknown
                # type, and each copy differs in those two relocation targets. The RTTI and vtable
                # records differ in layout, and the object has additional weak template instances.
                Object(NonMatching, "Game/Coaster.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/Cream.cpp", cflags=cflags_game_cpp),
                # Every function but the ThreatObj constructor is instruction-identical, and the
                # functions are in the original's order. The constructor's two table loads (model
                # table and holder) use r6 and r7 the other way round (four words). ThreatObj's
                # destructor is a global symbol here (a plain out-of-class definition under
                # auto_inline) where the original's is weak. The object also has a 32-byte local
                # copy of std::swap<any_pointer> (called by the weak boost::function1 swap) that
                # the original unit does not have. The data matches except for records the
                # original unit does not have: the std::exception type record in .sdata and its
                # two name strings in .rodata come before those of checked_deleter<ThreatObj>
                # (moving them 8 and 0x1C bytes later), and the std::exception and sp_counted_base
                # vtables are appended to .data.
                Object(NonMatching, "Game/ThreatObjHolder.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecCircuit.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecCircuitEntrance.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/EscapePlaneRail.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecCircuitExit.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/WeaponStickBase.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/TestNakazawa.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/BDHologram.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/BDTrue.cpp", cflags=cflags_game_cpp),
                # All 14 functions have identical instructions; .data, .rodata and .sdata match and
                # .sdata2 is 4 bytes short of the original's trailing pad. The unit cannot replace
                # the original object: GetTaskGroup is local here, where the original's is
                # inferred to be a weak function with weak statics (seven functions in other units
                # call it and its statics lie outside the unit). An inline definition is expanded
                # into Create, and a global one keeps its statics local to the unit.
                Object(NonMatching, "Game/RainEffect.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/GravitySignBoard.cpp", cflags=cflags_game_cpp),
                # Every function, the data and all relocations match except ProcessHits, which is
                # one word short: the original branches twice on one comparison of the word at 0x50
                # of the character info (the second branch skips the cast check), and the single
                # natural test compiles to one branch fewer.
                Object(NonMatching, "Game/TornadoCollision.cpp", cflags=cflags_game_cpp),
                # 56 of the 78 functions have identical instructions; 44 of them also have identical
                # relocation symbol names, the others refer to symbols that keep placeholder names.
                # checked_delete<CityLaser> is a local symbol here where the original's is weak.
                # SetState differs in 5 words (the order of the stack argument of the probe call
                # and the moves around it).
                # The rest are the std::vector instances of weak pointers: insert, reserve and
                # swap differ in structure (the original reaches the capacity and the allocator
                # through more out-of-line accessors), the vector destructor and the __vector_obj
                # constructor differ, and 16 small helper functions (the constructor chain, the
                # accessors, the pair swap) are not reproduced one for one.
                Object(NonMatching, "Game/CityLaserHolder.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/FallBuildingHolder.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecCristalWallSwitch.cpp", cflags=cflags_game_cpp),
                # All 11 functions have identical instructions, and the whole object matches once
                # the 14 weak and local copies the linker discards are set aside, except the order
                # of the nine floats in .sdata2 (0x18-0x38): the original pools 70.0f, 1.0f, 0.1f,
                # 0.6f, 0.0f, 0.3f, 0.7f, 1.2f, 1.5f; this build pools 0.3f, 0.7f, 1.2f, 1.0f,
                # 1.5f, 0.0f, 0.1f, 0.6f, 70.0f (UpdateRotation's order of first use), so the
                # .sdata2 relocations of the functions point at other offsets.
                Object(NonMatching, "Game/ElecCube.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecPillar.cpp", cflags=cflags_game_cpp),
                Object(NonMatching, "Game/GrindGadget.cpp", cflags=cflags_game_cpp),
                # 11 of 12 functions have identical instructions. UpdateBody differs in 8 words, all
                # register numbers in the case that moves the footing back (the hidden depth and the
                # height are in f2 and f0 here, f0 and f1 in the original).
                Object(NonMatching, "Game/PopFooting.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecSmallPillar.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecWakka.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/GravityChangeCollision.cpp", cflags=cflags_game_cpp),
                # All 33 functions have identical words. The data sections are not compared: the
                # compiled object's .data and .sbss are 0x104 and 0x2D bytes against the original's
                # 0xC0 and 0x10.
                Object(NonMatching, "Game/GravityChangeZone.cpp", cflags=cflags_game_cpp),
                # The remaining differences: five allocations pass lower __LINE__ values (one word
                # each; the original passes 723, 965, 667, 773 and 1046); CopyFirstComponent
                # computes the length of the component once where the original subtracts the
                # pointers twice (five words); ResetEntry reaches its five float literals through
                # one pooled .rodata base where the original addresses each literal with its own
                # lis (33 words, shorter by seven; the pooled base is the only remaining
                # difference).
                Object(NonMatching, "Game/CSD/csdAccess.cpp", cflags=cflags_csd),
                # The range 0x8035C5CC-0x8035F81C is the one gap between csdAccess and csdMatrix;
                # its .rodata holds the file name string "csdLoader.cpp", which only
                # ExtractFileParts uses (the __FILE__ of its allocations), so the other functions
                # are in this unit because they share its address range. All 53 of its functions
                # are sourced (the RCPtr<CTexList> members from 0x8035EC38 are template instances).
                # The RenderState functions, ResolveProject, LoadProject, the Normalize*
                # functions, FindSceneNameIndex, ApplyScreenAspect, DrawScene, DrawLayer,
                # DrawElement, UnpackPatternBlend, PackPatternBlend, SplitFloat, SetEntryColors,
                # TintEntryColors, HoldKeyValue, InterpolateLinear, InterpolateColor,
                # TransformPoint, UpdatePalette, UpdateEntries, UpdateEntry, EvaluateChannels,
                # UpdateEntryTransform, RCPtr::Reset and the constructor of its holder match
                # (rodata operands and the holder's vtable compared by target). SwapBytes differs by
                # the register of the loaded value in four instructions. InterpolateKeys differs
                # in that the original saves its first argument to pass it again in the default
                # case. InterpolateHermite differs in instruction order, registers and one extra
                # address instruction in the original. ExtractFileParts has the original's 102
                # words and differs in 13: the original loads the file's first word after copying
                # the six arguments to saved registers, where this build loads it after the first
                # copy and interleaves the rest with the comparison (eight words), and the five
                # allocation __LINE__ values (471, 480, 494, 504 and 512 in the original).
                # DrawQuadElement has the original's 111 words and differs in 11 (the filter value
                # and the value passed to SetField0x24 have each other's saved registers, and one
                # load is placed differently). DrawTextElement (152 words, the original's 165)
                # copies the corners of a glyph without texture with a different address
                # computation (the original multiplies the vertex index by the vertex size for the
                # x and y of corners 1 to 3), plus register and scheduling differences.
                # EvaluateTrack (168 words, the original's 187) calls PackPatternBlend in case 6
                # where the original expands it in place (and first copies the interpolated value
                # to a second stack slot); outside case 6 it differs only in branch displacements,
                # stack frame offsets (the original's frame is 16 bytes larger) and the register
                # of one comparison operand. Counting differing words at the same position (and,
                # in parentheses, the original's words outside the runs that still match once the
                # instruction streams are aligned): SetEntryPattern (317 words, the original's
                # 319) differs in 201 (105): the original copies the vertices that change layout
                # word by word through integer registers and addresses some stores differently.
                # RCPtr::Assign (117 words, the original's 120) differs in 100 (24): the original
                # keeps the object to release in r4 and copies it to r3 for each call, and the
                # allocation of the holder passes line 138 of csdRCObject.h where the original
                # passes 410. Reset and the holder's constructor come after RenderState::End, as
                # in the original; the out-of-line CMemoryAlloc::AllocObject that the holder
                # allocation emits after them is a weak copy of a function the game has at
                # 0x8003DF84.
                Object(NonMatching, "Game/CSD/csdLoader.cpp", cflags=cflags_csd),
                Object(Matching, "Game/CSD/fn_80360528.cpp", cflags=cflags_csd),
                # Current file system and its directory (original file name unknown).
                Object(Matching, "Game/CSD/fn_80361738.cpp", cflags=cflags_csd),
                Object(Matching, "Game/CSD/fn_80361A24.cpp", cflags=cflags_csd),
                Object(Matching, "Game/CSD/fn_80361CF8.cpp", cflags=cflags_csd),
                # Matching workaround: -inline auto for this unit only (the rest of the library uses
                # -inline noauto). The texture array's constructor (0xA0 bytes) and destructor (0x54
                # bytes) are emitted out of line and also expanded in CTexListRW::Load and in the
                # destructor of CTexListRW, as in the original; under -inline noauto a function is
                # expanded only when it is declared inline, and then no out-of-line copy is emitted.
                # Revisit if the original's flags or an inline declaration that reproduces this are
                # found.
                #
                # The RenderWare implementations of the library's file system, drawing, allocator
                # and texture list, and the function that installs them (the file name is the one
                # its allocation calls pass). 39 of its 45 functions match in raw instruction words.
                # DrawPrimitive2D (49 words) differs in 10: the registers of the source vertex
                # pointer and the colour word are swapped. The draw without texture coordinates (52
                # words) differs in 27: the original copies the vertex count to r5 before the loop,
                # where this build copies it after the loop, so the loop uses other registers.
                # CTexListRW::Load (79 words) differs in 8: the registers of the dictionary and of
                # the array's first element are swapped, and the allocation passes line 141 of this
                # file where the original passes 70. The texture array's destructor matches; its
                # constructor differs only in passing line 141 for line 70.
                # InitLibraryRW (139 words) differs in 25 only by the .bss offsets of its
                # function-local statics: the original has 12 more bytes before the first record.
                # The holder's Assign (at 0x8036342C, 117 words, the original's 120) shows the
                # difference described for csdLoader.cpp: the original keeps the object to release
                # in r4, and the holder allocation passes line 410 of csdRCObject.h. The weak copies
                # of the allocator's default slots and of the CTexList, CMemoryAlloc and RCObject
                # vtables, which the link discards, are also emitted here. .data differs as well:
                # the vtables come out in a different order from the original's. .rodata differs
                # too (0x9D bytes, the original's 0xA8): the original aligns the assertion text and
                # the empty string to 8 bytes.
                # Matching workaround: -inline auto,deferred,level=1 here, while the rest of Chao::CSD uses
                # -inline noauto. The original expands the TexArray constructor and destructor in
                # CTexListRW::Load and ~CTexListRW and also keeps their out-of-line copies; -inline noauto
                # cannot produce both. Revisit if the original's flags or an inline declaration that
                # reproduces both are found.
                Object(
                    NonMatching,
                    "Game/CSD/csdTexListRW.cpp",
                    cflags=[*cflags_csd, "-inline auto,deferred,level=1"],
                ),
                # The texture list over an existing RenderWare texture dictionary (file name
                # unknown). 6 of its 7 functions match in raw instruction words; the holder's
                # Assign (117 words, the original's 120) differs as in csdTexListRW.cpp. The
                # constructor and destructor of the texture array are called out of line, as in the
                # original. The constructors of CTexListRW2, CTexListRW and CTexList and the
                # allocator's AllocObject and FreeObject are also emitted as weak copies, as are the
                # CTexList and RCObject vtables; the link discards them.
                Object(NonMatching, "Game/CSD/fn_80364058.cpp", cflags=cflags_csd),
                # The scene object and the handles it is built from: constructors, destructors, Clear and
                # Copy of the handles of a scene's data, fonts, motions, palette and entries; the scene's
                # frame range, speed, pause, motion and palette members, SetHidden, SetTranslation,
                # SetScale, root-pattern accessors, fn_8036635C, fn_803663C0 and the end-of-motion policy.
                # fn_8036674C initialises frame state through the scene-data accessor fn_8036679C.
                # fn_803659CC and fn_803659DC write the existing motion-end policy action fields.
                # 84 of the unit's 195 function bodies are represented. 81 have canonical symbol names,
                # including a weak copy of the inline CMotionPattern destructor and RCPtr<CNode>::Reset.
                # Of these, 75 have the original's instructions; GetRootPattern and SetRootPattern differ
                # only in their allocation lines (297 vs 1124 and 284 vs 1102, respectively). The
                # 100-byte fn_8036635C differs in seven instruction words: layer-entry address formation,
                # registers and scheduling. Two more template bodies are the node holder's constructor
                # (identical instructions, but a weak vtable copy) and its delete-policy Assign (0x1D4 vs the
                # original's 0x1E0 at fn_803655F8, with register, scheduling and allocation-line differences).
                # The separate byte-holder Assign is 0x1C0 bytes; it is not the original node-holder Assign.
                # RCPtr<CNode>::Reset is emitted just after SetRootPattern, rather than after
                # CMotionPatternRef::Clear as in the original; this order difference also remains.
                # The full frame update fn_803669B8 has the original's 0x5C0 bytes; twelve instruction
                # words differ because its member-pointer constants have different offsets.
                # The callback-list erase has the original's 0x90 instruction bytes. Its allocator's
                # destroy body is represented, but the native USA copy keeps the placeholder
                # fn_80367D3C. The callback-reference destructor is 0x18C bytes versus 0x198;
                # the matrix-reference destructor is
                # 0x178 bytes versus 0x184. Both omit three argument moves, with corresponding
                # register and branch-displacement differences.
                # The other 111 bodies are not sourced: the scene's Copy, destructor and Draw,
                # the end-of-motion policy's destructor, CSceneListener's constructor, the listener and
                # node containers, remaining RCPtr members and listener dispatch. Unit data is incomplete.
                # The diagnostic object also emits weak byte-holder Reset, constructor and vtable copies,
                # CMemoryAlloc::AllocObject and FreeObject, and a weak node-holder constructor and vtable
                # whose destructor remains external. A weak RCPtr<unsigned char> destructor is
                # emitted as well. The original scene object and holder providers are
                # retained by the link; this source object is not selected.
                Object(NonMatching, "Game/CSD/csdmScene.cpp", cflags=cflags_csd),
                # The accessors that override the animated values of an element's entry
                # (CMotionPattern), its constructor and Copy, and UpdateTransform. 32 of 32
                # functions are sourced. 15 have identical instructions; 13 differ only in the
                # __LINE__ passed by their allocations (one word each, two in UpdateTransform; the
                # original's lines run from 54 to 1067 in a file far longer than this one). The
                # holder Assign for the free and function release policies (112 words, the
                # original's 115) shows the difference described for csdLoader.cpp. GetTranslation
                # and GetScale (49 words, the original's 45) copy their result through the float
                # registers where the original uses the integer ones. The .rodata holds the same
                # four strings in a different order (the original has the holder's error text
                # first). The unit also emits weak copies of the CMotionPattern destructor
                # (identical to the game's copy, which comes from csdmScene.cpp),
                # RCObject<unsigned char>::SetRelease, CMemoryAlloc::AllocObject and
                # CMemoryAlloc::FreeObject and of the RCObject vtables for unsigned char and
                # MatrixValue, which the link discards. Its .bss holds the free release policy as
                # one byte, where the game's symbol spans the 8 bytes up to the next one.
                Object(NonMatching, "Game/CSD/csdmMotionPattern.cpp", cflags=cflags_csd),
                # The project object (CProject: the file data of a project with the texture list and
                # the fonts its scenes use) and the handlers of the scene's end-of-motion policy.
                # 18 of the unit's 178 functions are sourced. 14 have the original's instructions:
                # the policy's OnReachEnd and OnReachStart, the four empty events of CSceneListener,
                # CProject::Load, GetTexList and SetTexList, the CFontList constructor and
                # CFontListRef::Attach used for the project's fonts, and the free-policy
                # CResourceBase::Attach, RCPtr<unsigned char>::Reset and RCObject constructor.
                # Copy and CProject::Attach differ in one word each, the __LINE__ of their
                # allocations (the original passes 83 and, in csdmProject.h, 197; this source is far
                # shorter than the original files). LoadFile (161 words, the original's 162) differs
                # in 122 (7) words: the __LINE__ of its two Alloc calls (the original passes 134 and
                # 161), the first strrchr, whose result the original compares before copying it
                # (cmplwi, mr) where ours emits one mr., and three branch displacements before it.
                # The free-policy Assign (112 words, the original's 115) differs in 95 (22) words,
                # including branch displacements, and shows the difference described for
                # csdLoader.cpp. The functions are defined in reverse address order, except
                # CFontListRef::Attach: the original has it between the CFontList constructor and
                # CResourceBase::Attach, after Load, where ours comes before Load. The other 160
                # functions are not sourced: the containers (the maps of scenes by name and by draw
                # order and the lists of scenes waiting to be added or removed, with their
                # iterators), the project's destructor, SetFonts, Update and Draw, the creation of
                # scenes (the CScene constructor included) and the listener base class. The unit's
                # .rodata and .data are incomplete for the same reason. It emits weak copies of the
                # free-policy RCObject vtable, of CMemoryAlloc::AllocObject and FreeObject and of
                # CMotionEndPolicy's destructor (the original's is in csdmScene.cpp), which the link
                # discards.
                Object(NonMatching, "Game/CSD/csdmProject.cpp", cflags=cflags_csd),
                Object(Matching, "Game/fn_80008D48.c"),
                Object(Matching, "Game/fn_80008DA4.c"),
                Object(Matching, "Game/fn_80008E1C.c"),
                Object(Matching, "Game/fn_800091D0.cpp"),
                # Matching workaround: -sdata 0 and -use_lmw_stmw on. The target reaches lbl_805C0AF0
                # and lbl_805C0AF4 with absolute @ha/@l addressing (lis with lwz/addi), not
                # r13-relative small data (without -sdata 0 the unit is 69% matching), and saves its
                # callee-saved registers with lmw/stmw (without it 87%). Both flags are also set on
                # the CRI library units, such as its caller CRI/uty_tmr.c. Revisit when the original
                # unit's flags are known.
                Object(
                    Matching,
                    "Game/fn_803CC9D0.c",
                    extra_cflags=["-sdata 0", "-use_lmw_stmw on"],
                ),
                # CRI middleware functions (MWSFD); the original source file names are not yet
                # established.
                Object(Matching, "Game/fn_80403E44.c"),
                Object(Matching, "Game/fn_8040E048.c"),
                # The units below that use -Cpp_exceptions on are those whose original object
                # carries extab/extabindex entries (C++ exception tables), plus Game/fn_8041583C.c,
                # the middle piece of a file whose other pieces carry them; the default is off.
                Object(
                    Matching,
                    "Game/fn_8040ABC8.cpp",
                    cflags=cflags_game_cpp,
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                Object(Matching, "Game/fn_8041FD6C.c"),
                Object(Matching, "Game/fn_8041FF08.c"),
                Object(Matching, "Game/fn_80420438.c"),
                Object(Matching, "Game/fn_80425858.cpp", cflags=cflags_effect),
                # All six method bodies have identical instruction words and named relocation
                # targets. Live local template copies have their original providers in
                # fn_80425860.cpp, so this split is still NonMatching. Linked as a whole object
                # (which first needs a name for 0x803A1754, the function the shared_count
                # constructor calls after __throw, here __end__catch), it still keeps a local
                # 0x18-byte boost operator== copy for shared_ptr<EffectElement> that the
                # original calls at 0x80426224 and extab and extabindex records for the template
                # copies. The object also emits .rodata and .data (the type records, name strings
                # and vtables of the boost template copies), but the linked image keeps none of
                # them. Of its .bss it keeps one byte, the object that the local
                # shared_ptr<EffectElement> constructor copies into its checked_deleter argument;
                # the original's counterpart is lbl_805E1500. Revisit when fn_80425860.cpp can be
                # merged into this unit.
                Object(NonMatching, "Game/EffectManager.cpp", cflags=cflags_effect),
                Object(NonMatching, "Game/fn_80425860.cpp", cflags=cflags_effect),
                Object(Matching, "Game/EffectArranger.cpp", cflags=cflags_effect),
                Object(Matching, "Game/fn_8040D9D8.c"),
                Object(Matching, "Game/fn_8040D498.c"),
                # 3x4 matrix helpers. Seven of 28 functions still differ (raw differing instruction
                # words, by position, or the larger length minus the exact LCS where the sizes
                # differ). The three axis rotations (fn_8040B0C0, fn_8040B138, fn_8040B1B0; 2 words
                # each) restore r31 before loading r0 in the epilogue, where the target loads r0
                # first, and the look-at matrix (fn_8040B33C; 2 words) swaps one fneg with the next
                # lfs. The quaternion matrix (fn_8040B228; 6 words) has the target's instructions
                # and registers with three neighbouring pairs in the other order. The inverses
                # (fn_8040BCD4, 140 words; fn_8040BEF8, 145 words) differ in scheduling, registers
                # and saved registers: fn_8040BEF8 has the target's arithmetic opcodes and saves 1
                # more FPR (152 instructions instead of 148); fn_8040BCD4 reuses the determinant's
                # products m10 * m22 and m12 * m20 for element [1][0], which the target computes
                # again (2 fsubs and 1 fneg where the target has 1 fmsubs and 1 fnmsubs), and saves
                # 2 more FPRs (146 instructions instead of 137).
                Object(
                    NonMatching,
                    "Game/fn_8040B034.cpp",
                    cflags=cflags_game_cpp,
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                Object(
                    Matching,
                    "Game/fn_8040CC6C.cpp",
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                # Vector length helpers (also Game/fn_8040CF50.cpp and Game/fn_8041116C.cpp), which
                # call std::sqrt(float) from MSL <cmath>. One level of inlining keeps that call out
                # of line, because std::sqrt's body calls the inline sqrtf; plain -inline auto
                # expands both into the helper. Each object also carries weak copies of std::sqrt and
                # sqrtf with their literals, which the link discards in favour of the copies at
                # 0x8000FEE4 and 0x8000FF04. Those copies match <cmath> compiled with fp_contract on
                # (cflags_base).
                Object(
                    Matching,
                    "Game/fn_8040CCEC.cpp",
                    cflags=cflags_game_cpp,
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                # SonicUSA::System::Random. Matching workaround: -sdata 0 and -sdata2 0; the target
                # keeps its doubles in .rodata and its RTTI record and twist table in .data. Revisit
                # when the original unit's flags are known.
                Object(
                    Matching,
                    "Game/fn_8040EA90.cpp",
                    cflags=cflags_game_cpp,
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-sdata 0",
                        "-sdata2 0",
                    ],
                ),
                # Interpolation helpers. Matching workaround: the target keeps its float constants in
                # .rodata (-sdata2 0); revisit when the original unit's flags are known.
                Object(
                    Matching,
                    "Game/fn_8040EFA4.cpp",
                    cflags=cflags_game_cpp,
                    extra_cflags=["-Cpp_exceptions on", "-sdata2 0"],
                ),
                # Rotation and segment helpers (Matching workaround: float constants in .rodata,
                # -sdata2 0; revisit when the original unit's flags are known; <cmath> leaves the
                # double constants and statics of the inline std::sqrt/sqrtf and the weak copies
                # of the inline math functions and of operator*, which the link discards).
                Object(
                    Matching,
                    "Game/fn_8040F364.cpp",
                    cflags=cflags_game_cpp,
                    extra_cflags=["-Cpp_exceptions on", "-sdata2 0"],
                ),
                # Motion integration (Matching workaround: float constants in .rodata, -sdata2 0;
                # revisit when the original unit's flags are known; <cmath> leaves the weak
                # sqrtf static locals, which the link discards).
                Object(
                    Matching,
                    "Game/fn_804103E8.cpp",
                    cflags=cflags_game_cpp,
                    extra_cflags=["-Cpp_exceptions on", "-sdata2 0"],
                ),
                # The library code at 0x80410630-0x8042166C is built with one flag set (-O4,s,
                # -use_lmw_stmw on, -str reuse,readonly, -inline auto,level=1), taken as the
                # library's own build settings rather than per-unit workarounds. Game/fn_80410630.c,
                # Game/fn_8041116C.cpp, Game/fn_80411714.c, Game/fn_80413B30.c,
                # Game/fn_804157E8.cpp, Game/fn_8041583C.c, Game/fn_80415854.cpp and the later
                # NonMatching pieces of the control of motions (Game/fn_80416198.cpp,
                # fn_8041FD70.cpp, fn_8041FF0C.cpp, fn_80420308.cpp and fn_8042043C.cpp) and the
                # later units from Game/fn_80420724.cpp to Game/fn_804211CC.c carry it.
                # Game/fn_804110EC.c, Game/fn_804202B8.c and the empty functions Game/fn_8041FD6C.c,
                # Game/fn_8041FF08.c and Game/fn_80420438.c omit it: their objects are identical
                # with or without it.
                # Loader of packs of named nodes. Inlining is automatic but one level deep: the
                # recursive node functions expand once, as in the original (with unlimited depth
                # fn_80410C7C grows from 0xC4 to 0x304 bytes).
                Object(
                    Matching,
                    "Game/fn_80410630.c",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # Replaceable memory functions of the library code at 0x80410630-0x8042166C.
                Object(Matching, "Game/fn_804110EC.c", extra_cflags=["-Cpp_exceptions on"]),
                Object(Matching, "Game/fn_8040DB6C.c"),
                # The middle piece of the control of motions (see Game/fn_804157E8.cpp
                # below); it takes the same flags as the other two pieces.
                Object(
                    Matching,
                    "Game/fn_8041583C.c",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                Object(Matching, "Game/fn_8040D608.c"),
                # Transform helpers over the current matrix stack: a C++ unit of the library, built
                # with the MSL headers for <cmath> and the library's flag set. The original keeps
                # the functions in definition order; the deferred inlining of cflags_game_cpp emits
                # them in reverse order and moves the 1.0f .sdata2 literal after the doubles (0x34
                # bytes instead of 0x30).
                Object(
                    Matching,
                    "Game/fn_8041116C.cpp",
                    cflags=[*cflags_base, *msl_includes],
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # Groups of numbered resources. Not complete (differing words, GUPE8P; for the two
                # functions of different length, original words left unmatched by
                # difflib.SequenceMatcher(autojunk=False) over the instruction words):
                # fn_804124DC 12 of 70 (r5 and r6 swapped in the inlined item lookup); fn_80413058 7
                # of 122 (r5 and r6 swapped in the overlap loop, and li r30,0 where the original
                # copies mr r30,r29 before the last loop); fn_80413318 0x21C bytes instead of 0x220,
                # 74 of 136 words differ: the original branches to an else block (li) for a zero
                # records offset where this build sets 0 first, it schedules one subf before two
                # register copies where this build has it after them, and it allocates the saved
                # registers differently; fn_80413538 0x478 bytes instead of 0x47C, 94 of 287
                # words differ: the original computes the name bytes of a group that owns itself in
                # r0 and copies them (mr r31,r0), and gives the saved registers in another order.
                # The .bss objects come out in first-use order; the original has nameBuffer first,
                # the definition order a C++ compile keeps, and fn_804124DC's initialisation test is
                # the code MWCC gives the guard of a C++ function-local static.
                Object(
                    NonMatching,
                    "Game/fn_80411714.c",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # Reading of animation curves. The flags follow the library code (fn_80410630.c,
                # where the inline depth is shown); with level=1 the key reader must be a macro.
                # Not complete: the registers differ in fn_80414120 (29 differing words),
                # fn_804142B0 (6), fn_80414438 (17), fn_80414CBC (38), fn_80414FD0 (60),
                # fn_8041474C and fn_804149E0 (41 each, including the larger frame; the original
                # also saves r31, 2 more words) and fn_80415530 (58, including the frame offsets;
                # the original also saves f28, 4 more words). The instruction sequences are
                # otherwise the same.
                Object(
                    NonMatching,
                    "Game/fn_80413B30.c",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # Control of motions: the pieces of one original C++ file (0x804157E8-0x80420724,
                # see Game/fn_804157E8_motion_control.h) that do not match yet, kept as separate
                # units around the Matching pieces Game/fn_8041583C.c, fn_8041FD6C.c, fn_8041FF08.c,
                # fn_804202B8.c and fn_80420438.c until the whole file matches. The flags follow the
                # neighbouring library code. The pieces that do not match yet are C++, the language
                # of the file (it has a static initializer, and fn_80415854.cpp calls the vector's
                # out-of-line assignment). Game/fn_804157E8.cpp has the original's .text and
                # exception tables, and its .sdata and .sbss hold the original's variables at the
                # original offsets (the original's sections end with 4 more bytes of alignment
                # padding; a C compile gives the .sbss variables in reverse definition order). It
                # cannot link yet: it owns the file's first .sdata2 constant (0x805FAE00, 0.0f),
                # which fn_80415854.cpp, fn_80416198.cpp and fn_8042043C.cpp read by name, and here
                # it is a local literal.
                Object(
                    NonMatching,
                    "Game/fn_804157E8.cpp",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # The .text bytes of thirteen of the fourteen functions and the exception tables
                # are identical to the original; fn_80415EA4 differs in one word (li r30,0 where the
                # original copies mr r30,r28 before its player loop). Not complete: the original reads 0.0f from the file's first
                # pooled constant 0x805FAE00 (Game/fn_804157E8.cpp), and other pieces of the file
                # read this unit's 0x805FAE04 (fn_80416198.cpp) and 0x805FAE08 (fn_80416198.cpp,
                # fn_80420308.cpp, fn_8042043C.cpp) by name. Here 0.0f is a literal of this unit, so
                # .sdata2 is 0xC bytes (0.0f, the epsilon, 1.0f) instead of 0x8, and 16 relocations
                # name other targets: the ten 0.0f loads of fn_80415854, fn_80415A18 and the three
                # stMtnRequestBlend functions, and the four epsilon and two 1.0f loads, whose
                # literals sit 4 bytes further on. Revisit when the pieces of the file are merged
                # into one unit.
                Object(
                    NonMatching,
                    "Game/fn_80415854.cpp",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # stMtnAbortBlend to fn_8041FD0C: 29 of the 67 functions are written. A word differs
                # when its instruction or its relocation target (symbol, or section and offset)
                # differs; the words are counted positionally and, where noted, as the larger length
                # minus the exact longest common subsequence (LCS). 16 are identical. Many loads of
                # 0.0f, 0.0001f and 1.0f differ only in their target: the original takes these from
                # the shared pool of Game/fn_804157E8.c and fn_80415854.cpp. The other constants of
                # this unit have the original values but sit 4 bytes later in its .sdata2 (the unit
                # defines the pool values itself and lacks the constants of the missing functions).
                # fn_804172D8 and fn_8041734C differ in 2 pool loads each, fn_8041E420 and
                # fn_8041FD0C in 1, fn_8041CC18 and fn_8041CD6C in 4, fn_8041AD94 in 8 (6 pool loads
                # and 2 loads of -0.0001f) and fn_804188A0 in 2 (a pool load and the conversion
                # constant), and stMtnControlWorkFlag only in the 2 words that address its jump
                # table, which the original keeps in .data at 0x80569F28. The others:
                # stMtnRequestMotion is 28 words against 30 in the original (25 positional, 2 by
                # LCS: the original tests the control a second time before the lookup of the
                # player); fn_8041E480 32 words (19 by LCS; the loads of the conversion constant and
                # of the matrix stack are scheduled in another order, as in fn_8041FD70, and 4 loads
                # of the angle constants differ in placement); fn_8041AFCC 18 words (14 by LCS) and
                # fn_8041B6FC 94 words against 98 (85 positional, 41 by LCS): the original forms the
                # address of the tracks of a step (0x10) before indexing them, and fn_8041B6FC also
                # keeps the address of the curves of each track in a register. The .bss at
                # 0x805E12E0 that many of the missing functions use is constructed by the static
                # initializer of the file, which this unit does not have yet.
                Object(
                    NonMatching,
                    "Game/fn_80416198.cpp",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                Object(
                    NonMatching,
                    "Game/fn_8041FD70.cpp",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # The instructions of all three functions, the exception tables and the relocation
                # types are identical to the original; the six 2.0f loads are relocated against this
                # unit's own literal instead of the shared pool's 0x805FAE2C. That literal is the
                # last of the original file's shared .sdata2 pool (0x805FAE00-0x805FAE30), and this
                # unit's 8-aligned pool cannot begin at 0x805FAE2C (4 mod 8). Revisit when the
                # pool-owning functions are merged into one unit.
                Object(
                    NonMatching,
                    "Game/fn_8041FF0C.cpp",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                Object(
                    NonMatching,
                    "Game/fn_80420308.cpp",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # fn_8042043C, fn_804204C8, fn_80420590 and fn_8042062C have the original's
                # instructions, but the 0.0f of fn_804204C8 and the 1.0f of fn_80420590 are local
                # literals here, while the original reads them from the file's shared .sdata2 pool
                # (0x805FAE00 in Game/fn_804157E8.cpp and 0x805FAE08 in Game/fn_80415854.cpp). The
                # static initializer fn_804206A4 (0x80 bytes) is missing: it constructs nine
                # matrices and four 0xC-byte objects (constructor fn_80028BA0) in lbl_805E12E0, which
                # is in the .bss of Game/fn_80416198.cpp. Revisit when the pieces of the file are
                # merged into one unit.
                Object(
                    NonMatching,
                    "Game/fn_8042043C.cpp",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # Quaternion helpers that follow the control of motions in the same library. All
                # three functions are identical to the original, but the pool of float literals
                # (.sdata2 0x805FAE30-0x805FAE48) is in another order: the original's is 0, 2, 1,
                # 0.9999, 0.5, 4 and ours is 1, 0, 0.9999, 0.5, 4, 2 (the order of first use), so
                # the literals are addressed at other offsets. The original pools 0 and 2 before
                # fn_80420724's literals, although 2 is first used in fn_804208EC: a throwaway
                # function storing 0.0f and then 2.0f placed first reproduces the original order,
                # so the pool was most likely started by code the executable does not contain.
                # Matching needs that code's real form. Built with the MSL headers for <cmath>.
                Object(
                    NonMatching,
                    "Game/fn_80420724.cpp",
                    cflags=[*cflags_base, *msl_includes],
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # Support code of the same library after the quaternion helpers: the start-up and
                # shut-down wrappers with a name registry, the byte reversal (two functions that do
                # not match yet) and the ARAM transfers and allocator.
                Object(
                    Matching,
                    "Game/fn_80420C30.c",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                # Not matching: fn_8042118C (4 of 16 words differ): the two byte loads of each pair
                # get each other's registers (a register swap). fn_804210E4
                # (32 of 42 words differ; 35 words against 42) tests the address of each of its two
                # 16-bit halves twice before reversing the bytes, and the original hoists only the
                # second address out of the loop; no single-level inline form tried (a null test
                # inside the inline function, around the call, or both) produces the double test.
                Object(
                    NonMatching,
                    "Game/fn_804210E4.c",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                Object(
                    Matching,
                    "Game/fn_804211CC.c",
                    extra_cflags=[
                        "-Cpp_exceptions on",
                        "-O4,s",
                        "-use_lmw_stmw on",
                        "-str reuse,readonly",
                        "-inline auto,level=1",
                    ],
                ),
                Object(Matching, "Game/fn_8040CF2C.c"),
                Object(Matching, "Game/fn_804202B8.c"),
                Object(Matching, "Game/fn_8040CD60.cpp"),
                Object(Matching, "Game/fn_8040D02C.cpp"),
                Object(
                    Matching,
                    "Game/fn_8040D044.cpp",
                    # Matching workaround: the original saves its two callee-saved registers with
                    # stmw/lmw, which this unit's code only gets when optimized for size with
                    # -use_lmw_stmw; revisit when the original unit's flags are known.
                    extra_cflags=["-Cpp_exceptions on", "-O4,s", "-use_lmw_stmw on"],
                ),
                Object(
                    Matching,
                    "Game/fn_8040CE74.cpp",
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                Object(
                    Matching,
                    "Game/fn_8040CF50.cpp",
                    cflags=cflags_game_cpp,
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                Object(Matching, "Game/fn_8040CF90.cpp"),
            ]
            if config.version == "GUPE8P"
            else []
        ) + [
            # These units are enabled for all regions; the conditional list above remains USA-only.
            # The one and zero literals remain local copies of the unowned shared original pool.
            Object(NonMatching, "Game/fn_8040C908.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8040C924.cpp", cflags=cflags_game_cpp),
            # The one, zero and threshold literals remain local copies of the shared original pool.
            Object(
                NonMatching,
                "Game/fn_8040C958.cpp",
                cflags=cflags_game_cpp,
                # The original retains eight-byte extab and twelve-byte extabindex records.
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "Game/fn_8040CA10.cpp", cflags=cflags_game_cpp),
            # The 0.0001f pool is shared with fn_8000ABA8/fn_8000AD08 in the original larger unit.
            Object(NonMatching, "Game/fn_8000B044.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80016260.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_801B1DB0.cpp", cflags=cflags_game_cpp),
            Object(
                Matching,
                "Game/fn_8040D0AC.cpp",
                cflags=cflags_game_cpp,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "Game/fn_8040CD74.cpp", cflags=cflags_game),
            Object(
                NonMatching,
                "Game/fn_8040C874.cpp",
                cflags=cflags_game_cpp,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(
                Matching,
                "Game/fn_8040CEE8.cpp",
                cflags=cflags_game,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(
                Matching,
                "Game/fn_8040D1B4.cpp",
                cflags=cflags_game,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            # The next three functions have identical instructions; each reads floats outside
            # its split that other functions also read (0x805F1AF8; 0x805F1C80 and 0x805F1C84;
            # 0x805F18E8), where these one-function units keep local copies.
            # fn_8003B9E8 and fn_800498A0 also emit weak std::sin, std::cos and sqrtf constant
            # copies, which the linker discards.
            Object(NonMatching, "Game/fn_8003B9E8.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_800498A0.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_8001E070.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8040CBF8.cpp"),
            Object(Matching, "Game/fn_8040CA84.c", cflags=cflags_game),
            Object(
                Matching,
                "Game/fn_8040C340.cpp",
                cflags=cflags_game_cpp,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(
                Matching,
                "Game/fn_8040C3AC.cpp",
                cflags=cflags_game_cpp,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "Game/fn_8040CAB4.cpp"),
            Object(Matching, "Game/fn_8040C244.c"),
            Object(Matching, "Game/fn_8040CCD4.c"),
            Object(Matching, "Game/fn_8040CD20.c"),
            Object(Matching, "Game/fn_8007D624.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8007D770.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8001C4B8.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80021C04.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80020A20.cpp", cflags=cflags_game_cpp),
            # Both original units own C++ exception-table records.
            Object(
                Matching,
                "Game/fn_8040C25C.cpp",
                cflags=cflags_game_cpp,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            # Matching workaround: extab_padding writes the original's two padding bytes of the
            # constructor's exception action record (extab+0x16, ac 02 in all three regions). MWCC
            # fills that padding with leftover stack contents, so no compiler host reproduces it;
            # the code and the meaningful exception data match.
            Object(
                Matching,
                "Game/fn_8040C2B8.cpp",
                cflags=cflags_game_cpp,
                extra_cflags=["-Cpp_exceptions on"],
                extab_padding=b"\xac\x02",
            ),
            # All 46 functions except LoadFile match individually; its allocation passes line 47
            # instead of 137. The map helpers emit in a different order, and an extra weak
            # CMemoryAlloc::FreeObject copy is emitted without a reference from this unit.
            Object(NonMatching, "Game/CSD/csdTexList.cpp", cflags=cflags_csd),
            Object(Matching, "Game/CSD/csdMatrix.cpp", cflags=cflags_csd),
            Object(Matching, "Game/ResourceOneFile.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80014D58.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80014EC0.cpp", cflags=cflags_game_cpp),
            # The fn_8040CB58 member owns the original extab/extabindex records.
            Object(
                Matching, "Game/fn_8040CB2C.cpp", extra_cflags=["-Cpp_exceptions on"]
            ),

            # The zero-tolerance literal still belongs to the shared original pool.
            # The 0.0001f literal lies four bytes past an 8-byte boundary in the shared .sdata2 pool
            # (USA 0x805F1714; Japan 0x805F1D34; PAL 0x805F27B4); a separate 8-aligned pool
            # cannot start there.
            Object(NonMatching, "Game/fn_8000A950.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8000A980.cpp", cflags=cflags_game_cpp),

            # Tail of a larger original file whose functions own the pool at 0x805FAD50:
            # 1.0f, 0.0f, 0.99999f, 0.5f and 1e-5f are used by fn_8040C3EC, fn_8040C55C,
            # fn_8040C874, fn_8040C908 and fn_8040C958. fn_8040CB58 uses the next pool at
            # 0x805FAD68. External literal symbols would hide this artificial unit boundary;
            # revisit when the data-owning functions can be sourced and merged into this unit.
            Object(NonMatching, "Game/fn_8040CAF8.cpp"),
            # Preserve the legacy USA status; Japan and PAL remain NonMatching because the clear
            # value belongs to the larger original file's shared .sdata2 pool (0x805FAD78-0x805FAD88).
            # Revisit when the pool-owning functions are merged into one unit.
            Object(Matching if config.version == "GUPE8P" else NonMatching, "Game/fn_8040CC5C.c"),
            # The clear value belongs to the same original-file pool; keep the ordinary literal
            # NonMatching until the pool-owning functions are merged into one unit.
            Object(NonMatching, "Game/fn_8040CE60.c"),
            Object(Matching, "Game/fn_8040E310.c"),
            Object(Matching, "Game/fn_8040E38C.c"),
            # Registers/scheduling differ; the literals continue the preceding functions' pool.
            # This one-function unit's 8-aligned pool cannot begin at 0x805F1984 (4 mod 8).
            Object(NonMatching, "Game/fn_80027D5C.cpp", cflags=cflags_game_cpp),
            # The palette object of a scene (CMotionPalette, which owns the palette block
            # through a reference-counted handle) and the CMotionPattern constructor that wraps
            # one of its entries. 12 of 12 functions are sourced; 8 match in raw instruction
            # words. Source read-only data is 70 bytes against the original 72-byte extent.
            # The CMotionPalette constructor and Rebuild differ in one word each, the __LINE__
            # of their allocations (the original passes 55 and 100). The holder Assign for the
            # function release policy (112 words, the original's 115) shows the difference
            # described for csdLoader.cpp. GetPattern retains seven positional setup/load
            # scheduling differences, including an equivalent palette-pointer base register.
            # The holder's SetRelease matches but comes last in the unit, where the original
            # has it right after the holder constructor for the function policy (see csdRCObject.h).
            # The source also emits weak copies of RCPtr<unsigned char>::Reset, the holder
            # constructor for the delete policy and CMemoryAlloc::AllocObject, plus a local
            # delete-policy Assign referenced only by that source Reset. It also emits a weak
            # RCObject<unsigned char> vtable (the original unit has no .data). Normal links
            # select the original unit and its existing providers; these links do not include
            # this source object or establish which source weak copies would be discarded.
            Object(NonMatching, "Game/CSD/csdmMotionPalette.cpp", cflags=cflags_csd),
            # All 25 functions and their owned data are represented. 23 of the 25 functions have
            # identical instruction words and relocation targets; BuildTextEntry and InitPalette
            # remain NonMatching. BuildTextEntry is 0x540 bytes versus 0x564, with 316 differing
            # overlapping instruction words and nine unpaired native words. InitPalette is 0x190
            # versus 0x19C, with 99 differing overlapping words and three unpaired native words.
            # The differences include control/register choices, allocation lines and InitPalette's
            # pooled read-only-data addressing. The original allocation lines are 442 and 234.
            # The scoped recursive expansion keeps MarkMatrixUsers in its original position.
            # Read-only data matches all 48 bytes; the three BSS words retain their offsets, with
            # 12 source bytes versus the original 16-byte extent including trailing alignment.
            Object(NonMatching, "Game/CSD/csdMotionPalette.cpp", cflags=cflags_csd),
            # The complete node source, including its constructor, destructor, callbacks, channel
            # map and owned data. The candidate emits 96 of the 97 native function bodies; the
            # 12-byte tree iterator arrow is represented by its inlined header definition.
            # 92 emitted bodies have identical instruction words. The channel-transfer switch is
            # 0x338 versus 0x2D0, with 159 differing positional words and 26 extra words. The original
            # translate-X/Y cases read the pair directly; the other 13 call outlined iterator and
            # reference arrows. This build expands the arrows in all 15 cases.
            # The constructor differs in its allocation line (45 versus 51).
            # The callback reference destructor is 0x18C versus 0x198, with 82
            # differing positional words and three omitted argument moves. The callback-list
            # constructor is 0x50 versus 0x58, with 12 differing positional words: the default
            # node allocator replaces the original conversion from the value allocator.
            # Both member pointers, all three vtable groups and the literal contents are represented.
            # The switch-table addends differ, and a weak byte-holder vtable shifts the
            # element-data-reference vtable.
            # Function order also differs: the map destruction group (__destroy, deallocate,
            # __node_alloc, allocator destroy and __alloc) follows the tree destructor, ahead of
            # root(), instead of following fn_8036B9FC. The pair destructor moves into that group
            # from its original position before ClearHidden. The element-data-reference
            # destructor follows root(), rather than the node destructor. Map and callback-list
            # constructors precede Attach/Clear instead of following them. fn_8036B9FC follows Copy
            # instead of the callback compressed-pair constructors. The two adjustment thunks are
            # reversed: GetSender precedes the destructor.
            # Extra weak reference/container copies and their support data are also emitted. A local,
            # non-weak 0x1C0 delete-policy Assign is reached only by the weak RCPtr<unsigned char>::Reset.
            # Normal links retain the original object in every region.
            Object(NonMatching, "Game/CSD/csdmNode.cpp", cflags=cflags_csd),
            Object(Matching, "Game/fn_80293AC8.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8004924C.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8004EDAC.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Task.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80165EFC.cpp", cflags=cflags_game_cpp),
            Object(
                Matching,
                "Game/fn_8040D640.cpp",
                cflags=cflags_game_cpp,
                # The original unit retains 48 bytes of extab and 24 bytes of extabindex.
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(
                Matching,
                "Game/fn_8040D714.cpp",
                cflags=cflags_game_cpp,
                # The native destructors retain 16 bytes of extab and 24 bytes of extabindex.
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(
                Matching,
                "Game/fn_8040D7C4.cpp",
                cflags=cflags_game_cpp,
                # The native constructors retain 52 bytes of extab and 36 bytes of extabindex.
                extra_cflags=["-Cpp_exceptions on"],
            ),
            # The size and preparation helpers match; the vertex-byte counter retains twelve
            # register-word differences in its normal-attribute case.
            Object(NonMatching, "rw/fn_8046ACBC.c", cflags=cflags_game),
            Object(Matching, "Game/fn_8000AAEC.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80014E04.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8006127C.cpp", cflags=cflags_game_cpp),
            # The material-copy callback retains seven register-word differences.
            Object(NonMatching, "rw/fn_804A076C.c", cflags=cflags_game),
            Object(Matching, "Game/fn_80015954.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80160B18.cpp", cflags=cflags_game_cpp),
            # Part cleanup retains one extra element-address addi and register differences.
            Object(NonMatching, "Game/fn_80162804.cpp", cflags=cflags_game_cpp),
            # The constructor retains inlining differences; UpdateBody lacks one switch branch.
            Object(NonMatching, "Game/fn_800E59F8.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_801A1FA0.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_801A35AC.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_801A4430.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_801A54E8.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_801A61E4.cpp", cflags=cflags_game_cpp),
            # All six function bodies match. The two .sdata2 floats are pooled in the opposite
            # order (the original's first, 0x805F5818, is the one read at unit offsets 0x150 and
            # 0x2D4), and symbols.txt gives lbl_804CC828 a size of 0x10: the original's bytes are
            # the nine-byte initializer of fn_801A6790's resourceIndices[9] (02 to 0A) followed by
            # seven zero alignment bytes, where this unit emits a 9-byte array.
            Object(NonMatching, "Game/fn_801A6588.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_801A687C.cpp", cflags=cflags_game_cpp),
            Object(
                Matching,
                "Game/fn_8040D180.cpp",
                cflags=cflags_game_cpp,
                # The original keeps eight-byte extab and twelve-byte extabindex records.
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "Game/fn_801D455C.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/FootingRoll.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/FootingBreak.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecBarrier.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecBomb.cpp", cflags=cflags_game_cpp),
            # All 49 functions have identical instructions apart from 14 raw differing words, all
            # immediate offsets into .rodata (6 in the preload hook 0x802647B8, 8 in the
            # constructor 0x802655E0), and the other sections match once the linker-discarded weak
            # and local copies are set aside. The original's .rodata has 0x4C bytes this object
            # does not: an unreferenced 0x40-byte table of floats (0, 0, 0, 0 and three rows of 0,
            # 0, 0, 1) in front of the TNumEdit names, as in CircusPole, and 12 zero bytes after
            # the CharaColliBase name. The strings after them sit 0x40 or 0x4C lower here. That
            # moves those 14 offsets, the targets of the code relocations into that part of .rodata
            # (they reach the same data, e.g. OnDraw's pair at .text+0x142/0x14A reaches
            # .rodata+0x230 in the original and .rodata+0x1E4 here) and the .rodata addends of four
            # data relocations (the descriptor name, two RTTI names and s_name).
            Object(NonMatching, "Game/ElecCoreWall.cpp", cflags=cflags_game_cpp),
            # 81 of the 85 functions have identical instructions and relocations once the weak and
            # local copies the linker discards are removed. The data matches by contents in an
            # object-relative comparison that sets those copies and the alignment aside; a
            # source-enabled link places the unit's data differently (in USA its .data, .rodata and
            # .sdata take 0x204, 0x3E4 and 0x9C bytes against 0x1EC, 0x2BC and 0x5C). Search
            # (0x801540F0) differs in 3 words (f2 and f3 swapped for 1 - fraction and 25, and the
            # operand order of one fmuls) and Sweep (0x80153CE8) in 5 words (f1 and f2 swapped for
            # the range and the speed product, and the operand order of two fmuls). UpdateBody is
            # two instructions longer (182 raw words differ by position): twice the original passes
            # the result of a copy constructor straight to fn_8040CF2C, where this build copies
            # into a local and passes its address. UpdateCone differs in 18 words, all stack
            # offsets: the original makes the up vector of the rotation-only transform
            # (fn_8040B618) after its result, as a temporary would be.
            Object(NonMatching, "Game/ElecSearchLight.cpp", cflags=cflags_game_cpp),
            # Compared as relocatable objects, with the weak and local copies the linker discards
            # removed and the local names and the data section alias (@2 against ...data.0)
            # resolved, 36 of the 37 functions have identical instructions and relocations. In a
            # diagnostic link of this object (normal builds link the original object), 17 of the 37
            # functions have identical raw words in each region; fn_80156C78 aside, the others
            # differ only in address operands, as code and data after it are placed at other
            # addresses. fn_80156C78 (0x80156C78) is 0x4A8 bytes
            # against the original's 0x4A4: aligned, 16 of its 297 words differ (274 by position).
            # Its switch on the state compares 1, 0 and 3 (9 words) where the original compares 2
            # and 0 and sends every value above 2 to the code after the switch (8 words, as for a
            # fourth case label leading there; nothing in this file sets a state above 2), and for
            # both codes a split creates, the original calls operator new before the fn_8040CDA8
            # and fn_8040CDD0/fn_8040CD74 calls that build the position (they are part of the
            # argument evaluation), where this build calls it after them (4 words placed after the
            # 8-word block instead of before it). Compared by contents with the absent copies set
            # aside, every data object matches except a 0x54-byte block at the start of the
            # original .data (the RTTI base lists of Player::PlayerCommand and of an unnamed class
            # derived from it, then 13 zero words) and the CharCommand and Player::PlayerCommand
            # RTTI records and names it refers to (.sdata 0x14 bytes, .rodata 0x24 bytes): no code
            # in this file uses commands, and the declarations that emitted them are not
            # identified. The literal "ElecSecurityCode" that the constructor passes to
            # fn_8015CB88 (0x80157518/0x80157520) is in another place in .rodata: the original has
            # it directly after s_targetHeights, before the bind_t type name and "Memory allocation
            # failure", and this build emits it after those two strings, directly before
            # "SECURITY.BIN", so the 0xF8 bytes between s_targetHeights and "SECURITY.BIN" hold the
            # same three strings in another order. Set aside: the TNumEdit subclasses' RTTI records
            # and names, the weak vtables of std::exception, boost::bad_weak_ptr and sp_counted_base
            # with their RTTI records and names, and the placeholders _2 to _9.
            Object(NonMatching, "Game/ElecSecurityCode.cpp", cflags=cflags_game_cpp),
            # Every function except UpdateBody is identical in every word and relocation, and the
            # data matches once the linker-discarded weak copies are set aside. UpdateBody
            # (0x80157F24) is 0x26C bytes against the original's 0x280: the original subtracts the
            # two positions with the out-of-line copy of operator- (0x8000AD64) and calls the
            # smart pointers' operator-> out of line for them (0x800CD638 and 0x80096434), keeping
            # the first result in r30, nine instructions where this build has four (it loads both
            # pointers in place and calls the subtraction itself). Aligned, that is the only
            # difference; the 0x14-byte shift after it makes 125 of the original's 160 words differ
            # by position.
            Object(NonMatching, "Game/ElecSecurityCodeGenerator.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecBlock.cpp", cflags=cflags_game_cpp),
            # 54 of the 55 functions have identical words and relocations, and the data matches
            # once the linker-discarded weak and local copies are set aside, apart from the
            # writable .sdata2 flag and section-end alignment (a USA DOL linked from
            # this source object, a diagnostic only, differs from the original in 9 words, all in
            # the hit callback). The hit callback (0x8015A260, USA) differs in 9 of its 188 words,
            # positionally at the same size and frame (0xA0): the int-to-float conversion of the
            # level index uses its own 8-byte stack slot (0x80) where the original reuses the slot
            # of the float-to-int conversion of the level (0x78), and the level and the difference
            # of the two table entries are held in f3 and f2 instead of f2 and f3, as in the
            # Spring and LongSpring hit callbacks.
            Object(NonMatching, "Game/SpringPanel.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/SecurityLaser.cpp", cflags=cflags_game_cpp),
            # Every function but the constructor has identical instructions and relocations, and
            # the data, symbols and relocations match once the linker-discarded weak and local
            # copies are set aside. The constructor (0x801414E4, USA) differs in 2 of its 230
            # words: for its call of GadgetModelInfo::fn_801629D8 with the crash information's
            # fn_802087D8 name, the original loads m_crashInfo's pointer before m_model's and this
            # build loads them the other way round (the same instructions, registers and
            # relocations otherwise), as in SuperComputer's constructor. Writing the call with get()
            # or operator* on either pointer gives the same order.
            Object(NonMatching, "Game/BreakWall.cpp", cflags=cflags_game_cpp),
            # 35 of the 36 functions have identical instructions and relocations, and the data,
            # symbols and relocations match once the linker-discarded weak and local copies are set
            # aside. The launch function (0x80142C88, USA) differs in 19 of its 168 words (raw, in a
            # USA DOL linked from this source object, a diagnostic only: the build links the
            # original object): the float-to-int conversion of the level and the int-to-float
            # conversion of the index get separate 8-byte stack slots where the original reuses
            # one (frame 0xA0 instead of 0x90, which also moves the register save offsets), and
            # the level and the difference of the two table entries are held in f3 and f2 instead
            # of f2 and f3, as in the Spring, CircusBall, DashPanel and DashRing hit callbacks.
            Object(NonMatching, "Game/Catapult.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/DamageBlock.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Door.cpp", cflags=cflags_game_cpp),
            # Word by word against the split original object (USA, Japan and PAL alike), with the
            # linker-discarded weak and local copies set aside: 54 of the 58 functions have
            # identical words and relocations. The preload hook (0x8014BD6C, USA) and the constructor (0x8014CB98)
            # differ in 3 words each, all immediate offsets into .rodata: the original's .rodata has
            # an unreferenced 0x40-byte table of floats (three rows of 1, 0, 0, 0 and one of zeros)
            # between the TNumEditFloat and CharaColliBaseCharInfo names, and 12 zero bytes after the
            # CharaColliBase name, which this object does not have (as in ElecCoreWall), so the
            # strings after them sit 0x40 or 0x4C lower here, which also moves the .rodata addends of
            # the data relocations to them. UpdateBody (0x8014C108) is 220 words against 221;
            # with the raw words aligned, 5 of the original's and 4 of this build's have no
            # counterpart: the original loads the character info's pointer into r30 before the call
            # of fn_8040B67C whose result it then copies there (one more saved register and one more
            # instruction), which also changes one branch displacement. The hit callback (0x8014D324) differs in 19 of 243 words,
            # the interpolation of the launch speed as in Spring and DashRing: the int-to-float
            # conversion of the index gets its own 8-byte stack slot (frame 0xB0 instead of 0xA0)
            # and the level and the table difference are in f3 and f2 instead of f2 and f3. In
            # .sdata2 the original places 1.0f after 0.0f (0x805F4744), this build after 0.063f
            # (first used in the hit callback), so the 12 literals between them sit one slot
            # earlier. In .data the original keeps the discarded weak vtables between the kept
            # base-class lists as zero-filled holes, as in Spring.
            Object(NonMatching, "Game/StretchGrass.cpp", cflags=cflags_game_cpp),
            # 91 of the 96 functions have identical instruction words and relocations in USA, Japan
            # and PAL. The original's .rodata has two blocks that this object lacks, of unknown
            # source: after the TNumEdit type names, three 16-byte groups that each start with
            # 1.0f followed by 16 zero bytes (the 1.0f groups follow the TNumEdit names in four
            # other files' .rodata too), and 12 zero bytes after the CharaColliBase type name.
            # They move the later strings by 0x4C, so the FireBall constructor differs in the 3
            # words that address its strings from the unit's .rodata base. The MotionSequenceModule
            # destructor (inline: EnemyMotionSequenceModule's destructor at 0x8018EAA4 expands it)
            # is emitted after ~Vine here and between UpdateSequences and SetupSequences in the
            # original, which moves the functions in between by 0x48 bytes. ~Vine differs in 8 raw
            # words: the original sets the loop counter to zero and copies it to the
            # offset and to the stored null; this build loads zero into the offset and the counter
            # separately and copies the offset's zero to the null. In GetLoseDistanceSq (2 words)
            # the original loads the radius before the 10.0f constant. UpdateClumpPosition is 19
            # words against the original's 23 (13 of 24 aligned rows differ: 12 of the original's
            # words and 8 of this build's): the original calls the smart pointer's operator-> out of
            # line for the character info's position, keeping the object in the saved r31; this
            # build loads the pointer inline and saves no register. UpdateModelMatrix differs in
            # 10 words, all stack offsets: the original keeps the target's position, built from the
            # frame's matrix and passed to fn_800CCE38, below the difference vectors of the two
            # branches, as a temporary made last would be.
            Object(NonMatching, "Game/Vine.cpp", cflags=cflags_game_cpp),
            # 104 of the 106 functions have identical instructions and relocations, and the .data,
            # .rodata and .sdata objects match once the linker-discarded weak and local copies are
            # set aside. The bars' hit callback (0x80152164) is one instruction longer: the original
            # passes the result of the span's copy constructor straight to fn_8040CF50, where this
            # build copies into a local and passes its address. ElecBarSub's UpdateModelMatrix
            # differs in 16 words: the original creates the transform's result before the scaled and
            # offset vectors it is computed from, as a nested expression does, but loads the line
            # pointer between them. Two placement differences remain: the tree's __destroy and the
            # allocator helpers it calls follow the tree's destructor here instead of coming first
            # among the tree members at the end of the code, where clear also comes before insert in
            # the original (the map units show the same placement); and the int-to-float constant of
            # .sdata2 follows ElecBarSub::Update's literals instead of preceding
            # ElecBarSub::UpdateModelMatrix's.
            Object(NonMatching, "Game/ElecBar.cpp", cflags=cflags_game_cpp),
            # Every function but the constructor has identical instruction words, and the data,
            # symbols and relocations match once the linker-discarded weak and local copies are set
            # aside. The constructor (0x8022E698, USA) differs in 6 of its 189 words: for each of its
            # three calls of a GadgetModelInfo method with a CrashInfoCtrl name as the argument, the
            # original loads m_crashInfo's pointer before m_model's, this build loads them the other
            # way round (the same instructions, registers and relocations otherwise).
            Object(NonMatching, "Game/SuperComputer.cpp", cflags=cflags_game_cpp),
            # Compared as relocatable objects, all 54 functions have identical instructions and
            # relocations once one local alias is resolved: the constructor's two relocations of the
            # 0xC-byte local object at .data+0 name it @2 in the original and the section symbol
            # ...data.0 here. The .data, .rodata, .sdata and .sbss objects and symbols match once
            # the linker-discarded weak and local copies are set aside. Linked into USA, Japan and
            # PAL executables (a diagnostic build only; the normal build links the original object),
            # 51 of the 54 functions have identical raw words: the original's .sdata2 has the
            # int-to-float conversion constant (0x805F6540, USA) before UpdateIdle's literals 1/3,
            # "mat1" and 0.25, where this build places it after them (0x18 bytes of .sdata2 in
            # another order), so the literals' addresses change 6 of the 126 words of UpdateIdle
            # (0x801F4C68, USA) and the word that addresses "mat1" in UpdateBreakColors (0x801F4FE4)
            # and in the constructor (0x801F5D8C). The compiler makes that constant after a
            # function's other literals, and an out-of-line function with an int-to-float conversion
            # compiled between ResetFromOwner and UpdateIdle reproduces the original order: the
            # original most likely compiled such a function there that the executable does not have
            # (its body is unknown), as in ElecBar.
            Object(NonMatching, "Game/ElecCoreProgram.cpp", cflags=cflags_game_cpp),
            # 3 of the 4 functions have identical instructions and relocations in USA, Japan and
            # PAL, and .rodata matches. The original sets the floats at 0x10 and 0x14 of both
            # descriptions (1.0 and 5.0, from two .sdata2 literals) in a start-up function
            # (0x801F65B0, USA) registered in .ctors and has zeros there in .data; this build
            # initialises them in .data, so it has no start-up function, .ctors slot or .sdata2
            # data. The two floats are probably one Fn8040Vector2 member (inferred), but MWCC
            # rejects an aggregate initialiser with a class-typed member, and a constructor for the
            # whole description initialises every member at start-up, where the original sets only
            # these two there. Also tried: literal floats and static const float objects are folded
            # into the static data; elements of a static const float array are initialised at
            # start-up but loaded through one base register. Const floats declared before and
            # defined after the descriptions, or inline functions that return the two values,
            # reproduce the start-up function's loads but have no support in the bytes beyond
            # forcing that initialisation, so none is used.
            Object(NonMatching, "Game/fn_801F6488.cpp", cflags=cflags_game_cpp),
            # 37 of the 38 functions have identical instructions and relocations in USA, Japan and
            # PAL. The constructor is 186 words against the original's 188: the original converts
            # the lock() result with a bool conversion (addic/subfe on the copy constructor's
            # result, then clrlwi. after the temporary is destroyed), where this source compares
            # attack.get() after the temporary is destroyed (lwz/cmplwi); this tree's shared_ptr
            # has no bool conversion. The later words move by 8 bytes, so 113 words of the
            # function differ in the object comparison. With a bool conversion added to shared_ptr
            # and used as the if condition the constructor matches, and that build passes the
            # whole-object comparison once the linker-discarded weak and local copies are removed.
            Object(NonMatching, "Game/ElecFireWall.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecCristalWall.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecKyuu.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/MagmaGuard.cpp", cflags=cflags_game_cpp),
            # The data matches once the linker-discarded weak and local copies are set aside, and
            # 33 of the 34 functions have the original's words. The hit callback (0x8010F028)
            # differs in 22 words: the frame is 0xE0 instead of 0xD0 because the two int-to-float
            # conversions of the level index get their own 8-byte stack slots (the original reuses
            # the float-to-int slot), and the level and the difference of the two table entries
            # are in f3 and f2 instead of f2 and f3.
            Object(NonMatching, "Game/JumpBoard.cpp", cflags=cflags_game_cpp),
            # The data matches once the linker-discarded weak and local copies are set aside, and
            # 33 of the 34 functions have the original's words (the function-local statics of
            # UpdateModelMatrix and ResetFromOwner have other compiler numbers than the names in
            # symbols.txt). The hit callback (0x80110320) differs in 9 words: the int-to-float
            # conversion of the level index gets its own stack slot and the level and the table
            # difference are in f3 and f2 instead of f2 and f3.
            Object(NonMatching, "Game/LongSpring.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BAGunShip.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BAMine.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BAShield.cpp", cflags=cflags_game_cpp),
            # 62 of 66 functions are identical in every word and relocation. The three record
            # lookups fn_80301D04, fn_80301D90 and fn_80301E1C each differ in one word, the id
            # comparison: the original has cmpw r0,r3 (0x7C001800 at 0x80301D1C, 0x80301DA8 and
            # 0x80301E34) where the direct comparison of the record's id with the parameter gives
            # cmpw r3,r0. UpdateBody (0x80302CC4) is 78 words instead of 79: after the state's
            # comparison with 0 at 0x80302DC4, the original branches twice to 0x80302DDC (blt at
            # 0x80302DCC, b at 0x80302DD0), the tree of a switch with further empty case labels
            # whose values are unknown; with the single case 0 the blt is missing and the beq at
            # 0x80302DC8 branches 8 bytes ahead instead of 0xC. Compared position by position, 13
            # of the 78 words differ and the original's last word has no counterpart (the beq, then
            # every word after the missing blt is shifted by one). The data sections match by
            # contents once alignment is accounted for and the weak and local copies that the
            # original unit does not have are set aside: as emitted, .data is 0xC0 bytes (0x80 in
            # the original; it adds three weak vtables and a 12-byte local record) and .rodata
            # 0x213 (0x1C8), and .sdata2 is writable where the original's is read-only. Normal
            # builds link the original object.
            Object(NonMatching, "Game/BASwitch.cpp", cflags=cflags_game_cpp),
            # UpdateBody loads the startup float after the store to the offset (6 of 91 words).
            Object(NonMatching, "Game/CloseWall.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/DefenseWall.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecCircuitColli.cpp", cflags=cflags_game_cpp),
            # Constructor temporary cleanup, rotation registers, and literal order still differ.
            # Constructor temporary cleanup and float literal pool order still differ.
            Object(NonMatching, "Game/ElecSmallCube.cpp", cflags=cflags_game_cpp),
            # Every function but UpdateBody and the range drawing (fn_800CBC1C) is identical in
            # every word and relocation. UpdateBody (0x80266BAC) differs in 6 of its 174 words: two
            # loads take f0 and f1 the other way round, which changes the registers and operand
            # order of the three floating-point multiplications that use them.
            # fn_800CBC1C differs in 10 words, all stack offsets: the original's centre is made
            # before the offset and rotation vectors it is computed from, as temporaries of a
            # nested call would be; the debug shape functions take pointers here.
            Object(NonMatching, "Game/ElecFan.cpp", cflags=cflags_game_cpp),
            # 34 of 35 functions and all data match once the weak copies the linker discards are
            # removed. __sinit_Meteor_cpp differs in 2 words: the original loads the saved link
            # register before the store of the startup float.
            Object(NonMatching, "Game/Meteor.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Meteors.cpp", cflags=cflags_game_cpp),
            # Constructor temporary cleanup, matrix calls, and literal/weak metadata layout differ.
            Object(NonMatching, "Game/ElecRollHexa.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/DefenseShield.cpp", cflags=cflags_game_cpp),
            # 45 of the 73 functions have identical instructions and relocations once the weak and
            # local copies the linker discards are removed and local symbols are paired by the data
            # they refer to: the names differ (OnAttack loads the .sdata2 float that the original
            # calls lbl_805F6E30 and this object @503) and so can the raw offsets, since this object
            # adds data (ChangeState's lbl_8054631C at .data 0x0C is @339 at 0x3C here). The
            # constructor (0x80224558) differs in 44 words, register numbers only: the original
            # keeps this in r31, the owner in r28, the loop counter in r29 and 0x10000 in r30, where
            # this build uses r28, r29, r30 and r31.
            # fn_8022440C differs in 3 words, the offsets of the two break-model records and the
            # model file name from the unit's .rodata base: the original's .rodata has two blocks of
            # zero bytes, 0x1C after the TNumEdit type names and 0xC after the CharaColliBase name,
            # that this object does not have. The other 26 belong to the std::vector of
            # boost::shared_ptr<Target> instance: insert, swap and reserve differ in size and
            # structure (the original calls small helpers that this build does not emit, among
            # them one returning the capacity and one returning 0x1FFFFFFF, where this build calls
            # __alloc, __cap and its own __vector_obj constructor and swap), the vector destructor
            # is 0x4C bytes here against the original's 0x50, the vector constructor, the
            # __vector_obj destructor and clear call helpers this build does not emit, and 19 small
            # helper functions (constructor chain, accessors, pair swap) have no counterpart here.
            # Compared by contents with the absent copies set aside, the data matches except for
            # the two zero blocks in .rodata; this object also has RTTI records and base lists that
            # the original unit does not have (TNumEdit, CharaColliBase), weak vtables
            # (SetAdapterState, std::exception, boost::bad_weak_ptr, sp_counted_base) with their
            # RTTI records, and the boost placeholders _2 to _9.
            Object(NonMatching, "Game/DefenseSystem.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ArkCannon.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/EggMeca511.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/EggMeca411.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/EggMeca310.cpp", cflags=cflags_game_cpp),
            # Every function except UpdateBody, the retained data and all retained relocations
            # match once the linker-discarded definitions (43 weak copies and nine local ones,
            # among them a converting constructor, RTTI records and literals) are set aside. Normal
            # builds link the original object; the source object was linked only as a separate
            # diagnostic. UpdateBody differs in 7 words, all
            # register colouring: the landing test's two float loads, its compare and its store
            # use f0 and f1 the other way round, and the bounce update's subtraction, 0.8 literal
            # load and multiply-add use f1 and f2 the other way round. Opcodes, relocations and
            # load/store counts are identical.
            Object(NonMatching, "Game/FallRoad.cpp", cflags=cflags_game_cpp),
            # All 60 functions have identical instructions and the data bytes match, but the 0.0f
            # and 120.0f literals are pooled in the opposite order, and the vtable relocations
            # name functions of other units whose symbols still have address names. 104 names
            # stay unresolved: 101 function references from the vtables (methods of
            # EnemyStatusCommon, EnemyDispCommon, EnemyBaseAI, TEnemySetBase and others) and three
            # vtables that instructions reference (__vt__11EnemyBaseAI, __vt__13BkWingSmallAI and
            # __vt__13FlyerCommonAI), so the unit does not link.
            Object(NonMatching, "Game/BkWingSmall.cpp", cflags=cflags_game_cpp),
            # First part of a larger original object. The following EggFleet/EggCannon code,
            # including ~EggFleet, is not reconstructed, so checked_delete<EggFleet> is inlined
            # here and the shared metadata remains in the original data.
            Object(NonMatching, "Game/FleetHolder.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/PowerDeviceNaked.cpp", cflags=cflags_game_cpp),
            # 94 of the original's 95 functions have identical instruction words and relocation
            # types; their calls to std::vector<u32>'s members go to the named copies at
            # 0x800CC35C-0x800CE48C (USA), and insert holds the new node in a node holder whose 13
            # functions (0x800D72B8-0x800D7510) the original has too. The std::list constructor is
            # 2 words short (the original calls the size's allocator accessor before building the
            # node allocator; see the note in <list>). By contents, with the linker-discarded weak
            # and local copies set aside, all of the original's .rodata, .data (relocation places
            # and other bytes; the original keeps zeros where discarded weak vtables were), .sdata
            # and .sdata2 contents are present, and insert addresses a 1-byte .sbss object as the
            # original's does (0x805EE7D4). The object also defines the boost placeholders _2 to
            # _9, which a source-enabled link leaves unused, and, when such a link selects this
            # object's std::vector<u32> members, adds the vector length-error strings they
            # reference.
            Object(NonMatching, "Game/RollCircle.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/FireRing.cpp", cflags=cflags_game_cpp),
            # 48 of the 49 functions have identical instructions and relocations. With the
            # linker-discarded weak and local copies set aside, the whole object differs only in 2
            # words of the constructor: the order of the two argument moves of the vector assignment
            # that copies the path's start position.
            Object(NonMatching, "Game/Monster.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/MonsterControl.cpp", cflags=cflags_game_cpp),
            # 58 of the 59 functions have identical instruction words and relocation types and are
            # in the original's order. The GongCSD constructor differs in 3 words, the offsets of
            # three strings from the unit's .rodata base: the original's .rodata has two blocks of
            # zero bytes, 0x1C after the TNumEdit type names and 0xC after the CharaColliBase name,
            # that this object does not have, which moves "PR0302RINGANM" and "Scene_0000" by 0x28;
            # "csdBase.h" is also 0x28 earlier: the shared reference destructor emits the
            # original's 0x20-byte deallocation error string. The original's .rodata also has
            # strings that this object lacks: "csdRCObject.h" and the type names of the RCPtr holders
            # of unsigned char, CTexList and CScene, their
            # CFunctorBase, CBase and CResourceBase<Chao::CSD::Project> (csdRCObject.h declares
            # those template members without bodies). This object has type records, base lists and
            # weak and local copies that the original unit does not have (TNumEdit, CharaColliBase,
            # std::exception, boost::bad_weak_ptr, sp_counted_base, the boost placeholders _2 to
            # _9). It also emits weak copies of the 0x18C RCPtr<CScene> destructor and
            # CMemoryAlloc::FreeObject.
            Object(NonMatching, "Game/Gong.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ShootingGameBalloon.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ShootingGameManagerTask.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BombingWall.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BombServer.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Researcher.cpp", cflags=cflags_game_cpp),
            # 215 of the 219 functions have identical instructions and relocations once the weak and
            # local copies that the linker discards are removed. The static initialiser differs in 5
            # words: two registers swapped and the three roll velocities addressed from the start of
            # .bss here, 0x3C bytes after it in the original (see below). The constructor of the
            # stone list's implementation differs in 14 words (22 against 20): the original also
            # calls __alloc() to build the node allocator (see the note in <list>).
            # StonePole::CreateStones (5 words) and StonePole::Update (3 words) load a stone kind
            # from the step table differently: the original adds the step offset to the holder's
            # address and loads at 0x50 from it, this source adds 0x50 to the offset and uses an
            # indexed load (in CreateStones the argument set-up around it is also ordered
            # differently). 27 raw differing words. The data match by contents (the original
            # object's absent copies set aside) with two differences: .sdata2 holds 0.0001 before 1
            # and 70 in the original and after them here (3 words in a different order; the cause is
            # not established), and the original's .bss starts with 0x3C bytes that no code
            # references, relative to which its static initialiser addresses the three vectors
            # (their type is unknown).
            Object(NonMatching, "Game/PoleHolder.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/StoneStatue.cpp", cflags=cflags_game_cpp),
            # All 56 functions have identical instructions, and the data matches by contents once
            # the copies that the original object lacks (weak copies the linker discards and the
            # exception classes' local RTTI records) are set aside, except .sbss, where the
            # original has fn_800DDF20's guard before its pointer and this object the reverse.
            # fn_800DDF20 comes after the converting shared_ptr constructor instead of before it
            # (the original's copy looks like the out-of-line copy of an inline function).
            Object(NonMatching, "Game/Tornado.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/CityChaosEmerald.cpp", cflags=cflags_game_cpp),
            # Every function but the static initialiser is identical, and the data matches once
            # the weak copies the linker discards are removed. The static initialiser differs in 2
            # words: the original loads the saved link register before it stores the startup float
            # (0x80231AF4), this build after it.
            Object(NonMatching, "Game/MissionClearCollision.cpp", cflags=cflags_game_cpp),
            # The data matches once the weak copies the linker discards are removed, apart from
            # .bss, where the original keeps 12 bytes that no code reads or writes before the four
            # line segments. Differences (raw words): the static initialiser 6 (the segments'
            # offsets from that .bss start, and the order of two startup-float loads) and
            # DrawDebugMarker 10 (the original loads the startup radius before it saves the
            # registers).
            Object(NonMatching, "Game/DebugMissionClearCollision.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/RailSwitch.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/DarkSpinEntrance.cpp", cflags=cflags_game_cpp),
            # Constructor (22 words) and UpdateBody (6 words): the two startup floats load later
            # than in the original (const floats with a dynamic initialiser reproduce both loads).
            Object(NonMatching, "Game/SuctionPillar.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/SecretDisk.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/GiantLantern.cpp", cflags=cflags_game_cpp),
            # 35 of 36 functions have identical words and relocations, and the data matches once
            # the linker-discarded weak and local copies are set aside. The constructor is 185
            # words in the original and 184 here: in the branch for a stored state of 3, the
            # original loads the com id into r30 before the fn_800CB69C call and zero-extends it
            # with clrlwi for fn_801690D0, where this build loads it into r4 after the call (a
            # local copy of m_comId moves the load but leaves mr for the clrlwi).
            Object(NonMatching, "Game/ElecAccessPanel.cpp", cflags=cflags_game_cpp),
            # 35 of 38 functions have identical words and relocations, and the data matches once the
            # linker-discarded weak and local copies are set aside. Differences (raw words in a USA
            # DOL linked from this source object, a diagnostic only: the build links the original
            # object): the hit callback (29 of its 304 words differ as a relocatable object), its frame 0x170 instead of 0x160 (the original
            # keeps the float-to-int conversion of the level and both int-to-float conversions of
            # the index in one 8-byte slot, this build gives each its own), the first interpolation
            # holding the level in f3 and loading the next table entry into f0 where the original
            # uses f2 and f3, the panel-space offset built in place where the original calls the
            # out-of-line three-float constructor (0x80008E1C) for it, vectors at other stack
            # offsets, and three calls whose targets swapped places; the CancelBehaviorCommand
            # constructor defined in the file (see its note) follows the converting
            # weak_ptr<CharInfo> constructor instead of preceding it (12 and 13 words).
            Object(NonMatching, "Game/DashPanel.cpp", cflags=cflags_game_cpp),
            # 33 of 34 functions have identical words and relocations, and the data matches once
            # the linker-discarded weak and local copies are set aside. Differences (raw words in
            # a USA DOL linked from this source object, a diagnostic only: the build links the
            # original object): the hit callback 9 of 220 (the original keeps the float-to-int
            # conversion of the level and the int-to-float conversion of the index in one 8-byte
            # slot, this build gives each its own, and the interpolation holds the level in f3 and
            # loads the next table entry into f0 where the original uses f2 and f3).
            Object(NonMatching, "Game/DashRing.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/EnergyCore.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Fan.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Fire.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Fruit.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/GoalRing.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/SetVehicle.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/SetWeapon.cpp", cflags=cflags_game_cpp),
            # 52 of 53 functions are identical in every word and relocation. The hit callback
            # (0x801213E8) differs in 9 of its 276 words: the int-to-float conversion of the index
            # uses a second 8-byte stack slot where the original reuses the slot of the float-to-int
            # conversion of the level, and the level and the difference of the two table entries are
            # held in f3 and f2 instead of f2 and f3. The weak vtables of bad_weak_ptr,
            # sp_counted_base_impl for CharCommandUnit, std::exception, std::bad_alloc and
            # sp_counted_base lie between kept base-class lists in .data; the original keeps their
            # 0x60 bytes as zero-filled holes. Linked in place of the original object, this object's
            # sp_counted_base_impl vtable, functions and type record for CharCommandUnit and its
            # checked_deleter type record and name are kept, so .rodata is 0x2BC and .sdata 0x5C
            # against the original's 0x238 and 0x50; the other data match when those copies are set
            # aside.
            Object(NonMatching, "Game/Spring.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Switch.cpp", cflags=cflags_game_cpp),
            # fn_8040C55C's instructions match. fn_8040C3EC (92 words) differs in 15 positional raw
            # words (19 when relocation targets are compared by section and offset: the shared-pool
            # loads and the threshold load at +0x90, which refers to .sdata2+0 in the original and
            # .sdata2+0x18 here): the original loads the dot product's first terms before it copies
            # from and to into saved registers, and a load and a multiply of the final weighted sum
            # are exchanged. Returning the quaternion by value gives the original order of the
            # latter, but the Matching callers in fn_8040EFA4.cpp then differ whether they copy the
            # result or bind a reference to it (the copy interface is not yet settled). The
            # functions load 1.0f and 0.0f (fn_8040C55C also 0.5f) from the original's shared
            # .sdata2 pool, which this unit does not own (4 and 7 load words), and fn_8040C55C's
            # index table is 12 bytes against the original's 16-byte .rodata extent (its last word
            # is zero; likely alignment padding).
            Object(
                NonMatching,
                "Game/fn_8040C3EC.cpp",
                cflags=cflags_game_cpp,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "Game/ShootingGameScore.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8005A300.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80054DD0.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80096B64.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_800532F8.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80422070.cpp", cflags=cflags_effect),
            Object(Matching, "Game/fn_804220CC.cpp", cflags=cflags_effect),
            Object(Matching, "Game/fn_8042A2D8.cpp", cflags=cflags_effect),
            Object(Matching, "Game/EffectGlobalParam.cpp", cflags=cflags_effect),
            Object(Matching, "Game/EffectLensFlare.cpp", cflags=cflags_effect),
            Object(Matching, "Game/EffectShimmer.cpp", cflags=cflags_effect),
            # Update retains six register-word differences; the other seven bodies have
            # identical instruction words.
            Object(NonMatching, "Game/EffectLocus.cpp", cflags=cflags_effect),
            # The vertex builder retains independent-literal addressing, stack and
            # instruction-layout differences (2492 versus 2568 bytes).
            Object(NonMatching, "Game/fn_80430A54.cpp", cflags=cflags_effect_rw3),
            Object(Matching, "Game/fn_804340CC.cpp", cflags=cflags_effect_rw3),
            # Render and the Data48 vertex builder retain branch/instruction-layout and
            # register differences; the Shimmer constructor differs by one instruction and
            # fn_80436128 by registers.
            Object(NonMatching, "Game/fn_804342CC.cpp", cflags=cflags_effect_rw3),
            # Matching workaround: these two units address literals independently in the
            # original. Keep unpooled addressing until their original build options are known.
            Object(NonMatching, "Game/fn_80437118.cpp", cflags=cflags_effect),
            Object(NonMatching, "Game/fn_80437C40.cpp", cflags=cflags_effect),
            Object(Matching, "Game/EffectGlobalParamRW3.cpp", cflags=cflags_effect_rw3),
            # Four functions match; the vertex builder is 2556 versus 2736 bytes, with
            # vertex-copy, stack and instruction-layout differences (the original also copies the
            # point's position into a local component by component, where this build calls the
            # vector's copy constructor).
            Object(NonMatching, "Game/EffectLocusRW3.cpp", cflags=cflags_effect_rw3),
        ],
    },
    DolphinLib(
        "os",
        [
            Object(Matching, "dolphin/OSAlarm.c"),
            Object(Matching, "dolphin/OSCache.c"),
            Object(Matching, "dolphin/OSError.c"),
            Object(Matching, "dolphin/OSContext.c"),
            Object(Matching, "dolphin/OSArena.c"),
            Object(Matching, "dolphin/OSAudioSystem.c"),
            Object(Matching, "dolphin/OSSync.c"),
            Object(Matching, "dolphin/OSInterrupt.c"),
            Object(Matching, "dolphin/OSMemory.c"),
            Object(Matching, "dolphin/OSReboot.c"),
            Object(Matching, "dolphin/OSResetSW.c"),
            Object(Matching, "dolphin/OSRtc.c"),
            Object(Matching, "dolphin/OSReset.c"),
            Object(Matching, "dolphin/db.c"),
            Object(Matching, "dolphin/OS.c"),
            Object(Matching, "dolphin/OSAlloc.c"),
            Object(Matching, "dolphin/OSExec.c"),
            # Matching workaround: -char signed. OSFatal sign-extends the characters of its message
            # (extsb); the SDK default -char unsigned makes the same source load them with lbz. Revisit
            # when the library's real compiler flags are known.
            Object(Matching, "dolphin/OSFatal.c", extra_cflags=["-char signed"]),
            Object(Matching, "dolphin/OSFont.c"),
            Object(Matching, "dolphin/OSLink.c"),
            Object(Matching, "dolphin/OSMutex.c"),
            Object(Matching, "dolphin/OSThread.c"),
            Object(Matching, "dolphin/OSTime.c"),
            Object(Matching, "dolphin/__start.c"),
            Object(Matching, "dolphin/__ppc_eabi_init.c"),
        ],
        cflags=cflags_sdk_rev2,
    ),
    DolphinLib(
        "card",
        [
            Object(Matching, "dolphin/CARDBios.c"),
            Object(Matching, "dolphin/CARDBlock.c"),
            Object(Matching, "dolphin/CARDDir.c"),
            Object(Matching, "dolphin/CARDRdwr.c"),
            Object(Matching, "dolphin/CARDCheck.c"),
            Object(Matching, "dolphin/CARDMount.c"),
            Object(Matching, "dolphin/CARDFormat.c"),
            # Matching workaround: -char signed for this unit only (the rest of the library is
            # unsigned char). With unsigned char, __CARDCompareFileName and __CARDGetFileNo lose
            # the extsb of the char comparisons. Revisit if the original per-file flags are found.
            Object(Matching, "dolphin/CARDOpen.c", extra_cflags=["-char signed"]),
            Object(Matching, "dolphin/CARDCreate.c"),
            Object(Matching, "dolphin/CARDRead.c"),
            Object(Matching, "dolphin/CARDWrite.c"),
            Object(Matching, "dolphin/CARDDelete.c"),
            Object(Matching, "dolphin/CARDStat.c"),
            Object(Matching, "dolphin/CARDUnlock.c"),
            Object(Matching, "dolphin/CARDNet.c"),
        ],
    ),
    DolphinLib(
        "ax",
        [
            Object(Matching, "dolphin/AX.c"),
            Object(Matching, "dolphin/AXAlloc.c"),
            Object(Matching, "dolphin/AXAux.c"),
            Object(Matching, "dolphin/AXCL.c"),
            Object(Matching, "dolphin/AXOut.c"),
            Object(Matching, "dolphin/AXSPB.c"),
            Object(Matching, "dolphin/AXVPB.c"),
            Object(Matching, "dolphin/AXProf.c"),
            Object(Matching, "dolphin/AXComp.c"),
            Object(Matching, "dolphin/DSPCode.c"),
        ],
    ),
    DolphinLib(
        "axfx",
        [
            Object(Matching, "dolphin/reverb_hi.c"),
            Object(Matching, "dolphin/reverb_std.c"),
            Object(Matching, "dolphin/chorus.c"),
            Object(Matching, "dolphin/delay.c"),
            Object(Matching, "dolphin/axfx.c"),
        ],
    ),
    DolphinLib(
        "mix",
        [
            Object(Matching, "dolphin/mix.c"),
        ],
    ),
    DolphinLib(
        "dsp",
        [
            Object(Matching, "dolphin/dsp.c"),
            Object(Matching, "dolphin/dsp_debug.c"),
            Object(Matching, "dolphin/dsp_task.c"),
        ],
    ),
    DolphinLib(
        "si",
        [
            Object(Matching, "dolphin/SIBios.c"),
            Object(Matching, "dolphin/SISamplingRate.c"),
        ],
    ),
    DolphinLib(
        "dvd",
        [
            Object(Matching, "dolphin/dvderror.c"),
            Object(Matching, "dolphin/dvdlow.c"),
            # Matching workaround: -char signed on dvdfs.c, dvdidutils.c and fstload.c. They sign-extend
            # char values (extsb); the SDK default -char unsigned gives the unsigned load instead.
            # Revisit when the library's real compiler flags are known.
            Object(Matching, "dolphin/dvdfs.c", extra_cflags=["-char signed"]),
            Object(Matching, "dolphin/dvd.c"),
            Object(Matching, "dolphin/dvdqueue.c"),
            # Matching workaround: -char signed, as above.
            Object(Matching, "dolphin/dvdidutils.c", extra_cflags=["-char signed"]),
            Object(Matching, "dolphin/dvdFatal.c"),
            # Matching workaround: -char signed, as above.
            Object(Matching, "dolphin/fstload.c", extra_cflags=["-char signed"]),
        ],
    ),
    DolphinLib(
        "base",
        [
            Object(Matching, "dolphin/PPCArch.c"),
        ],
    ),
    DolphinLib(
        "mtx",
        [
            Object(Matching, "dolphin/mtx.c"),
            Object(Matching, "dolphin/mtx44.c"),
        ],
    ),
    DolphinLib(
        "exi",
        [
            # Matching workaround: -schedule off for EXIBios only. With scheduling on, about 2.2k
            # of its disassembly lines differ. Revisit when the original library build flags
            # are known.
            Object(Matching, "dolphin/EXIBios.c", extra_cflags=["-schedule off"]),
            Object(Matching, "dolphin/EXIUart.c"),
        ],
    ),
    DolphinLib(
        "amcstubs",
        [
            Object(Matching, "dolphin/AmcExi2Stubs.c"),
        ],
    ),
    DolphinLib(
        "odenotstub",
        [
            Object(Matching, "dolphin/odenotstub.c"),
        ],
    ),
    DolphinLib(
        "OdemuExi2",
        [
            # Matching workaround: GC/1.2.5 with -inline deferred,noauto for this unit only. This
            # older compiler version schedules epilogues as the original does, and deferred
            # inlining emits the functions in reverse source order with no auto-inlining, as in the
            # original object. Revisit when the original OdemuExi2 library build flags are known.
            Object(
                Matching,
                "dolphin/DebuggerDriver.c",
                mw_version="GC/1.2.5",
                extra_cflags=["-inline deferred,noauto"],
            ),
        ],
    ),
    {
        "lib": "PowerPC_EABI_Support",
        "mw_version": config.linker_version,
        "cflags": cflags_eabi,
        "objects": [
            Object(Matching, "PowerPC_EABI_Support/Runtime/Src/CPlusLibPPC.cp"),
            Object(Matching, "PowerPC_EABI_Support/Runtime/Src/ptmf.c"),
            Object(
                Matching,
                "PowerPC_EABI_Support/Runtime/Src/NMWException.cp",
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_acos.c"),
            Object(
                Matching,
                "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/w_acos.c",
            ),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_asin.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_atan2.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_fmod.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_log10.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_pow.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_rem_pio2.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_cos.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_rem_pio2.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/k_sin.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_atan.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ceil.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_copysign.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_cos.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_floor.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_frexp.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_ldexp.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_modf.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_sin.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/s_tan.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_sqrt.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/w_sqrt.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/ansi_files.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Src/ansi_fp.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/arith.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/bsearch.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/buffer_io.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/char_io.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Src/critical_regions.gamecube.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/ctype.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/extras.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/file_io.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/FILE_POS.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/locale.c"),
            # Matching workaround: -inline noauto. At the library's default setting
            # is_utf8_complete is also inlined into mbtowc (0x1C4 bytes instead of 0x120); the
            # original keeps it out of line. Revisit when the original unit's flags are known.
            Object(
                Matching,
                "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/mbstring.c",
                extra_cflags=["-inline noauto"],
            ),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/mem.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/mem_funcs.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/direct_io.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/errno.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/printf.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/qsort.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/scanf.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/signal.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/string.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/float.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/strtold.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/strtoul.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/wchar_io.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Src/uart_console_io_gcn.c"),
            Object(Matching, "PowerPC_EABI_Support/Runtime/Src/__va_arg.c"),
        ],
    },
    DolphinLib(
        "vi",
        [
            Object(Matching, "dolphin/vi.c"),
        ],
    ),
    DolphinLib(
        "pad",
        [
            Object(Matching, "dolphin/Pad.c"),
        ],
    ),
    DolphinLib(
        "ai",
        [
            Object(Matching, "dolphin/ai.c"),
        ],
    ),
    DolphinLib(
        "ar",
        [
            Object(Matching, "dolphin/ar.c"),
            Object(Matching, "dolphin/arq.c"),
        ],
    ),
    DolphinLib(
        "gx",
        [
            Object(Matching, "dolphin/GXInit.c"),
            Object(Matching, "dolphin/GXFifo.c"),
            Object(Matching, "dolphin/GXAttr.c"),
            Object(Matching, "dolphin/GXMisc.c"),
            Object(Matching, "dolphin/GXGeometry.c"),
            Object(Matching, "dolphin/GXFrameBuf.c"),
            Object(Matching, "dolphin/GXLight.c"),
            Object(Matching, "dolphin/GXTexture.c"),
            Object(Matching, "dolphin/GXBump.c"),
            Object(Matching, "dolphin/GXTev.c"),
            Object(Matching, "dolphin/GXPixel.c"),
            Object(Matching, "dolphin/GXDisplayList.c"),
            Object(Matching, "dolphin/GXTransform.c"),
            Object(Matching, "dolphin/GXPerf.c"),
        ],
        cflags=cflags_sdk_rev2,
    ),
    {
        # CRI middleware units that are identical and split in every region.
        "lib": "CRI",
        "mw_version": config.linker_version,
        "cflags": cflags_game,
        "objects": [
            Object(
                Matching,
                "CRI/cftcoladj.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/cft_lib.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/cmptime.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/memcpyd.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/memsetd.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mps_dec.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mps_del.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mps_get.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mps_lib.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_deli.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_emp.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_err.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_frm.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_get.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_bdec.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_cdec.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_dec.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: one word in mpvhdec_DecSeqUdsc, the operand order of the position add
            # (original `add r26,r24,r25`, buf + i; this source `add r26,r25,r24`).
            Object(
                NonMatching,
                "CRI/mpv_hdec.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_lib.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: MPVMC08_OneRefH2_TuneC, 251 words (0x35C bytes against 0x3EC). The
            # original loads the masks once at entry, needs no stack frame, packs with 24 rlwimi
            # and loads case 3's first word byte-reversed (lwbrx); this C has masks per case,
            # saves r30/r31 and uses 16 rlwimi and no lwbrx. The original unrolls two rows per
            # iteration in all four cases, this C in two of them.
            Object(
                NonMatching,
                "CRI/mpv_mc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Matching workaround: -inline auto,deferred. The table generators are emitted in reverse
            # source order. Revisit if the original unit's compiler flags are found.
            Object(
                Matching,
                "CRI/mpv_vlc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            Object(
                Matching,
                "CRI/mpvabdec.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/muldiv.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/uty_ppc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/muldivr.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/uty_tmr.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/rna_res.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: sfxcnv_CropFrm, 28 of 42 words: the same instructions and relocations in a
            # different order and with different registers (the original loads the plane height
            # before the buffer); the other 8 functions match.
            Object(
                NonMatching,
                "CRI/sfx_set.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: sfxcnv_ChkCompoByCbFunc, 180 of 1065 words, registers only (the same
            # instructions, order and relocations): sfx and dst are in r29 and r31 instead of r31
            # and r29, the inlined field-split and table-builder flags take other registers, and
            # two table-type loads swap r0 and r3 with their flags. The other 10 functions and
            # .rodata match.
            Object(
                NonMatching,
                "CRI/sfx_cnv.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfx_inf.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfx_lib.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: SFXA_Create, 12 of 36 words: the same instructions and relocation
            # targets in another order: the high half of sfxa_work's address (lis) is formed
            # before the first store instead of after the second, and 255 is loaded before the
            # store of 100 instead of after it, so the constants take other registers; the other 7
            # functions and the .bss layout match.
            Object(
                NonMatching,
                "CRI/sfx_alp.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfx_zmv.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfx_sud.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sud_lib.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Matching workaround: MSL's <math.h> in place of include/libc. MSL's inline sqrtf has
            # static double constants: they are the first, weak .rodata objects, and with them MWCC
            # loads DCT_AcInit's double literals each with its own lis/lfd pair instead of through
            # one pooled .rodata base. include/libc's sqrtf has no static constants (with it
            # DCT_AcInit differs in 59 of 64 words). Revisit when include/libc/math.h's sqrtf gets
            # the static constants for every unit that includes it.
            Object(
                Matching,
                "CRI/dct_ac.c",
                cflags=[*cflags_base, *msl_includes],
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/dct_ver.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/dct_fsri.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/rna_err.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_pl2.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Matching workaround: -inline auto,deferred. fn_803FE96C expands fn_803FEA5C, which is
            # emitted after it; without the flag the functions come out in source order (fn_803FEA5C
            # at 0x0, the original has SFTMR_AddTsum there). Revisit when the library's real
            # compiler flags are known.
            Object(
                Matching,
                "CRI/sfd_tmr.c",
                extra_cflags=[
                    "-inline auto,deferred",
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_bahx.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_baif.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_amp.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_bau.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # ADXB_ExecOneAdx differs in 4 words: the sample offset of the Pro Logic II loop is in r29
            # where the original has r24 (same instructions otherwise).
            Object(
                NonMatching,
                "CRI/adx_bsc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_bsps.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_bwav.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Matching workaround: -inline auto,deferred. MWCC then emits the functions (and lays out .bss) in
            # reverse order, as the original has them; revisit when the original unit's flags are found.
            Object(
                Matching,
                "CRI/adx_crs.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_dcd.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_errs.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_fsvr.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_gc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_inis.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_insh.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_rnaa.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_mgc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_sugc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_dcd3.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: ADX_DecodeSte4AsSte (115 of 152 words differ) and ADX_DecodeSte4AsMono
            # (137 of 188) have the original's instructions (the same opcode counts and
            # relocations) in other registers and order; ADX_DecodeSte4AsSte also saves one
            # callee-saved register more (r18). The other two functions match.
            Object(
                NonMatching,
                "CRI/adx_dcd5.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_fini.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_fcch.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Matching workaround: -inline noauto, so only the functions declared inline are expanded; revisit
            # when the original unit's flags are found.
            Object(
                Matching,
                "CRI/adx_fs.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline noauto",
                ],
            ),
            # NonMatching: cvFsGetFileSize differs in 14 words (two registers swapped).
            Object(
                NonMatching,
                "CRI/cri_cvfs.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_xpnd.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Matching workaround: -inline auto,deferred. MWCC then emits the functions (and lays out .bss) in
            # reverse order, as the original has them; revisit when the original unit's flags are found.
            Object(
                Matching,
                "CRI/adx_lsc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_sje.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Matching workaround: -inline noauto, so only the helpers declared inline are expanded; revisit
            # when the original unit's flags are found.
            Object(
                Matching,
                "CRI/adx_stmc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline noauto",
                ],
            ),
            # Matching workaround: -inline auto,deferred. MWCC then emits the functions (and lays out .bss) in
            # reverse order, as the original has them; revisit when the original unit's flags are found.
            Object(
                Matching,
                "CRI/adx_tlk.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            # Matching workaround: -inline auto,deferred. MWCC then emits the functions (and lays out .bss) in
            # reverse order, as the original has them; revisit when the original unit's flags are found.
            Object(
                Matching,
                "CRI/adx_tlk2.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            Object(
                NonMatching,
                "CRI/adx_tsvr.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/adx_sfa.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Matching workaround: -inline auto,deferred. MWCC then emits the functions (and lays out .bss) in
            # reverse order, as the original has them; revisit when the original unit's flags are found.
            Object(
                Matching,
                "CRI/adx_sjd.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            # NonMatching: mwsffrm_AnalySofdecHeader, 17 of 118 words: the handle and the Sofdec
            # header object have r30 and r31 exchanged, and the original copies the handle into its
            # register one prologue slot later. Every other function the original object keeps is
            # byte-identical.
            Object(
                NonMatching,
                "CRI/mwsfdfrm.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Matching workaround: -inline noauto on mwsfdsfx.c. The original keeps the conversion
            # helper (fn_80403520) and the tag reader (mwsftag_GetAinfFromSj) out of line, while
            # the small helpers written as static inline functions are expanded; automatic
            # inlining expands the first two into their callers (fn_804035A8 0x80 bytes instead
            # of 0x30, MWSFTAG_UpdateTagInf 0x20C instead of 0xB8). Revisit when the library's
            # real compiler flags are known.
            # NonMatching, USA raw differing words per function: MWSFSFX_CnvFrmInfToSfx 12 of 231;
            # the other functions are exact. 7 words are in the YCC plane block: the original
            # loads plane 1's buffer before its pitch (this build loads the pitch first for all
            # three planes, as the original does for planes 0 and 2) and gives r5 to the luma
            # pitch and r4 to the halved height (this build the other way round). 5 are string
            # offsets: the original pools the function's four error texts in the reverse of their
            # order of use. Per-switch inline helpers, function-local static texts and named
            # static texts declared in their order of use keep that order; only declaring named
            # texts in the reverse order reproduces it.
            Object(
                NonMatching,
                "CRI/mwsfdsfx.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline noauto",
                ],
            ),
            Object(
                Matching,
                "CRI/mwsfdply.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching, USA raw differing words per function (branch displacements included;
            # data, rodata and bss match): mwSfdDestroy, MWSFCRE_ResetSfdHn, mwsfcre_MallocRfb,
            # MWSFCRE_SetSupplySj and mwPlyCalcWorkCprmSfd exact; mwsfcre_MallocCompoWork 69 of
            # 177 and mwPlyCreateSofdec 116 of 1037, register colouring only (the handle and the
            # creation parameters take different callee-saved registers); mwPlyCalcWorkSfd 13 of
            # 119, register colouring and the order of the last two additions; mwsfcre_CreateSfd
            # is 12 words short (908 of 920) and at objdiff row alignment 44 rows differ (32
            # register, 12 other): the original keeps the allocator's negative size test in the
            # 0x4000 and 0x700 allocations (10 words), plus one additional branch in each of
            # its two ADXT type-4 arms. The explicit type-4 comparisons are reproduced.
            Object(
                NonMatching,
                "CRI/mwsfdcre.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mwsfdrna.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mwsfdsst.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mwsfdlib.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/fn_8040305C.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mwsfdsl.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Matching workaround: -inline auto,deferred on mwsfdset.c and mwsfdsvr.c. Without it
            # mwPlyGetSfdHn is 80 bytes instead of 84 (18 differing words) and mwSfdExecDecSvrHndl
            # differs by 4 words. Revisit when the library's real compiler flags are known.
            Object(
                Matching,
                "CRI/mwsfdset.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            Object(
                Matching,
                "CRI/mwsfdsvr.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            Object(
                Matching,
                "CRI/fn_80408680.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: 4 of 7 functions match; the .rodata and .bss bytes match. The paired-single converters
            # are C with asm statements only for GQR4 and the paired-single adds, merges and quantised stores; all
            # their raw words differ: cnvDynamicYcc420plnToArgb8888 is 0x50C bytes (original 0x5C8, 370 words) and
            # cnvStaticYcc420plnToArgb8888 0x498 (original 0x508, 322 words). The static converter's plane and
            # table loads come in the same order as the original's relative to dcbz and the stores; in the dynamic
            # one the compiler keeps the chroma table values of pixel rows 0 and 2 for rows 1 and 3, where the
            # original loads 3 and 5 of them again. CFT_Ycc420plnToArgb8888Init differs in 9 of 63 words (f6, f7
            # and f8 rotate).
            # Matching workaround: -inline auto,deferred. MWCC then emits the functions (and lays out .bss) in
            # reverse order, as the original has them; revisit when the original unit's flags are found.
            Object(
                NonMatching,
                "CRI/cftyp422_ppc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            # NonMatching: 3 of 4 functions match. cnvDynamicYcc420plnToA256UserTable differs in 137 of 139
            # words (same size): the original advances a separate register per tile row (ywidth - 4 computed
            # in the inner loop and copied, the +4 of each row kept as its own addi) and saves r25-r31 with
            # stmw in a 48-byte frame; ours walks one pointer by ywidth per row, reads the samples at
            # negative offsets and saves four registers in a 32-byte frame.
            # Matching workaround: -inline auto,deferred. MWCC emits the functions (and lays out .bss) in
            # reverse source order. Revisit if the original unit's compiler flags are found.
            Object(
                NonMatching,
                "CRI/cftfx.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            Object(
                Matching,
                "CRI/mwsfdrsc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mwsfdsvm.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mwstm.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mcp_not.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_m2v.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mwsfdsee.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/fn_80403E4C.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/rna_crs.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: AXRNA_SetPlaySw, 1 of 99 words: the add that forms the voice's end
            # address has its source operands in the other order (original `add r8,r7,r8`,
            # loop + last; this source `add r8,r8,r7`); the other 22 functions match.
            Object(
                NonMatching,
                "CRI/ax_rna.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_cmc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_con.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # sfadxt_ExecServerSub 62 differing words, sfadxt_ExcludeHdr 38 (same sizes,
            # registers only: the parameters sfd / data take other saved registers).
            Object(
                NonMatching,
                "CRI/sfd_adxt.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_aoap.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_see.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # SFD_SetCond: 8 differing words (r28/r31 swapped between the handle pointer and the
            # shifted condition index).
            Object(
                NonMatching,
                "CRI/sfd_set.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                NonMatching,
                "CRI/sfd_tim.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_trn.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: all 39 functions are written and 27 are identical. With equal sizes, raw
            # words differ in SFMPV_AddRead (2 of 71), SFMPV_Destroy (40 of 68), SFMPV_Create
            # (61 of 156), sfmpv_SetFrmPara (25 of 156), sfmpv_IsSkip (7 of 268) and
            # sfmpv_ExecServerSub (78 of 263). The sizes differ in these (original / source words,
            # raw words differing by position, then the larger length minus the longest common
            # subsequence of words): SFMPV_Stop (4 / 2, 4, 2), sfmpv_InitInf (228 / 229, 145, 19),
            # sfmpv_GoDdelim (164 / 163, 91, 51), fn_803F5134 (338 / 353, 346, 236),
            # sfmpv_DecodePicAtr (702 / 699, 495, 97) and sfmpv_Concat (210 / 201, 76, 13). The
            # original sfmpv_Concat tests the time difference against zero again after the clamp and
            # compares a status (0, or -1 after a failed sample count read) with -1 before it
            # discards the end codes; ours returns directly when the sample count cannot be read.
            # The original SFMPV_Stop loads the driver handle and compares it with zero without
            # using the result; ours does neither. In SFMPV_AddRead the original loads the used flag
            # before it computes the frame record's address; ours computes the address first.
            # sfmpv_SetFrmPara differs in registers and scheduling only (the same instructions): the
            # registers of the picture width and height and the order of their rounding steps, and
            # the registers and order of the plane size products and of the load of the first
            # reference frame's buffer.
            # sfmpv_IsSkip has r0 and r3 exchanged between picstat and the result of the picture
            # state test. The original sfmpv_DecodePicAtr loads the work pointer from the handle
            # again before the PTS lookup and the stream information, and copies the bit rate, the
            # VBV buffer size and the picture time into further registers; ours keeps one work
            # pointer there and uses the values where they are. The register assignment differs in
            # SFMPV_Destroy, SFMPV_Create and sfmpv_ExecServerSub. Beyond that, the original
            # SFMPV_Destroy interleaves an mr, a lis and an addi with the register saves, while
            # ours places them after the saves, the original SFMPV_Create saves r29 before the
            # addi of the .bss base where ours saves it after, and one compare in
            # sfmpv_ExecServerSub is cmpwi where the original has cmplwi.
            # Deferred inlining: the original .bss holds the variables in the reverse of their
            # declaration order (sfmpv_discard_wsiz first, sfmpv_work last), as a deferred compile
            # lays them out; without it they come out in the order of first use (sfmpv_para first)
            # and SFMPV_Init, SFMPV_Destroy, SFMPV_Create and SFD_SetMpvParaTbl differ.
            Object(
                NonMatching,
                "CRI/sfd_mpv.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_mpvf.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_ply.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_buf.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # SFHDS_SetHdr: 11 differing words (register choice for result).
            Object(
                NonMatching,
                "CRI/sfd_hds.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # SFPTS_ReadPtsQue: 11 differing words at matching offsets from conditional success-branch
            # layout in the found tests and an extra queue-index copy.
            Object(
                NonMatching,
                "CRI/sfd_pts.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: the seven little-endian 32-bit readers (SFH_AnlyElemSmpHz,
            # SFH_AnlyMaxFrmNum, SFH_AnlyMaxPlyLenVid, SFH_AnlyMaxPlyLenAud, SFH_AnlyPackSiz,
            # SFH_AnlyHdrSiz and SFH_AnlyByteRate) store the swapped word with stwbrx where the
            # original has a rlwinm/rlwimi chain and a plain stw: the original is 87/39/44 words
            # where ours is 83/35/40. Compared in order, each differs in 6 words: 2 replaced (the
            # load's destination register and the first rlwinm against stwbrx) and 4 original-only
            # (the three rlwimi and the stw). The other 29 functions match.
            # Why: after register allocation MWCC turns a `stw` at offset 0 into `stwbrx` when the
            # stored register is built in the same basic block by the four-lane rlwinm/rlwimi swap
            # of one source register that is not redefined before the store. Why the original's
            # store was not folded is not established; one possibility is that the store was in a
            # different block when the fold ran. The original's loaded word is also in r6 where
            # ours takes r5 (r4 in SmpHz), the register of the header pointer (of the stream id in
            # SmpHz), so that register may have stayed live past the load in the original.
            Object(
                NonMatching,
                "CRI/sfh_main.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_mem.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_uo.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_vom.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_tst.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_lib.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # sfmps_DecodeOneUnit: 16 differing words (register and base choice for the syshd
            # stores).
            Object(
                NonMatching,
                "CRI/sfd_mps.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/mpv_umc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # NonMatching: MPVMC16_OneRef4p_TuneC differs in 106 of 139 words (registers and order).
            Object(
                NonMatching,
                "CRI/mpv_mcy.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(Matching, "CRI/lsc_err.c", extra_cflags=["-sdata 0"]),
            Object(Matching, "CRI/lsc_crs.c"),
            # Matching workaround: -inline auto,deferred. MWCC then emits the functions (and lays out .bss) in
            # reverse order, as the original has them; revisit when the original unit's flags are found.
            Object(
                Matching,
                "CRI/lsc.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                    "-inline auto,deferred",
                ],
            ),
            # The build string lives in .rodata (-str readonly).
            Object(
                Matching,
                "CRI/lsc_ini.c",
                extra_cflags=["-sdata 0", "-sdata2 0", "-str readonly"],
            ),
            Object(
                Matching,
                "CRI/lsc_svr.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sj_crs.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # The two-part error messages need the readonly string pool.
            Object(
                Matching,
                "CRI/sj_mem.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sj_rbf.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sj_uni.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sj_utl.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sj_err.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/svm.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # Const pointer data stays out of small data (-sdata2 0).
            Object(
                Matching,
                "CRI/mfci.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            # String literals live in .rodata (-str readonly).
            Object(
                Matching,
                "CRI/gcci.c",
                extra_cflags=["-sdata 0", "-str readonly", "-use_lmw_stmw on"],
            ),
            Object(
                Matching,
                "CRI/gcci_sub.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
        ],
    },
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects


# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Progress categories (decomp.dev). Every configured unit belongs to exactly one
# top-level category (game, renderware, cri, sdk, runtime). Game units may also carry a
# subtag (csd, stlib, gcax) that overlaps "game". Categories cover only the configured
# source units; original code without a configured unit belongs to no category, so the
# labels say so. categorize_object below assigns these categories; the libraries above
# carry no progress_category of their own.
config.progress_categories = [
    ProgressCategory("game", "Game code (configured units)"),
    ProgressCategory("renderware", "RenderWare (configured units)"),
    ProgressCategory("cri", "CRI middleware, ADX/Sofdec (configured units)"),
    ProgressCategory("sdk", "Dolphin SDK (configured units)"),
    ProgressCategory("runtime", "CodeWarrior runtime, MSL/MetroTRK (configured units)"),
    ProgressCategory("csd", "Chao CSD UI library (subset of game code)"),
    ProgressCategory(
        "stlib", "Sonic Team middleware and effect library (subset of game code)"
    ),
    ProgressCategory("gcax", "Sega GameCube AX sound library (subset of game code)"),
]

# USA DOL address ranges [start, end) of the src/Game units that are not core game
# code, taken from the split layout. A unit is placed by the .text start address that
# config/GUPE8P/splits.txt gives it, so renaming or merging a unit keeps its category.
# CRI ADX/Sofdec objects that sit under src/Game:
CRI_ADDRESS_RANGE = (0x803B61F0, 0x8040ABC8)
# Sonic Team middleware, from Sonicteam::System::Heap (Game/fn_8040ABC8.cpp) up to the
# gcax library: Sonicteam::System::Peripheral, SonicUSA::System::Random, the stMtn*
# motion API and, for most of the range, the Effect::System and Effect::RW3 effect
# library (EffectManager, EffectParticle, EffectRay, EffectLocus, the EffCmd*Exec
# commands and their RW3 renderers):
STLIB_ADDRESS_RANGE = (0x8040ABC8, 0x8043AA7C)
# Sega's GameCube AX sound library (gcaxSndMemory.c and the gcax* units after it):
GCAX_ADDRESS_RANGE = (0x8043AA7C, 0x80440410)


def read_text_start_addresses(splits_path: Path) -> Dict[str, int]:
    """Map each unit name in a splits.txt to the start address of its .text split."""
    text_starts: Dict[str, int] = {}
    unit_name = None
    for line in splits_path.read_text(encoding="utf-8").splitlines():
        # Unit headers are unindented "name:" lines; their sections are indented.
        if line and not line[0].isspace() and line.endswith(":"):
            unit_name = line[:-1]
            continue

        text_match = re.match(r"\s+\.text\s+start:0x([0-9A-Fa-f]+)", line)
        if unit_name is not None and text_match is not None:
            text_starts[unit_name] = int(text_match.group(1), 16)
    return text_starts


# The ranges above are USA addresses, so every region categorises by the USA splits.
USA_TEXT_START_ADDRESSES = read_text_start_addresses(
    Path("config") / "GUPE8P" / "splits.txt"
)


def categorize_object(unit_name: str) -> List[str]:
    """Return the progress categories of a unit from its unit name.

    rw/, CRI/, dolphin/ and the runtime directories are matched by path, Game/CSD by
    path, and the other Game units by their USA .text start address, so a new or
    renamed unit needs no category edit as long as it keeps that layout.
    """
    # Path prefix -> top-level category (checked in order)
    prefix_categories = [
        ("rw/", "renderware"),
        ("CRI/", "cri"),
        ("dolphin/", "sdk"),
        ("PowerPC_EABI_Support/", "runtime"),  # MSL C/C++, Runtime, MetroTRK
        ("Runtime.PPCEABI.H/", "runtime"),
    ]
    for prefix, category in prefix_categories:
        if unit_name.startswith(prefix):
            return [category]

    # Everything else lives in src/Game and is game code unless it is middleware.
    if unit_name.startswith("Game/CSD/"):
        # Chao::CSD UI library (csd*.cpp __FILE__ strings, Chao::CSD RTTI names)
        return ["game", "csd"]

    address = USA_TEXT_START_ADDRESSES.get(unit_name)
    if address is not None:
        if CRI_ADDRESS_RANGE[0] <= address < CRI_ADDRESS_RANGE[1]:
            return ["cri"]
        if STLIB_ADDRESS_RANGE[0] <= address < STLIB_ADDRESS_RANGE[1]:
            return ["game", "stlib"]
        if GCAX_ADDRESS_RANGE[0] <= address < GCAX_ADDRESS_RANGE[1]:
            return ["game", "gcax"]
    else:
        # Without a USA .text start (a misspelt unit name or a unit absent from the USA
        # splits) the unit cannot be checked against the ranges above, so it falls back
        # to "game". Warn so a middleware unit is not misfiled silently.
        print(
            f"Warning: {unit_name} has no .text split in config/GUPE8P/splits.txt; "
            "categorised as game",
            file=sys.stderr,
        )
    return ["game"]


for library in config.libs:
    # A library-level progress_category (the dtk-template convention) would be
    # overridden by object-level categories, so leave such a library's objects alone.
    if library.get("progress_category") is not None:
        continue

    for library_object in library["objects"]:
        # An explicit Object(..., progress_category=...) overrides the rules above.
        if library_object.options["progress_category"] is None:
            library_object.options["progress_category"] = categorize_object(
                library_object.name
            )

config.progress_each_module = args.verbose
# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
