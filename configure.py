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
# Game/fn_8041116C.cpp and Game/fn_80420724.cpp (for <cmath>) search them in place of include/libc;
# C game code and the other fn_8040 C++ helpers (cflags_game) keep the C declarations in
# include/libc.
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
# 04:13:58, from the same build run, although the dolsdk2004 Makefile labels it Apr 20 2004.
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
                Object(NonMatching, "rw/fn_80446A04.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80447598.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804485BC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80448CB0.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8044920C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8044BAB4.c", cflags=[*cflags_game, "-inline noauto"]),
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
                Object(NonMatching, "rw/fn_80474BEC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80476ABC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80477344.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047832C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80479040.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80479598.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80479A50.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047A7C8.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047B720.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8047CCF8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8047DB10.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8047DC90.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047E100.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047FA28.c", cflags=cflags_game),
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
                Object(NonMatching, "rw/fn_8048A178.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8048ADE0.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8048B6AC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8048D7C0.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8048D85C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049043C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80491AB4.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804939DC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804951B8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80497198.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80497FB4.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049952C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80499B24.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049A78C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049A82C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049B0CC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049B14C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8049B1F8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049B8BC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8049D408.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049EA2C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049F2A4.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049F3F8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049F8AC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049FAFC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804A076C.c", cflags=cflags_game),
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
                Object(Matching, "Game/BackDrop.cpp", cflags=cflags_game_cpp),
                # 34 of 35 functions have identical instructions. The startup function is 12 words
                # instead of 25: the original builds each of its two vectors in a temporary and
                # copies it with the out-of-line vector copy. The unit's own data and its order are
                # the original's; the accessor of the switch counter is emitted after the template
                # instances that follow ApplyState here, directly after ApplyState in the original.
                Object(NonMatching, "Game/CandleStick.cpp", cflags=cflags_game_cpp),
                # All 52 functions have identical instructions; checked_delete<Chao> is a local
                # symbol here where the original's is weak. The data differs from the original's
                # in records the original unit does not have: a weak SetAdapterState vtable (with
                # its weak destructor) sits at .data 0x128, in front of the base-class list of
                # sp_counted_base_impl<Chao>'s type record, which moves that list and the
                # sp_counted_base_impl<Chao> vtable 0xC bytes later; the std::exception type
                # record (.sdata) and its two name strings (.rodata) come before those of
                # checked_deleter<Chao> (moving them 8 and 0x1C bytes later); and the
                # std::exception and sp_counted_base vtables are appended to .data.
                Object(NonMatching, "Game/ChaoHolder.cpp", cflags=cflags_game_cpp),
                # 33 of 36 functions are identical in every word and relocation. Two more have
                # identical words and differ only in the symbol name of one callee: the copies of
                # boost's list1 operator[] and function1 destructor at 0x800D05D0 and 0x800D0CEC share
                # their mangled names with copies at other addresses that symbols.txt already names.
                # The hit callback (0x80130364) differs in 19 words: the float-to-int conversion of
                # the level and the int-to-float conversion of the index get separate 8-byte stack
                # slots (the original reuses one; frame 0xB0 instead of 0xA0), and the level and the
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
                # 79 of the 85 functions have identical instructions. The preload hook differs in 5
                # words (the registers of the table pointer and the stage number), SetupPath in 2
                # (the order of two argument moves), ResetFromOwner in 7 (the original calls the
                # smart pointer's operator-> out of line for the yaw of the character info), the
                # camera event handler in 2 (the original tests its argument for null) and
                # UpdateBody in 14 (exchanged vector stack slots, one argument move scheduled
                # earlier, the register of the stage pointer); the conversion fn_80131654 has no
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
                # All 46 functions have identical instructions and are in the original's order,
                # and the original's data is reproduced at its offsets. The implementation class's
                # destructor is a global symbol here (a plain out-of-class definition under
                # auto_inline, which keeps boost::checked_delete out of line and emits the
                # destructor at its definition) where the original's is weak. Defining it inline,
                # in the class or after the constructor, makes it weak but emits it at its first
                # use instead of at its definition, which moves it away from the original's
                # position. The object also defines a local
                # boost::checked_delete<UnkAttack_8005A840> (0x24 bytes), which the original unit
                # does not have (the checked_deleter<UnkAttack_8005A840> call operator is external
                # there), and appends to .rodata, .data and .sdata the type records, name strings
                # and vtables of sp_counted_base, std::exception and the UnkAttack_8005A840
                # sp_counted_base_impl and checked_deleter.
                Object(NonMatching, "Game/WeaponStickBase.cpp", cflags=cflags_game_cpp),
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
                # 55 of the 78 functions have identical instructions; 44 of them also have identical
                # relocation symbol names, the others refer to symbols that keep placeholder names.
                # checked_delete<CityLaser> is a local symbol here where the original's is weak.
                # SetState differs in 5 words (the order of the stack argument of the probe call
                # and the moves around it) and Update in 5 (the stack slot of the result of
                # fn_8040FC3C).
                # The rest are the std::vector instances of weak pointers: insert, reserve and
                # swap differ in structure (the original reaches the capacity and the allocator
                # through more out-of-line accessors), the vector destructor and the __vector_obj
                # constructor differ, and 16 small helper functions (the constructor chain, the
                # accessors, the pair swap) are not reproduced one for one.
                Object(NonMatching, "Game/CityLaserHolder.cpp", cflags=cflags_game_cpp),
                # All 48 functions and the data match the original once the weak copies that the
                # linker discards are removed, and linking this object gives the original
                # executable, but checked_delete<FallBuilding> is a local symbol here where the
                # original's is weak, as in CityLaserHolder.cpp.
                Object(NonMatching, "Game/FallBuildingHolder.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecCristalWallSwitch.cpp", cflags=cflags_game_cpp),
                Object(NonMatching, "Game/ElecCube.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecPillar.cpp", cflags=cflags_game_cpp),
                Object(NonMatching, "Game/GrindGadget.cpp", cflags=cflags_game_cpp),
                # 11 of 12 functions have identical instructions. UpdateBody differs in 13 words: the
                # original loads the two startup floats (the hidden distance along the footing's z
                # axis and the speed) before the stores to the gadget's members, so one load is scheduled
                # earlier and its register numbers differ in the hidden-height assignments and the
                # final comparison.
                Object(NonMatching, "Game/PopFooting.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecSmallPillar.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecWakka.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/GravityChangeCollision.cpp", cflags=cflags_game_cpp),
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
                # TransformPoint, UpdateEntries, UpdateEntry, EvaluateChannels,
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
                # instruction streams are aligned): UpdatePalette (244 words, the original's 243)
                # differs in 164 (111): the registers of the arguments and locals, plus the
                # branches of the early exit and of the loop heads (the opcode counts differ only
                # in b, beq, bne and mr). SetEntryPattern (317 words, the original's 319) differs
                # in 201 (105): the original copies the vertices that change layout word by word
                # through integer registers and addresses some of the stores differently.
                # RCPtr::Assign (117 words, the original's 120) differs in 100 (24): the original
                # keeps the object to release in r4 and copies it to r3 for each call, and the
                # allocation of the holder passes line 132 of csdRCObject.h where the original
                # passes 410. Reset and the holder's constructor come after RenderState::End, as
                # in the original; the out-of-line CMemoryAlloc::AllocObject that the holder
                # allocation emits after them is a weak copy of a function the game has at
                # 0x8003DF84.
                Object(NonMatching, "Game/CSD/csdLoader.cpp", cflags=cflags_csd),
                # Every function matches except InitMatrixStack, whose allocations pass lines 64 and
                # 65 instead of the original 53 and 54. .rodata also starts with the two static
                # doubles of MSL <cmath>'s sqrtf, unreferenced weak data that the link discards.
                Object(NonMatching, "Game/CSD/csdMatrix.cpp", cflags=cflags_csd),
                Object(Matching, "Game/CSD/fn_80360528.cpp", cflags=cflags_csd),
                # Current file system and its directory (original file name unknown).
                Object(Matching, "Game/CSD/fn_80361738.cpp", cflags=cflags_csd),
                Object(Matching, "Game/CSD/fn_80361A24.cpp", cflags=cflags_csd),
                Object(Matching, "Game/CSD/fn_80361CF8.cpp", cflags=cflags_csd),
                # Every function present matches except BuildTextEntry and InitPalette, which address
                # .rodata through one section base where the original addresses each literal on its
                # own (probably also the cause of their register and scheduling differences), and
                # whose allocations pass lower __LINE__ values than the original 442 and 234.
                # MarkMatrixUsers also comes after its caller MarkLayerMatrixUsers, where the
                # original has it before.
                Object(NonMatching, "Game/CSD/csdMotionPalette.cpp", cflags=cflags_csd),
                # Every function matches except LoadFile (its allocation passes __LINE__ 47 where
                # the original passes 137) and RegisterTexList (the pair conversion takes its
                # argument from a recomputed stack address where the original reuses make_pair's
                # returned pointer: two words). The map's out-of-line functions also come out in a
                # different order: __destroy and its four helpers directly follow the tree
                # destructor, where the original has them after root, InitTexLists, the map
                # constructor, tree::find, its two key comparisons and comp; after the map
                # constructor the final block begins with erase (followed by __size, __begin, begin
                # and insert) instead of tree::find, and comp, tree::find and its two comparisons
                # come only after insert's pair constructor and comparison (the original's order is
                # not reproduced yet). The inline CMemoryAlloc::FreeObject is also emitted as an
                # unreferenced weak copy (0x2C bytes); the original's copy was removed at link, and
                # the game keeps its own at 0x8003CB64.
                Object(NonMatching, "Game/CSD/csdTexList.cpp", cflags=cflags_csd),
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
                # frame range, speed, pause, motion and palette members and the setters of its root
                # pattern, and the constructor of the scene's end-of-motion policy (the class whose
                # handlers are in csdmProject.cpp). 53 of the unit's 195 functions are sourced, and each
                # has the original's instructions and relocation targets (GetAspect's float literal is a
                # differently named label of the same value). The other 142 are not sourced: the scene's
                # Copy, destructor, Update and Draw, the end-of-motion policy's destructor, the
                # CSceneListener constructor, the container code (lists and trees of listeners and
                # nodes), the RCPtr instances (for MatrixValue and for the node pointers), the listener
                # dispatch and the root pattern's allocation. The unit's .rodata and .data
                # are incomplete for the same reason (the strings and the vtables of the unsourced
                # classes are missing), and it emits weak copies of RCPtr<unsigned char>::Reset, the
                # RCObject<unsigned char> constructor and vtable and CMemoryAlloc::AllocObject and
                # FreeObject, which the link discards. It also emits a local instance of the
                # delete-policy Assign (0x1C0 bytes) that only the weak Reset calls: this unit's form of
                # the original's fn_803655F8 (0x1E0 bytes), which it does not match yet.
                Object(NonMatching, "Game/CSD/csdmScene.cpp", cflags=cflags_csd),
                # The palette object of a scene (CMotionPalette, which owns the palette block
                # through a reference-counted handle) and the CMotionPattern constructor that wraps
                # one of its entries. 12 of 12 functions are sourced; 8 match in raw instruction
                # words, and the unit's .rodata layout is identical. The CMotionPalette constructor
                # and Rebuild differ in one word each, the __LINE__ of their allocations (the
                # original passes 55 and 100). The holder Assign for the function release policy
                # (112 words, the original's 115) shows the difference described for csdLoader.cpp.
                # GetPattern differs in 7 words: the original loads the element's index before the
                # palette and the layer index (scheduling; four spellings of the entry address gave
                # the same code). The holder's SetRelease matches but comes last in the unit, where
                # the original has it right after the holder constructor for the function policy
                # (see csdRCObject.h). The unit also emits weak copies of
                # RCPtr<unsigned char>::Reset and the holder constructor for the delete policy and
                # of CMemoryAlloc::AllocObject, which the link discards in favour of the game's
                # earlier copies, and a local instance of the delete-policy Assign that only the
                # discarded Reset calls. It also emits a weak copy of the RCObject<unsigned char>
                # vtable (the original unit has no .data), which the link discards in favour of the
                # game's copy.
                Object(NonMatching, "Game/CSD/csdmMotionPalette.cpp", cflags=cflags_csd),
                # The accessors that override the animated values of an element's entry
                # (CMotionPattern), its constructor and Copy, and UpdateTransform. 32 of 32
                # functions are sourced. 9 have identical instructions; 13 differ only in the
                # __LINE__ passed by their allocations (one word each, two in UpdateTransform; the
                # original's lines run from 54 to 1067 in a file far longer than this one). The
                # holder Assign for the free and function release policies (112 words, the
                # original's 115) shows the difference described for csdLoader.cpp. Six getters
                # (GetHidden, GetRotation, GetTint, GetPattern, GetValue and GetColor) differ in the
                # order of two prologue instructions (the original loads the entry pointer before it
                # saves the link register; 2 words each). GetValue and GetColor also swap the
                # registers of the channel bit and the mask (6 words each in all). GetTranslation
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
                # The node of a scene element: the handle of its file data (constructor, destructor,
                # Clear, Attach and Copy), the accessors that forward to its CMotionPattern (hidden
                # flag, translation, rotation, scale, pattern, tint, colours and user values), the
                # channel defaults and the node's copy of the fonts. 32 of the unit's 97 functions are
                # sourced, and each has the original's instructions and relocation targets
                # (SetTranslateX, SetTranslateY and SetPattern use a differently named label for their
                # float literal). The other 65 are not sourced: the node's constructor and destructor,
                # the loop over a map of other nodes that sets the pattern's values from them, and
                # the containers and RCPtr instances they need. The unit's .rodata and .data are
                # incomplete for the same reason, and it emits weak copies of
                # RCPtr<unsigned char>::Reset, the RCObject<unsigned char> constructor and vtable and
                # CMemoryAlloc::AllocObject, which the link discards, and a local instance of the
                # delete-policy Assign (0x1C0 bytes) that only the weak Reset calls (the original
                # unit has no such function).
                Object(NonMatching, "Game/CSD/csdmNode.cpp", cflags=cflags_csd),
                # The project object (CProject: the file data of a project with the texture list and
                # the fonts its scenes use) and the handlers of the scene's end-of-motion policy.
                # 18 of the unit's 178 functions are sourced. 14 have the original's instructions:
                # the policy's OnReachStart and OnReachEnd, the four empty events of CSceneListener,
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
                Object(Matching, "Game/fn_800091D0.c"),
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
                Object(NonMatching, "Game/EffectManager.cpp", cflags=cflags_effect),
                Object(NonMatching, "Game/fn_80425860.cpp", cflags=cflags_effect),
                Object(NonMatching, "Game/EffectArranger.cpp", cflags=cflags_effect),
                Object(Matching, "Game/fn_8040D8D4.c"),
                Object(Matching, "Game/fn_8040D850.c"),
                Object(Matching, "Game/fn_80424D18.cpp", cflags=cflags_effect),
                Object(Matching, "Game/fn_8040D9D8.c"),
                Object(Matching, "Game/fn_8040D498.c"),
                # 3x4 matrix helpers. Nine of 28 functions still differ. The three axis rotations
                # (fn_8040B0C0, fn_8040B138, fn_8040B1B0) load r0 and r31 in the opposite order in
                # the epilogue, and the look-at matrix (fn_8040B33C) swaps one fneg with the next
                # lfs (2 words each). The quaternion matrix (fn_8040B228) and the vector transforms
                # (fn_8040B618, fn_8040B67C) have the same instructions in a different order or with
                # different registers. The inverses (fn_8040BCD4, fn_8040BEF8) differ in scheduling,
                # registers and saved registers: fn_8040BEF8 has the target's arithmetic opcodes and
                # saves 1 more FPR (152 instructions instead of 148); fn_8040BCD4 reuses the
                # determinant's products m10 * m22 and m12 * m20 for element [1][0], which the target
                # computes again (2 fsubs and 1 fneg where the target has 1 fmsubs and 1 fnmsubs),
                # and saves 2 more FPRs (146 instructions instead of 137).
                Object(
                    NonMatching,
                    "Game/fn_8040B034.cpp",
                    cflags=cflags_game_cpp,
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                Object(Matching, "Game/fn_8040C244.c"),
                Object(Matching, "Game/fn_8040CC5C.c"),
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
                Object(Matching, "Game/fn_8040CCD4.c"),
                Object(Matching, "Game/fn_8040D6F4.c"),
                Object(Matching, "Game/fn_8040E38C.c"),
                # SonicUSA::System::Random. Matching workaround: -sdata 0 and -sdata2 0; the target
                # keeps its doubles in .rodata and its RTTI record and twist table in .data. Revisit
                # when the original unit's flags are known. NextFloat and
                # NextFloatInclusive still differ: the target adds the m_state[i + 397] offset to
                # an index register (lwzx), while this build folds it into the load displacement.
                Object(
                    NonMatching,
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
                # -sdata2 0; revisit when the original unit's flags are known). fn_8040FC78
                # (closest point on a segment) still differs: the target calls fn_800091EC (copy;
                # start or end of the segment) and fn_8040CDD0 (add; the point in between) as
                # by-value functions that build the point directly in its caller's result slot,
                # which their void (destination, source) prototypes cannot express. This build
                # builds the point in a local and copies it out: 75 instructions instead of 69,
                # with a stack address in place of the result pointer in the three calls. A
                # result-pointer signature matches fn_8040FC78 but makes its by-value wrapper
                # fn_8040FC3C differ.
                Object(
                    NonMatching,
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
                # Game/fn_8041116C.cpp, Game/fn_80411714.c, Game/fn_80413B30.c, Game/fn_804157E8.c,
                # Game/fn_8041583C.c, Game/fn_80415854.c and the later NonMatching pieces of the
                # control of motions (Game/fn_80416198.cpp, fn_8041FD70.cpp, fn_8041FF0C.cpp,
                # fn_80420308.cpp and fn_8042043C.cpp) and the later units from Game/fn_80420724.cpp
                # to Game/fn_804211CC.c carry it. Game/fn_804110EC.c, Game/fn_804202B8.c and the
                # empty functions Game/fn_8041FD6C.c, Game/fn_8041FF08.c and Game/fn_80420438.c
                # omit it: their objects are identical with or without it.
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
                # The middle piece of the control of motions (see Game/fn_804157E8.c
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
                Object(Matching, "Game/fn_8040CB2C.c"),
                Object(Matching, "Game/fn_8040E310.c"),
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
                # Groups of numbered resources. Not complete: fn_80411818 and fn_80411DEC are not
                # written yet, and fn_804124DC, fn_80413058, fn_80413318 and fn_80413538 differ.
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
                # units around the Matching pieces Game/fn_8041583C.c, fn_8041FD6C.c,
                # fn_8041FF08.c, fn_804202B8.c and fn_80420438.c until the whole file matches. The
                # flags follow the neighbouring library code. Game/fn_804157E8.c cannot link yet:
                # it owns the file's first .sdata2 constant, which unsourced code of the same file
                # references by name. Game/fn_804157E8.c and fn_80415854.c stay the C units they
                # were; the pieces added with the newly written functions are C++, the language of
                # the file (it has a static initializer).
                Object(
                    NonMatching,
                    "Game/fn_804157E8.c",
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
                    "Game/fn_80415854.c",
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
                # Shared RTTI and floating-point literals remain in the original fallback.
                Object(NonMatching, "Game/fn_80422070.cpp", cflags=cflags_effect),
                Object(NonMatching, "Game/EffCmdRequestStopExec.cpp", cflags=cflags_effect),
                Object(NonMatching, "Game/EffCmdLaunchExec.cpp", cflags=cflags_effect),
                # Six method instruction sequences match. Their local RTTI is shared with
                # remaining command classes and stays in the original data fallback.
                Object(NonMatching, "Game/fn_80422B18.cpp", cflags=cflags_effect),
                # Exact instruction sequences; floating-point literals remain shared with other commands.
                Object(NonMatching, "Game/EffCmdMatAlphaExec.cpp", cflags=cflags_effect),
                Object(NonMatching, "Game/EffCmdMatColorExec.cpp", cflags=cflags_effect),
                Object(NonMatching, "Game/EffCmd2DSclExec.cpp", cflags=cflags_effect),
                Object(NonMatching, "Game/EffCmd3DSclExec.cpp", cflags=cflags_effect),
                # Exact pattern methods; local float literals remain shared with other commands.
                Object(NonMatching, "Game/EffCmdPtnAnimExec.cpp", cflags=cflags_effect),
                # Exact instruction sequences; local float literals remain shared with other commands.
                Object(NonMatching, "Game/EffCmdVector3Exec.cpp", cflags=cflags_effect),
                Object(NonMatching, "Game/EffCmdVector2Exec.cpp", cflags=cflags_effect),
                Object(NonMatching, "Game/EffCmdSingleExec.cpp", cflags=cflags_effect),
                # Exact methods; other commands still share the local floating-point literals.
                Object(NonMatching, "Game/EffCmdWaitExec.cpp", cflags=cflags_effect),
                # Twelve destructor instruction sequences match. Their vtables/RTTI belong with
                # other command classes, across the intervening matching unit.
                Object(NonMatching, "Game/fn_80424D20.cpp", cflags=cflags_effect),
                Object(Matching, "Game/fn_8040CF2C.c"),
                Object(Matching, "Game/fn_804202B8.c"),
                Object(Matching, "Game/fn_8040CD20.c"),
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
            # The size and preparation helpers match; the vertex-byte counter retains twelve
            # register-word differences in its normal-attribute case.
            Object(NonMatching, "rw/fn_8046ACBC.c", cflags=cflags_game),
            Object(NonMatching, "Game/fn_801A1FA0.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_801A35AC.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_801A4430.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_801A54E8.cpp", cflags=cflags_game_cpp),
            Object(NonMatching, "Game/fn_801A61E4.cpp", cflags=cflags_game_cpp),
            # All six function bodies match; float literal order and readonly-span layout differ.
            Object(NonMatching, "Game/fn_801A6588.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_804220CC.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/EffCmdGlareSWExec.cpp", cflags=cflags_effect),
            Object(Matching, "Game/fn_8042A2D8.cpp", cflags=cflags_effect),
            Object(Matching, "Game/EffectGlobalParam.cpp", cflags=cflags_effect),
            Object(Matching, "Game/EffectLensFlare.cpp", cflags=cflags_effect),
            # Nine of ten functions identical; the constructor differs in allocation
            # exception handling and loop code (592 versus 556 bytes).
            Object(NonMatching, "Game/EffectShimmer.cpp", cflags=cflags_effect),
            # Six of eight functions identical; Update has six register-word differences; the
            # constructor is 968 versus 920 bytes, including allocation exception handling.
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
            # Four functions match; the vertex builder is 2556 versus 2736 bytes,
            # with vertex-copy, stack and instruction-layout differences.
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
            Object(Matching, "PowerPC_EABI_Support/Runtime/Src/ptmf.c"),
            Object(
                Matching,
                "PowerPC_EABI_Support/Runtime/Src/NMWException.cp",
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Math/Double_precision/e_acos.c"),
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
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/ansi_files.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common_Embedded/Src/ansi_fp.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/arith.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/bsearch.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/buffer_io.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/char_io.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/Src/critical_regions.gamecube.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/ctype.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/fn_803B1F68.c"),
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
            # NonMatching: RNARES_Init, 29 words: register allocation only (the buffer offset and
            # the 0x1000 size constant swap r4 and r5 in the copy loop).
            Object(
                NonMatching,
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
            # NonMatching: sfxcnv_ChkCompoByCbFunc, 219 of 1065 words with the same opcodes: the
            # handle and destination registers are exchanged throughout and some locals get other
            # registers, buf[0] is loaded into a saved register before the converter's frame
            # record is filled instead of after it, and the .rodata offsets differ by 0x20 because
            # the messages of functions that the linker removed (an E5022301 work size check,
            # SFX_MakeTblZ32 and SFX_MakeTblZ16; 0x78 bytes of .rodata) are not reproduced; the
            # other 10 functions match.
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
            # NonMatching: SFXA_Create, 12 of 36 words: the same instructions, but the address of
            # sfxa_work is formed before the constants instead of after them, so the constants take
            # other registers; the other 7 functions and the .bss layout match.
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
            # NonMatching: DCT_AcInit reaches its four double literals through one pooled .rodata
            # base (the original loads each with its own lis/lfd pair, and its literal order is
            # 0.5, int-to-double, 1/sqrt(8), pi/8): 59 words; the other functions match. The .bss
            # puts dctac_version_dummy before dctac_f_const (first-reference order; the original
            # stores it last).
            Object(
                NonMatching,
                "CRI/dct_ac.c",
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
                NonMatching,
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
            # NonMatching: MWSFSFX_CnvFrmInfToSfx, 12 of 231 words: two chroma plane loads in the
            # other order with r4/r5 exchanged (7 words), and five string offsets, because the
            # original pools the function's four error texts in the reverse of their order of use
            # (the reason is not known).
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
            # NonMatching: 2 of 4 functions match. cnvStaticYcc420plnToA256V differs in 20 of 103 words and
            # cnvDynamicYcc420plnToA256UserTable in 137 of 139.
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
            # sfadxt_ExecServerSub 239 differing words (1048 vs 1064 bytes),
            # sfadxt_ExcludeHdr 38 words.
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
            # NonMatching: all 39 functions are written and 19 are identical (sfmpv_ChkFatal among
            # them). With equal sizes, raw words differ in SFMPV_AddRead (2 of 71), SFMPV_Seek
            # (24 of 93), SFMPV_Destroy (40 of 68), SFMPV_Create (61 of 156),
            # sfmpv_ExecServerSub (78 of 263), sfmpv_NeedSafeDlmRefresh (34 of 233) and
            # sfmpv_GetActiveSize (95 of 474). The sizes differ, or the stack frame shifts the
            # offsets of most words, in these (original / source words, then the larger length
            # minus the longest common subsequence of words): SFMPV_Stop (4 / 2, 2),
            # sfmpv_InitInf (228 / 229, 19), sfmpv_SetFrmPara (156 / 155, 36), sfmpv_GoDdelim
            # (164 / 163, 62), sfmpv_DecodeFrm (324 / 322, 171), sfmpv_CalcRepeatField (308 / 310,
            # 63), sfmpv_IsSkip (268 / 255, 80), sfmpv_DecodeOneUnit (364 / 356, 170),
            # sfmpv_Pts2Tc (119 / 118, 94), sfmpv_DoReformTc (148 / 143, 112), sfmpv_Concat
            # (210 / 204, 120), fn_803F5134 (338 / 353, 236) and sfmpv_DecodePicAtr (702 / 702,
            # 230; 464 words differ by position). The original SFMPV_Stop loads the driver handle
            # and compares it with zero without using the result; ours does neither. In
            # sfmpv_Pts2Tc the original addresses sfmpv_fps_round, sfmpv_conv_59_94 and
            # sfmpv_conv_29_97 each with its own lis/addi pair, while ours reaches all three from
            # one pooled .rodata base. The register assignment differs in SFMPV_Destroy,
            # sfmpv_NeedSafeDlmRefresh and sfmpv_ExecServerSub. Beyond that, the original
            # SFMPV_Destroy interleaves an mr, a lis and an addi with the register saves, while
            # ours places them after the saves, and one compare in sfmpv_ExecServerSub is cmpwi
            # where the original has cmplwi.
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
            # SFBUF_InitHn: 429 differing words (1712 vs 1744 bytes; adr kept on the stack in the
            # original).
            Object(
                NonMatching,
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
            # SFPTS_ReadPtsQue: 11 differing words (branch layout of the two found arms).
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
            # SFH_AnlyHdrSiz and SFH_AnlyByteRate) store the swapped word with stwbrx; the original
            # has a rlwinm/rlwimi chain and a plain stw (87/39/44 words against 83/35/40: in each,
            # 3 aligned words differ and the original has 4 more, 7 in all); the other functions
            # match.
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
