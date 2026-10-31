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

# The MSL C, math and C++ library headers. The Elec and Chao::CSD C++ units (cflags_game_cpp,
# cflags_csd) and the fn_8040 vector length helpers search them in place of include/libc; C game
# code and the other fn_8040 C++ helpers (cflags_game) keep the C declarations in include/libc.
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

# The fn_8040 vector length helpers (C++ with exception tables) call std::sqrt(float) from MSL
# <cmath>. One level of inlining keeps that call out of line, because std::sqrt's body calls the
# inline sqrtf; plain -inline auto expands both into the helper. Each object also carries weak
# copies of std::sqrt and sqrtf with their literals, which the link discards in favour of the
# copies at 0x8000FEE4 and 0x8000FF04. The std::sqrt and sqrtf copies at those addresses match
# <cmath> compiled with fp_contract on (cflags_base), so the helpers do not turn it off.
cflags_game_vector_length = [
    *cflags_base,
    *msl_includes,
    "-Cpp_exceptions on",
    "-inline noauto,level=1",
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
    "-DSDK_REVISION=1",
]
# GX and OS in this game are the Nov 10 2004 builds (SDK_REVISION 2 in the reference sources);
# the other SDK libraries are the Apr 2004 builds (revision 1).
cflags_sdk_rev2 = [flag for flag in cflags_sdk if flag != "-DSDK_REVISION=1"] + ["-DSDK_REVISION=2"]


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
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_sdk,
        "progress_category": "sdk",
        "objects": objects,
    }


# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "progress_category": "game",
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
        "progress_category": "sdk",
        "objects": [
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/mainloop.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/nubevent.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/nubinit.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/msg.c"),
            Object(Matching, "PowerPC_EABI_Support/MetroTRK/msgbuf.c"),
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
        "progress_category": "sdk",  # str | List[str]
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
        "progress_category": "game",
        "objects": (
            [
                Object(Matching, "Game/gcaxSndMemory.c", cflags=cflags_game_gcax),
                Object(Matching, "Game/fn_8043B25C.c", cflags=cflags_game_gcax),
                Object(NonMatching, "Game/fn_8043BD80.c", cflags=cflags_game_gcax),
                Object(Matching, "Game/fn_8043D7B4.c", cflags=cflags_game_gcax),
                Object(Matching, "Game/fn_8043E2B4.c", cflags=cflags_game_gcax),
                Object(Matching, "Game/fn_8043FB58.c", cflags=cflags_game_gcax),
                Object(Matching, "rw/fn_80440410.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_80442B98.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_80443044.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_80443A74.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_804447B0.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_804448B8.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_80445CF4.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_80446A04.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_80447598.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_804485BC.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_80448CB0.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_8044920C.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_8044BAB4.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_8044CDC4.c", cflags=[*cflags_game, "-inline auto"]),
                Object(NonMatching, "rw/fn_8044E6BC.c", cflags=[*cflags_game, "-inline auto"]),
                Object(Matching, "rw/fn_80450D6C.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_80451A6C.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_80451F48.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_80453018.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_804535C8.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_80453980.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_80454AA4.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_80455D0C.c", cflags=[*cflags_game, "-inline auto", "-pragma \"inline_max_size(1024)\""]),
                Object(NonMatching, "rw/fn_80458CF8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8045A760.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_8045AF28.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8045B6F0.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8045BF30.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8045CA38.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8045F2A0.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(Matching, "rw/fn_8045F9E8.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80460974.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804626F8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8046288C.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_8046335C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80463D28.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804641CC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80464814.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80464A94.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8046528C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80465CDC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80466B58.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8046ACBC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8046B110.c", cflags=[*cflags_game, "-inline noauto"]),
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
                Object(NonMatching, "rw/fn_8047CCF8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8047DB10.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8047DC90.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047E100.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047FA28.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8048053C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_80480C6C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804815F0.c", cflags=cflags_game),
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
                Object(NonMatching, "rw/fn_8048ADE0.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8048B6AC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8048D7C0.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8048D85C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049043C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80491AB4.c", cflags=cflags_game),
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
                Object(NonMatching, "rw/fn_8049B8BC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8049D408.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049EA2C.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049F2A4.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049F3F8.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049F8AC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_8049FAFC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804A076C.c", cflags=cflags_game),
                # The original calls fn_804A2F24 from fn_804A457C: the default limit inlines it, an
                # inline_max_size of 350 to 480 keeps the call.
                Object(Matching, "rw/fn_804A2C90.c", cflags=[*cflags_game, "-pragma \"inline_max_size(400)\""]),
                Object(NonMatching, "rw/fn_804A55CC.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_804A6CDC.c", cflags=cflags_game),
                Object(Matching, "rw/fn_804A869C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804A90F8.c", cflags=cflags_game),
                Object(Matching, "Game/BackDrop.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecCircuit.cpp", cflags=cflags_game_cpp),
                # All 42 functions are instruction-identical except seven displacements in the
                # constructor, which address the unit's .data. The original .data starts with 0x30
                # bytes (four base lists naming TNumEdit and
                # SonicteamUSA::System::CharaColli::CharaColliBase) that this source does not
                # produce, and the same classes also have 16 bytes of RTTI records in .sdata and
                # 0x40 bytes of name strings in .rodata. No code in the unit uses them and their
                # classes are unknown. The .sdata2 offsets also differ by 0x10, because the
                # candidate emits dead weak sqrtf doubles.
                Object(NonMatching, "Game/ElecCircuitEntrance.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecCircuitExit.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/TestNakazawa.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/BDTrue.cpp", cflags=cflags_game_cpp),
                # Every function, the data and all relocations match except ProcessHits, which is
                # one word short: the original branches twice on one comparison of the word at 0x50
                # of the character info (the second branch skips the cast check), and the single
                # natural test compiles to one branch fewer.
                Object(NonMatching, "Game/TornadoCollision.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecCristalWallSwitch.cpp", cflags=cflags_game_cpp),
                Object(NonMatching, "Game/ElecCube.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecPillar.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecSmallPillar.cpp", cflags=cflags_game_cpp),
                Object(Matching, "Game/ElecWakka.cpp", cflags=cflags_game_cpp),
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
                # are in this unit because they share its address range. 19 of its 53 functions are
                # sourced so far: ResolveProject, LoadProject, the Normalize* functions,
                # UnpackPatternBlend, PackPatternBlend, SplitFloat, SetEntryColors, TintEntryColors,
                # HoldKeyValue, InterpolateLinear, InterpolateColor and TransformPoint match (rodata
                # operands compared by target). SwapBytes differs by the register of the loaded
                # value in four instructions. InterpolateKeys differs in that the original saves its
                # first argument to pass it again in the default case. InterpolateHermite differs in
                # instruction order, registers and one extra address instruction in the original.
                # ExtractFileParts has the original's 102 words and differs in 13: the original
                # loads the file's first word after copying the six arguments to saved registers,
                # where this build loads it after the first copy and interleaves the rest with the
                # comparison (eight words), and the five allocation __LINE__ values (471, 480, 494,
                # 504 and 512 in the original).
                Object(NonMatching, "Game/CSD/csdLoader.cpp", cflags=cflags_csd),
                # Every function matches except InitMatrixStack, whose allocations pass lines 64 and
                # 65 instead of the original 53 and 54. .rodata also starts with the two static
                # doubles of MSL <cmath>'s sqrtf, unreferenced weak data that the link discards.
                Object(NonMatching, "Game/CSD/csdMatrix.cpp", cflags=cflags_csd),
                Object(Matching, "Game/CSD/fn_80360528.cpp", cflags=cflags_csd),
                Object(Matching, "Game/CSD/fn_80361A24.cpp", cflags=cflags_csd),
                Object(Matching, "Game/CSD/fn_80361CF8.cpp", cflags=cflags_csd),
                # Every function present matches except BuildTextEntry and InitPalette, which address
                # .rodata through one section base where the original addresses each literal on its
                # own (probably also the cause of their register and scheduling differences), and
                # whose allocations pass lower __LINE__ values than the original 442 and 234. MarkEntryDirty (0x803609D4) is not emitted and MarkMatrixUsers comes after its
                # caller: the original file has callers the game does not use (removed at link).
                Object(NonMatching, "Game/CSD/csdMotionPalette.cpp", cflags=cflags_csd),
                # Every function matches except LoadFile (its allocation passes __LINE__ 53 where
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
                Object(Matching, "Game/fn_80008D48.c"),
                Object(Matching, "Game/fn_80008DA4.c"),
                Object(Matching, "Game/fn_80008E1C.c"),
                Object(Matching, "Game/fn_800091D0.c"),
                Object(
                    Matching,
                    "Game/fn_803CC9D0.c",
                    extra_cflags=["-sdata 0", "-use_lmw_stmw on"],
                ),
                # CRI middleware units named after their original source files.
                Object(Matching, "Game/fn_80403054.c"),
                Object(Matching, "Game/fn_804030A8.c"),
                Object(Matching, "Game/fn_80403058.c"),
                Object(Matching, "Game/fn_8040307C.c"),
                Object(Matching, "Game/fn_8040350C.c"),
                Object(Matching, "Game/fn_80403E44.c"),
                Object(Matching, "Game/fn_80406EB0.c"),
                Object(Matching, "Game/fn_80406E88.c"),
                Object(Matching, "Game/fn_80406EBC.c"),
                # Target addresses the conversion constants through .rodata.
                Object(Matching, "Game/fn_80406F08.c", extra_cflags=["-sdata2 0"]),
                Object(Matching, "Game/fn_8040773C.c"),
                Object(Matching, "Game/fn_80406B70.c"),
                Object(Matching, "Game/fn_80407920.c"),
                Object(Matching, "Game/fn_80407930.c"),
                Object(Matching, "Game/fn_80068A3C.c"),
                Object(Matching, "Game/fn_804110EC.c"),
                Object(Matching, "Game/fn_804212F4.c"),
                Object(Matching, "Game/fn_8040E048.c"),
                Object(Matching, "Game/fn_8040AB94.c"),
                Object(
                    Matching,
                    "Game/fn_8040ABC8.cpp",
                    cflags=cflags_game_cpp,
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                Object(Matching, "Game/fn_8041FD6C.c"),
                Object(Matching, "Game/fn_8041FF08.c"),
                Object(Matching, "Game/fn_80420438.c"),
                Object(Matching, "Game/fn_80425858.c"),
                Object(Matching, "Game/fn_8040D8D4.c"),
                Object(Matching, "Game/fn_8040D850.c"),
                Object(Matching, "Game/fn_80424D18.c"),
                Object(Matching, "Game/fn_8040D9D8.c"),
                Object(Matching, "Game/fn_8040D498.c"),
                Object(Matching, "Game/fn_8040C244.c"),
                Object(Matching, "Game/fn_8040CC5C.c"),
                Object(
                    Matching,
                    "Game/fn_8040CC6C.cpp",
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                Object(
                    Matching,
                    "Game/fn_8040CCEC.cpp",
                    cflags=cflags_game_vector_length,
                ),
                Object(Matching, "Game/fn_8040CCD4.c"),
                Object(Matching, "Game/fn_8040D6F4.c"),
                Object(Matching, "Game/fn_8040E38C.c"),
                # SonicUSA::System::Random: the target keeps its doubles in .rodata and its RTTI record
                # and twist table in .data (-sdata 0, -sdata2 0). NextFloat and NextFloatInclusive
                # still differ: the target adds the m_state[i + 397] offset to an index register
                # (lwzx), while this build folds it into the load displacement.
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
                Object(Matching, "Game/fn_8040DB6C.c"),
                Object(Matching, "Game/fn_8041583C.c"),
                Object(Matching, "Game/fn_8040CB2C.c"),
                Object(Matching, "Game/fn_8040E310.c"),
                Object(Matching, "Game/fn_8040D608.c"),
                Object(Matching, "Game/fn_80411548.c"),
                # Size-optimized like the game C++ units: the target saves r30/r31 with stmw/lmw.
                Object(
                    Matching,
                    "Game/fn_8041157C.c",
                    extra_cflags=["-Cpp_exceptions on", "-O4,s", "-use_lmw_stmw on"],
                ),
                Object(Matching, "Game/fn_8040CF2C.c"),
                Object(Matching, "Game/fn_804202B8.c"),
                Object(Matching, "Game/fn_8040CD20.c"),
                Object(
                    Matching,
                    "Game/fn_8040CD60.cpp",
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                Object(
                    Matching,
                    "Game/fn_8040D02C.cpp",
                    extra_cflags=["-Cpp_exceptions on"],
                ),
                Object(
                    Matching,
                    "Game/fn_8040D044.cpp",
                    # The original saves its two callee-saved registers with stmw/lmw, which this
                    # unit's code only gets when optimized for size with -use_lmw_stmw.
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
                    cflags=cflags_game_vector_length,
                ),
                Object(
                    Matching,
                    "Game/fn_8040CF90.cpp",
                    extra_cflags=["-Cpp_exceptions on"],
                ),
            ]
            if config.version == "GUPE8P"
            else []
        ),
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
            Object(Matching, "dolphin/OS.c", cflags=cflags_sdk_rev2),
            Object(Matching, "dolphin/OSAlloc.c"),
            Object(Matching, "dolphin/OSExec.c", cflags=cflags_sdk_rev2),
            Object(Matching, "dolphin/OSFatal.c"),
            Object(Matching, "dolphin/OSFont.c"),
            Object(Matching, "dolphin/OSLink.c"),
            Object(Matching, "dolphin/OSMutex.c"),
            Object(Matching, "dolphin/OSThread.c"),
            Object(Matching, "dolphin/OSTime.c"),
        ],
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
            Object(Matching, "dolphin/CARDCreate.c"),
            Object(Matching, "dolphin/CARDRead.c"),
            Object(Matching, "dolphin/CARDWrite.c"),
            Object(Matching, "dolphin/CARDDelete.c"),
            Object(Matching, "dolphin/CARDStat.c"),
            Object(Matching, "dolphin/CARDUnlock.c"),
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
            # These three were built with signed char (the code sign-extends char values)
            Object(Matching, "dolphin/dvdfs.c", extra_cflags=["-char signed"]),
            Object(Matching, "dolphin/dvd.c"),
            Object(Matching, "dolphin/dvdqueue.c"),
            Object(Matching, "dolphin/dvdidutils.c", extra_cflags=["-char signed"]),
            Object(Matching, "dolphin/dvdFatal.c"),
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
            # Built without instruction scheduling (code stays in source order)
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
        "odemustubs",
        [
            # Older compiler (epilogue scheduling) with deferred inlining: functions come out in
            # reverse source order, and none are auto-inlined
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
        "progress_category": "sdk",
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
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/file_io.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/FILE_POS.c"),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/Src/locale.c"),
            # no auto-inlining: the original keeps is_utf8_complete out of line
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
            Object(Matching, "dolphin/GXInit.c", cflags=cflags_sdk_rev2),
            Object(Matching, "dolphin/GXFifo.c", cflags=cflags_sdk_rev2),
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
    ),
    {
        # CRI middleware units that are identical and split in every region.
        "lib": "CRI",
        "mw_version": config.linker_version,
        "cflags": cflags_game,
        "progress_category": "game",
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
            # Deferred inlining: the table generators are emitted in reverse order.
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
            Object(
                Matching,
                "CRI/sfd_tmr.c",
                extra_cflags=[
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
            Object(
                Matching,
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
            # Deferred inlining: MWCC emits the functions (and lays out .bss) in reverse order.
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
                "CRI/adx_fini.c",
                extra_cflags=["-sdata 0", "-sdata2 0", "-str readonly"],
            ),
            Object(Matching, "CRI/adx_fcch.c", extra_cflags=["-sdata 0"]),
            # Only functions declared inline are expanded (-inline noauto).
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
            Object(
                Matching,
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
            # Deferred inlining: MWCC emits the functions (and lays out .bss) in reverse order.
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
            # -inline noauto: only the helpers declared inline are expanded.
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
            # Deferred inlining: MWCC emits the functions (and lays out .bss) in reverse order.
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
            # Deferred inlining: MWCC emits the functions (and lays out .bss) in reverse order.
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
            # Deferred inlining: MWCC emits the functions (and lays out .bss) in reverse order.
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
            Object(
                Matching,
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
            Object(
                Matching,
                "CRI/sfd_hds.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
                "CRI/sfd_pts.c",
                extra_cflags=[
                    "-sdata 0",
                    "-sdata2 0",
                    "-str readonly",
                    "-use_lmw_stmw on",
                ],
            ),
            Object(
                Matching,
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
            Object(
                Matching,
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
            Object(
                Matching,
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
            # Deferred inlining: MWCC emits the functions (and lays out .bss) in reverse order.
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
            Object(Matching, "CRI/sj_crs.c", extra_cflags=["-sdata 0"]),
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
            Object(Matching, "CRI/sj_uni.c", extra_cflags=["-sdata 0"]),
            Object(Matching, "CRI/sj_utl.c", extra_cflags=["-use_lmw_stmw on"]),
            Object(Matching, "CRI/sj_err.c"),
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


# Optional extra categories for progress tracking
# Adjust as desired for your project
# Categories cover only the configured source units; original code without a configured unit
# belongs to no category, so the labels say so.
config.progress_categories = [
    ProgressCategory("game", "Game Code (configured units)"),
    ProgressCategory("sdk", "SDK Code (configured units)"),
]
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
