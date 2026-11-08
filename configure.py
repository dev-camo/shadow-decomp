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
                Object(NonMatching, "Game/fn_8043BD80.c", cflags=cflags_game_gcax),
                Object(NonMatching, "rw/fn_80443A74.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804448B8.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80445CF4.c", cflags=cflags_game),
                # Two functions differ. The hierarchy create function fn_804472AC: 7 of its 91
                # words, and it saves r26-r31 (its two relocations name _savegpr_26/_restgpr_26
                # instead of _savegpr_27/_restgpr_27) because maxInterpKeyFrameSize and the new
                # hierarchy get separate registers where the original shares r31. The hierarchy
                # attach function fn_804474D8: 5 of its 41 words (the original copies the node
                # index's zero into the node offset and keeps the index in r31).
                Object(NonMatching, "rw/fn_80446A04.c", cflags=cflags_game),
                # Matching workaround: -inline noauto; see the comment on rw/fn_80440410.c.
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
                Object(NonMatching, "rw/fn_80451F48.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80453018.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80453980.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8045B6F0.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8045CA38.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8046335C.c", cflags=cflags_game),
                # fn_804649C0 and the data match. fn_80464814 differs in 28 of 107 instruction
                # words, all register numbers (opcodes, relocations and stack offsets are the
                # original's): the two component sizes, start and end take r29/r31/r30 instead
                # of r31/r30/r29, and r5-r8 are permuted among short-lived values (the two GQR6
                # settings, the morph target array, the position type and two table addresses).
                Object(NonMatching, "rw/fn_80464814.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80464A94.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80465CDC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80466B58.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8046B110.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8046D8C8.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8046EE8C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80470AC4.c", cflags=cflags_game),
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
                # The line reader (fn_8047A518) is 79 words against the original's 78 (316 bytes
                # instead of 312). Counting differing words at the same position (and, in
                # parentheses, rows that differ once the instruction streams are aligned): 76 of 78
                # (18), because the extra word at 0x08 shifts every later word. maxLen is copied to
                # r7 because the buffer argument move is scheduled before maxLen - 1, and buffer and
                # file take r31 and r29 where the original has r29 and r31.
                Object(NonMatching, "rw/fn_80479A50.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047A7C8.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8047B720.c", cflags=cflags_game),
                # The orthonormalizer (fn_8047E298) retains 21 register-word differences: the
                # floating-point registers of the axis dot products that pick the pivot pair.
                Object(NonMatching, "rw/fn_8047E100.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8048222C.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80482BC4.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804842C8.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_80487914.c", cflags=cflags_game),
                # USA raw differing words: fn_8048A178 16 (eight first-block instructions in
                # another order: lis, the y shift, a store, two lfd, the compare, fsubs and fmuls;
                # registers and operands identical), fn_8048A790 40 (non-volatile register
                # assignment r20-r31 only; its inlined copies in fn_8048A96C match).
                Object(NonMatching, "rw/fn_8048A178.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_8048D85C.c", cflags=cflags_game),
                # fn_80496408 differs in 7 words: its error strings are addressed from a .data
                # base 0x74 bytes below the original's. The original loads 0x8056E4F8 (fn_80495428's
                # jump table, 0x74 bytes after fn_804951B8's at 0x8056E484) as its base. That suggests
                # the original file's .data starts there and fn_804951B8 belonged to another file
                # (inferred, not established).
                Object(NonMatching, "rw/fn_804951B8.c", cflags=cflags_game),
                # fn_8049D934 differs in 10 words: in the duplicate check of the clusters of interest
                # the index, its 12-byte offset and the hoisted count take r7/r5/r6 instead of
                # r5/r6/r7. All other functions and the data match.
                Object(NonMatching, "rw/fn_8049D408.c", cflags=cflags_game),
                # Matching workaround: -inline noauto; at the default setting .text is 0x1738 bytes,
                # with the flag 0x1710 like the original's. Revisit when the unit's real inline
                # settings are found.
                Object(NonMatching, "rw/fn_804A55CC.c", cflags=[*cflags_game, "-inline noauto"]),
                Object(NonMatching, "rw/fn_804A6CDC.c", cflags=cflags_game),
                Object(NonMatching, "rw/fn_804A90F8.c", cflags=cflags_game),
                # All 35 functions have identical instructions. The unit's own data and its order
                # are the original's; the accessor of the switch counter is emitted after the
                # template instances that follow ApplyState here, directly after ApplyState in the
                # original.
                Object(NonMatching, "Game/CandleStick.cpp", cflags=cflags_game_cpp),
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
                # 81 of the 85 functions have identical instructions. The preload hook differs in 5
                # words (the registers of the table pointer and the stage number), SetupPath in 2
                # (the order of two argument moves), ResetFromOwner in 7 (the original calls the
                # smart pointer's operator-> out of line for the yaw of the character info) and
                # UpdateBody in 6 (the order of two argument moves and the register of the stage
                # pointer). The RTTI and vtable records differ in layout, and the object has
                # additional weak template instances.
                Object(NonMatching, "Game/Coaster.cpp", cflags=cflags_game_cpp),
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
                # All 14 functions have identical instructions; .data, .rodata and .sdata match and
                # .sdata2 is 4 bytes short of the original's trailing pad. The unit cannot replace
                # the original object: GetTaskGroup is local here, where the original's is
                # inferred to be a weak function with weak statics (seven functions in other units
                # call it and its statics lie outside the unit). An inline definition is expanded
                # into Create, and a global one keeps its statics local to the unit.
                Object(NonMatching, "Game/RainEffect.cpp", cflags=cflags_game_cpp),
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
                # All 11 functions have identical instructions, and the whole object matches once
                # the 14 weak and local copies the linker discards are set aside, except the order
                # of the nine floats in .sdata2 (0x18-0x38): the original pools 70.0f, 1.0f, 0.1f,
                # 0.6f, 0.0f, 0.3f, 0.7f, 1.2f, 1.5f; this build pools 0.3f, 0.7f, 1.2f, 1.0f,
                # 1.5f, 0.0f, 0.1f, 0.6f, 70.0f (UpdateRotation's order of first use), so the
                # .sdata2 relocations of the functions point at other offsets.
                Object(NonMatching, "Game/ElecCube.cpp", cflags=cflags_game_cpp),
                Object(NonMatching, "Game/GrindGadget.cpp", cflags=cflags_game_cpp),
                # 11 of 12 functions have identical instructions. UpdateBody differs in 8 words, all
                # register numbers in the case that moves the footing back (the hidden depth and the
                # height are in f2 and f0 here, f0 and f1 in the original).
                Object(NonMatching, "Game/PopFooting.cpp", cflags=cflags_game_cpp),
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
                # are in this unit because they share its address range. All 53 of its functions are
                # sourced (the RCPtr<CTexList> members from 0x8035EC38 are template instances). The
                # RenderState functions, ResolveProject, LoadProject, the Normalize* functions,
                # FindSceneNameIndex, ApplyScreenAspect, DrawScene, DrawLayer, DrawElement,
                # UnpackPatternBlend, PackPatternBlend, SplitFloat, SetEntryColors, TintEntryColors,
                # HoldKeyValue, InterpolateLinear, InterpolateColor, TransformPoint, UpdatePalette,
                # UpdateEntries, UpdateEntry, EvaluateChannels, UpdateEntryTransform, RCPtr::Reset
                # and the constructor of its holder match (rodata operands and the holder's vtable
                # compared by target). SwapBytes differs by the register of the loaded value in four
                # instructions. InterpolateKeys differs in that the original saves its first
                # argument to pass it again in the default case. InterpolateHermite differs in
                # instruction order, registers and one extra address instruction in the original.
                # ExtractFileParts has the original's 102 words and differs in 13: the original
                # loads the file's first word after copying the six arguments to saved registers,
                # where this build loads it after the first copy and interleaves the rest with the
                # comparison (eight words), and the five allocation __LINE__ values (471, 480, 494,
                # 504 and 512 in the original). DrawQuadElement has the original's 111 words and
                # differs in 11 (the filter value and the value passed to SetField0x24 have each
                # other's saved registers, and one load is placed differently). DrawTextElement (152
                # words, the original's 165) copies the corners of a glyph without texture with a
                # different address computation (the original multiplies the vertex index by the
                # vertex size for the x and y of corners 1 to 3), plus register and scheduling
                # differences. EvaluateTrack (168 words, the original's 187) calls PackPatternBlend
                # in case 6 where the original expands it in place (and first copies the
                # interpolated value to a second stack slot); outside case 6 it differs only in
                # branch displacements, stack frame offsets (the original's frame is 16 bytes
                # larger) and the register of one comparison operand. Counting differing words at
                # the same position (and, in parentheses, the original's words outside the runs that
                # still match once the instruction streams are aligned): SetEntryPattern (317 words,
                # the original's 319) differs in 201 (105): the original copies the vertices that
                # change layout word by word through integer registers and addresses some stores
                # differently. The DeleteRelease RCPtr::Assign has the original 120-word size and
                # differs only in the holder allocation line: 147 in csdRCObject.h versus native
                # 410. Reset and the holder's constructor come after RenderState::End, as in the
                # original; the out-of-line CMemoryAlloc::AllocObject that the holder allocation
                # emits after them is a weak copy of a function the game has at 0x8003DF84.
                # Additional weak copies include the CTexList holder and CFunctorBase destructors,
                # the functor vtable, default RCPtr constructor, no-policy Assign, holder
                # constructor, and RCPtr<CTexList> operator= (0x30) and counted-reference Assign
                # (0x1AC). Source .data is 28 bytes; the original has none. Normal links use the
                # original object.
                Object(NonMatching, "Game/CSD/csdLoader.cpp", cflags=cflags_csd),
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
                # constructor differs only in passing line 141 for line 70. InitLibraryRW (139
                # words) differs in 25 only by the .bss offsets of its function-local statics: the
                # original has 12 more bytes before the first record. The holder's Assign (at
                # 0x8036342C, 120 words) differs only in its holder allocation line, 147 versus
                # native 410 in csdRCObject.h. Weak copies of the allocator's default slots and of
                # the CTexList, CMemoryAlloc and RCObject vtables are also emitted here. .data
                # differs as well: the vtables come out in a different order from the original's.
                # .rodata differs too (0x9D bytes, the original's 0xA8): the
                # original aligns the assertion text and the empty string to 8 bytes. Matching
                # workaround: -inline auto,deferred,level=1 here, while the rest of Chao::CSD uses
                # -inline noauto. The original expands the TexArray constructor and destructor in
                # CTexListRW::Load and ~CTexListRW and also keeps their out-of-line copies; -inline
                # noauto cannot produce both. Revisit if the original's flags or an inline
                # declaration that reproduces both are found. Further weak copies include the
                # CTexList and byte holder/CFunctorBase destructors, their tables, and the byte
                # pointer's default constructor, no-policy Assign and holder constructor. Source
                # .data is 664 bytes versus 544 native bytes. Normal links use the original object;
                # the extra copies are not selected by those links.
                Object(
                    NonMatching,
                    "Game/CSD/csdTexListRW.cpp",
                    cflags=[*cflags_csd, "-inline auto,deferred,level=1"],
                ),
                # The texture list over an existing RenderWare texture dictionary (file name
                # unknown). 6 of its 7 functions match in raw instruction words; the holder's Assign
                # (120 words) differs only in allocation line 147 versus native 410. The constructor
                # and destructor of the texture array are called out of line, as in the original.
                # The constructors of CTexListRW2, CTexListRW and CTexList and the allocator's
                # AllocObject and FreeObject are also emitted as weak copies, as are the CTexList
                # and RCObject vtables. The byte pointer's default constructor, no-policy Assign and
                # holder constructor are also emitted, along with byte and CTexList
                # holder/CFunctorBase destructors and tables. Source .data is 128 bytes versus 40
                # native bytes. Normal links use the original object.
                Object(NonMatching, "Game/CSD/fn_80364058.cpp", cflags=cflags_csd),
                # The accessors that override the animated values of an element's entry
                # (CMotionPattern), its constructor and Copy, and UpdateTransform. 32 of 32
                # functions are sourced. 15 have identical instructions; 13 differ only in the
                # __LINE__ passed by their allocations (one word each, two in UpdateTransform; the
                # original's lines run from 54 to 1067 in a file far longer than this one). The
                # holder Assign for the free and function release policies has the original
                # 115-word size and differs only in allocation line 147 versus native 410.
                # GetTranslation and GetScale (49 words, the original's 45) copy their result
                # through the float registers where the original uses the integer ones. The .rodata
                # holds the same four strings in a different order (the original has the holder's
                # error text first). The unit also emits weak copies of the CMotionPattern
                # destructor (identical to the game's copy, which comes from csdmScene.cpp),
                # RCObject<unsigned char>::SetRelease, CMemoryAlloc::AllocObject and
                # CMemoryAlloc::FreeObject and of
                # the RCObject vtables for unsigned char and MatrixValue. Its .bss holds the free
                # release policy as one byte, where the game's symbol spans the 8 bytes up to the
                # next one. The MatrixValue pointer's default constructor, no-policy Assign and
                # holder constructor are also weak copies, as are the byte and MatrixValue
                # holder/CFunctorBase destructors and tables. Source .data is 72 bytes versus 16
                # native bytes. Normal links use the original object; the extra copies are not
                # selected by those links.
                Object(NonMatching, "Game/CSD/csdmMotionPattern.cpp", cflags=cflags_csd),
                # The units below that use -Cpp_exceptions on are those whose original object
                # carries extab/extabindex entries (C++ exception tables); the default is off.
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
                # Support code of the same library after the quaternion helpers: the byte reversal
                # (two functions that do not match yet). The start-up and shut-down wrappers with a
                # name registry before it (Game/fn_80420C30.c) and the ARAM transfers and allocator
                # after it (Game/fn_804211CC.c) are in the all-region list below.
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
            Object(
                NonMatching,
                "Game/fn_8040C874.cpp",
                cflags=cflags_game_cpp,
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
            # Weak byte-holder and CFunctorBase destructors and their 12-byte and 16-byte tables
            # are also emitted. They shift the CTexList table: source .data is 66 bytes versus
            # 40 native bytes, and the object comparison's matched-data count is 8 bytes rather
            # than 48. Normal links use the original object.
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
            # function release policy has the original 115-word size and differs only in
            # allocation line 147 versus native 410. GetPattern retains seven positional setup/load
            # scheduling differences, including an equivalent palette-pointer base register.
            # The holder's SetRelease matches but comes last in the unit, where the original
            # has it right after the holder constructor for the function policy (see csdRCObject.h).
            # The source also emits weak copies of RCPtr<unsigned char>::Reset, the holder
            # constructor for the delete policy and CMemoryAlloc::AllocObject, plus a local
            # delete-policy Assign referenced only by that source Reset. It also emits a weak
            # RCObject<unsigned char> vtable (the original unit has no .data). Normal links
            # select the original unit and its existing providers; these links do not include
            # this source object or establish which source weak copies would be discarded.
            # Additional weak copies include byte and MatrixValue default constructors,
            # no-policy Assign and holder constructors, plus their holder/CFunctorBase destructors
            # and tables. Source .data is 56 bytes; the original has none.
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
            # Scene objects, playback, node lookup and non-owning listeners. Of the 195 native
            # bodies, this source emits 166 identified counterparts. The 157 instruction-identical
            # bodies include two tree methods that call the key comparator instead of distinct
            # value_compare wrappers. The source expands those wrappers into their callers; the
            # original keeps separate copies and emits the key comparator after the first wrapper.
            # Five destructor models remain provisional, and five
            # operations are expanded in headers. Remaining helpers are external to this object;
            # five are emitted by Node. Unidentified callback/functor tables keep placeholders.
            # The nested CFunctorBase name comes from the original name pool; its association with
            # the holder's +0x0C pointer and CBase inheritance remain inferred. Matching deleting
            # instructions and table/destructor references alone do not identify fn_80367FB4,
            # fn_80364BE4, fn_80365F28, lbl_8055FE98 or lbl_8055FEA8 in USA (GUPE8P).
            # Nine bodies differ in allocation lines, pooled constants, indexed addressing,
            # iterator accessors, return-object stores or the final repeated motion-index copy.
            # Scene initialization passes allocation line 128 rather than native 173; root-pattern
            # allocations pass 486/473 rather than 1124/1102.
            # Definitions with established native identities follow descending addresses; callback
            # destructor placement is provisional. Among the 166 paired bodies, 129
            # retain native relative order. Generated helpers still differ: map traversal helpers
            # precede the font-handle copy constructor, while callback-list and map/reference
            # destruction helpers follow the scene destructor instead of their earlier native
            # positions. Tree find/insert helper order and the reversed GetSender/destructor
            # adjustment thunks also remain. The MatrixValue default reference constructor and its
            # assignment/holder helpers follow CMotionPattern's copy constructor as in the original.
            # The first 0x58 .data bytes and 0x4C .rodata bytes match with their relocations; this
            # includes the member-pointer constants used by the now-identical frame update.
            # Node lookup emits both 64-byte pair constructors and has the original 372-byte size;
            # its remaining instruction difference is allocation line 420 versus native 974.
            # Source .data is 448 bytes versus 464 native bytes. One unused direct member pointer
            # is unemitted; two native 28-byte zero ranges remain unclassified. The provisional
            # callback-holder table has a four-byte native zero tail. Read-only data is 130 bytes
            # before alignment versus 136 native bytes. The one-byte policy object does not identify
            # the remaining seven native BSS bytes. Extra weak reference/functor copies remain.
            # A source link with Node, without Project, lacks the constructor helpers fn_80364F30
            # and fn_8036528C. Normal links retain the original Scene object in every region.
            Object(NonMatching, "Game/CSD/csdmScene.cpp", cflags=cflags_csd),
            # The complete node source, including its constructor, destructor, callbacks, channel
            # map and owned data. The original translate-X/Y cases read the pair directly; the
            # other 13 call outlined iterator and reference arrows. Ten cases preserve the original
            # calls and operations. Rotation, pattern and the three value cases expand the arrows
            # to six words instead of four, adding eight bytes per case. All 97 native functions
            # are emitted, with 95 identical instruction bodies. In USA (GUPE8P), Japan (GUPJ8P)
            # and PAL (GUPP8P), the switch is 760 bytes versus 720, with 128 differing positional
            # words and 10 extra words. Besides the five expanded cases and default/epilogue branch
            # displacements, the prologue load order differs: the original saves LR and r31 and
            # runs mr r31,r3 before lwz r5,0(r4); the source puts the load at +0x8, before
            # stw r0,52(r1). The constructor differs in its allocation line (45 versus 51).
            # The callback reference destructor has the original 0x198-byte body. The object also
            # emits a 0x20-byte weak typed-pointer remove copy, whose native definition is in Scene
            # at 0x803655D0 in USA, and a 0x1C-byte weak RCPtr<CNode>::operator-> copy that the
            # original object references externally. Both member pointers, all three vtable groups
            # and the literal contents are represented. The switch-table addends differ, and a
            # weak byte-holder vtable shifts the element-data-reference vtable.
            # Function order also differs: the map destruction group (__destroy, deallocate,
            # __node_alloc, allocator destroy and __alloc) follows the tree destructor, ahead of
            # root(), instead of following fn_8036B9FC. The pair destructor moves into that group
            # from its original position before ClearHidden. The element-data-reference destructor
            # follows root(), rather than the node destructor. Map and callback-list constructors
            # precede Attach/Clear instead of following them. fn_8036B9FC follows Copy instead of
            # the callback compressed-pair constructors. The two adjustment thunks are reversed:
            # GetSender precedes the destructor. Extra weak reference/container copies and their
            # support data are also emitted. A local, non-weak 0x1CC delete-policy Assign is reached
            # only by the weak RCPtr<unsigned char>::Reset. Normal links retain the original object
            # in every region. Weak holder/CFunctorBase destructors and tables are emitted for
            # CNode, MatrixValue, CNodeCallback, unsigned char and UnkNodePayload_80364C40. Byte and
            # payload default constructors, no-policy Assign and holder constructors are also
            # emitted. The source object's .data is 292 bytes versus 152 native bytes, before link
            # selection of weak copies.
            Object(NonMatching, "Game/CSD/csdmNode.cpp", cflags=cflags_csd),
            # Project resources, scene/collection creation, named and ordered indexes, queued
            # changes and listener cleanup. USA (GUPE8P), Japan (GUPJ8P) and PAL (GUPP8P) each
            # have 169 emitted counterparts for 178 native functions; 151 instruction streams
            # are identical. Nine iterator and comparator operations have no separate
            # emitted copy. Five of the 151 bodies make seven calls to SceneNameCompare where the
            # original calls distinct value_compare wrappers (USA functions 8036E5F4, 8036E6CC,
            # 8036EDA4, 8036F580 and 8036F688). For example, 8036E5F4+0x48 calls native 8036E678;
            # the source expands value_compare and calls the key comparator directly. The
            # original emits its comparator immediately after the first separate wrapper.
            # In 8036D0BC and 8036F4F0, three calls also use local weak const-key pair destructors
            # instead of the original external definitions at 800418A4 and 80041B70 in USA.
            #
            # Differing bodies: USA addresses identify the corresponding functions in all regions.
            # Sizes and positional counts below agree in each region. Different words are counted
            # at equal positions in the overlap; tail words are unpaired by length, not an aligned
            # instruction-edit distance. Allocation lines, inlining and stack/register choices
            # remain. LoadFile combines the original cmplwi and mr into one mr. instruction;
            # in 8036C318 the source loads before two prologue stores instead of after them.
            # In fn_8036C8E0, both versions construct a null RCPtr<CScene> and independently
            # null-check each holder, reading the scene's object before the temporary's. The
            # original saves the comparison result before destroying the temporary. The source
            # saves both object pointers and compares them afterward. Iterator loads and the
            # prologue also differ.
            # Definitions follow descending established original addresses. Of the 169 paired
            # bodies, 111 retain original relative order; generated helper order still differs.
            # USA address  Native bytes  Source bytes  Different words  Native tail  Source tail
            # 8036BCAC        312           312               1            0            0
            # 8036BE04        480           480               1            0            0
            # 8036C0C4        388           388               1            0            0
            # 8036C318         84            84               3            0            0
            # 8036C46C        248           248               1            0            0
            # 8036C8E0        412           400              56            3            0
            # 8036D054        104           136              22            0            8
            # 8036D2FC        276           276               1            0            0
            # 8036D448        180           180               1            0            0
            # 8036D598        200           200               1            0            0
            # 8036DD44        212           212               1            0            0
            # 8036DF18        460           460               1            0            0
            # 8036E114        648           644             121            1            0
            # 8036E3A8        176           176               1            0            0
            # 8036E458        308           320              47            0            3
            # 8036ECF0        176           176               1            0            0
            # 8036EE74        176           176               1            0            0
            # 8036F470        128           128               4            0            0
            #
            # The source keeps weak copies of the SceneList-handle destructor, raw Scene-pointer
            # reset, Node-handle, callback-list and scene-node-map constructors, and the begin/end
            # pairs for the removal and order-change lists. The original callback-list and
            # scene-node-map constructors belong to Scene; the other seven definitions are outside
            # these CSD files. For example, original removal processing at 8036C3AC calls external
            # list begin/end at 8003EB64/8003EB28 in USA; this source keeps local weak copies.
            # The Project destructor also calls local weak order-change-list, float-multimap and
            # name-multimap destructors instead of the external definitions at 8003E874, 8003EF90
            # and 8003F0A8 in USA. Each immediate destructor has the original 76-byte instruction
            # shape after masking branch displacements; the different call targets remain.
            # The weak SceneList release helper has the original 328-byte body and direct calls;
            # the combined source link selects its external definition at 80042B10 in USA. The
            # implicit CSceneList destructor emits a 92-byte weak copy; the linked definition
            # remains the original external one at 80042CB4 in USA.
            #
            # Weak copies include the nested holder CFunctorBase destructors and tables for byte,
            # Scene, SceneList, CTexList, CNode, SceneCallback and UnkNodePayload references.
            # The pointer association and CBase inheritance remain inferred as in csdRCObject.h;
            # these emitted names do not identify the unnamed native tables. Holder allocations
            # pass line 147, including the explicit DeleteRelease Reset overload.
            # Inline reference-copy constructors and assignment operators call the inferred
            # counted-reference helpers directly. Their weak copies have the original bodies and
            # call targets; the combined source link retains the original external definitions.
            # The combined source link includes this file's real CScene constructor, which emits
            # the scene-node-map and callback-list constructor helpers needed by Scene/Node.
            # Strings, table slots and policy values are represented; pooling, order, alignment
            # and table selection differ. The combined scene/node/project source link has a
            # 120-byte Project-to-next-object data interval, including six alignment bytes after
            # its 114-byte named extent, versus the native 104-byte span. Two locally kept
            # 12-byte holder tables have native definitions outside Project. BSS contains seven
            # one-byte empty-object temporaries at four-byte strides. After their 28-byte slot
            # span, alignment to the following 32-byte-aligned OS.c BSS adds 12 bytes in USA and
            # four in Japan/PAL.
            # The normal link retains the original project object.
            Object(NonMatching, "Game/CSD/csdmProject.cpp", cflags=cflags_csd),
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
            # Matching workaround: -inline noauto on fn_80440410.c and fn_8044BAB4.c. With the
            # library default fn_80440D58..fn_80440E80 and fn_8044C56C..fn_8044CD4C grow (e.g.
            # fn_80440E80 0x198 -> 0x404, fn_8044C5FC 0x158 -> 0x1F8); the other units from
            # fn_80442B98.c through fn_80451A6C.c build to the same code and data with or
            # without the flag. Revisit when the original unit flags are known.
            Object(Matching, "rw/fn_80440410.c", cflags=[*cflags_game, "-inline noauto"]),
            Object(Matching, "rw/fn_80442B98.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80443044.c", cflags=cflags_game),
            Object(Matching, "rw/fn_804447B0.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80447598.c", cflags=cflags_game),
            Object(Matching, "rw/fn_804485BC.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80448CB0.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8044920C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80450D6C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80451A6C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_804535C8.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80454AA4.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80455D0C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80458CF8.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8045A760.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8045AF28.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8045BF30.c", cflags=cflags_game),
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
            Object(Matching, "rw/fn_80463D28.c", cflags=cflags_game),
            Object(Matching, "rw/fn_804641CC.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8046528C.c", cflags=cflags_game),
            # The size and preparation helpers match; the vertex-byte counter retains twelve
            # register-word differences in its normal-attribute case.
            Object(NonMatching, "rw/fn_8046ACBC.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8046FC1C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80470474.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80473E9C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80479040.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80479598.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8047CCF8.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8047DB10.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8047DC90.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8047FA28.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8048053C.c", cflags=cflags_game),
            Object(Matching, "Game/fn_8000AAEC.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80014E04.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8006127C.cpp", cflags=cflags_game_cpp),
            Object(Matching, "rw/fn_80480C6C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_804815F0.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80482984.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80483444.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8048349C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80483554.c", cflags=cflags_game),
            Object(Matching, "rw/fn_804836EC.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80483AFC.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80485BCC.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80486888.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80489B2C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8048ADE0.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8048B6AC.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8048D7C0.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8049043C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_80491AB4.c", cflags=cflags_game),
            Object(Matching, "rw/fn_804939DC.c", cflags=cflags_game),
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
            Object(Matching, "rw/fn_8049EA2C.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8049F2A4.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8049F3F8.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8049F8AC.c", cflags=cflags_game),
            Object(Matching, "rw/fn_8049FAFC.c", cflags=cflags_game),
            # The material-copy callback retains seven register-word differences.
            Object(NonMatching, "rw/fn_804A076C.c", cflags=cflags_game),
            # Matching workaround: the original calls fn_804A2F24 from fn_804A457C; the default
            # limit inlines it, an inline_max_size of 350 to 480 keeps the call. Revisit when the
            # unit's real inline settings, or a source shape that keeps the call at the default
            # limit, are found.
            Object(Matching, "rw/fn_804A2C90.c", cflags=[*cflags_game, "-pragma \"inline_max_size(400)\""]),
            Object(Matching, "rw/fn_804A869C.c", cflags=cflags_game),
            Object(Matching, "Game/fn_80015954.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80160B18.cpp", cflags=cflags_game_cpp),
            # Part cleanup retains one extra element-address addi and register differences.
            Object(NonMatching, "Game/fn_80162804.cpp", cflags=cflags_game_cpp),
            # Only the constructor differs (239 words against 240): the model and crash-information
            # loads before the first fn_802087D8 call are swapped, and the model pointer is loaded
            # inline where the original calls the 8-byte accessor fn_800E590C. That accessor is not
            # emitted, so the source object cannot be linked; normal builds link the original.
            Object(NonMatching, "Game/BreakableOrnament.cpp", cflags=cflags_game_cpp),
            # The constructor retains inlining differences; UpdateBody lacks one switch branch.
            Object(NonMatching, "Game/fn_800E59F8.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BreakObj.cpp", cflags=cflags_game_cpp),
            # UpdateBody lacks one branch of the state switch: the original's compare tree has an
            # extra blt.
            Object(NonMatching, "Game/Cage.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/CaptureCage.cpp", cflags=cflags_game_cpp),
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
            # 82 of the 85 functions have identical instructions and relocations once the weak and
            # local copies the linker discards are removed. The data matches by contents in an
            # object-relative comparison that sets those copies and the alignment aside; a
            # source-enabled link places the unit's data differently (in USA its .data, .rodata and
            # .sdata take 0x204, 0x3E4 and 0x9C bytes against 0x1EC, 0x2BC and 0x5C). Search
            # (0x801540F0) differs in 3 words (f2 and f3 swapped for 1 - fraction and 25, and the
            # operand order of one fmuls) and Sweep (0x80153CE8) in 5 words (f1 and f2 swapped for
            # the range and the speed product, and the operand order of two fmuls). UpdateBody is
            # two instructions longer (182 raw words differ by position): twice the original passes
            # the result of a copy constructor straight to fn_8040CF2C, where this build copies
            # into a local and passes its address.
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
            # Matching workaround: -inline auto,deferred,level=1 for this unit only (cflags_game_cpp
            # has -inline noauto). The original expands fn_8015A858 and fn_8015A818 (owner load,
            # null test, call to fn_800C9A64 or fn_800C996C) into Gadget::IsFinished and still
            # emits both as functions, which BreakableOrnament::IsFinished calls; with -inline
            # noauto they stay calls (IsFinished is then 0x70 bytes against 0xAC). Revisit if the
            # original's flags or an inline declaration that reproduces both are found.
            Object(
                Matching,
                "Game/Gadget.cpp",
                cflags=[*cflags_game_cpp, "-inline auto,deferred,level=1"],
            ),
            Object(Matching, "Game/fn_8015AA84.cpp", cflags=cflags_game_cpp),
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
            # Relocatable comparison with the split original object (USA, Japan and PAL alike; data
            # by contents, with the definitions the link discards set aside: 70 weak definitions
            # that the link does not take from this object and 21 unreferenced local ones): 61 of
            # the 63 functions have the original's instructions and relocation targets. The original
            # keeps the constructor of fn_80209B58's parameter block (0x80145B58, USA) out of line
            # after Drop; this build expands it in Drop, so that function is 62 words against 49 (by
            # position, 41 of the 49 common words differ and 13 build words are unpaired; aligned by
            # raw words, 43 pairs are equal, 6 words are only in the original and 19 only in this
            # build; its relocations add two Fn8040Vector3 copy constructor calls and two
            # lbl_805E07A0 references in place of the one constructor call), the constructor has no
            # counterpart, and in .sdata2 the literals 2.0f and 4.0f are in the opposite order
            # (0x805F45D0 and 0x805F45D4, USA). The rest of .rodata, .data, .sdata and .sdata2 has
            # the original's contents. The compiled .sdata2 is writable (WA) where the original's is
            # read-only (A), and it is 0x44 bytes without the original's section-end alignment to
            # 0x48.
            Object(NonMatching, "Game/EggBalloon.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Footing.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Elevator.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/FootingMovable.cpp", cflags=cflags_game_cpp),
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
            # is 115 words against the original's 116, with 67 positional raw word differences
            # and one unpaired word: the original creates the transform's result before the scaled
            # and offset vectors it is computed from, as a nested expression does, but loads the
            # line pointer between them, which the direct member assignments here do not
            # reproduce. Two placement differences remain: the tree's __destroy and the
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
            Object(Matching, "Game/PoisonGas.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BAGunShip.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BAMine.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BAShield.cpp", cflags=cflags_game_cpp),
            # 81 of the 83 functions of the original split object are identical in every word and
            # relocation (local names resolved; USA, Japan and PAL alike; addresses below are
            # USA's). Counts: positional = word i against word i; aligned = instructions matched
            # up, as paired-differing, original-only and source-only words.
            # - Balloon::OnCommand (0x800E6AF0), 35 words each: the original copies the command
            #   pointer to r31 inside case 0x14, after the m_type test (0x800E6B18); the compiled
            #   code copies it in the prologue. Positional: 7 words differ. Aligned: 1
            #   original-only and 1 source-only word (the moved copy) and 1 paired-differing word:
            #   the beq into the case keeps its word, but it and the lwz it reaches sit one word
            #   later here, and the b past the case keeps its target, but its displacement is one
            #   word shorter here; which of the two pairs as differing depends on whether branch
            #   targets or raw words are compared.
            # - Balloon::UpdateBody (0x800E6FC8), 226 words against 225: in STATE_CIRCLE the
            #   original loads m_charInfo's pointer into r31 before it calls fn_8040B67C
            #   (0x800E72EC), so it keeps this in r30 and saves r30-r31; the compiled code, through
            #   the matrix-vector operator*, loads the pointer after the call, keeps this in r31
            #   and saves only r31. In STATE_START the vector built from X, Y and Z is at sp+0xA4
            #   in the original and at sp+0x50 here, and the other cases' vectors sit 12 bytes
            #   higher here. Positional: 115 of the first 225 words differ and the original has 1
            #   more; aligned: 98 paired-differing, 6 original-only, 5 source-only.
            # Data: once the 100 linker-discarded copies are set aside (75 weak, 25 local; the
            # original keeps four of the weak vtables as zero-filled holes in .data), the bytes of
            # .rodata, .data, .sdata, .sbss and .sdata2 match the original by contents. Because
            # UpdateBody is one word shorter here, every function after it starts 4 bytes lower,
            # and four relocations that point at them target .text offsets 4 bytes lower: the
            # .ctors entry (__sinit_Balloon_cpp, .text+0x24EC in the original, +0x24E8 here) and
            # the three .data entries at +0x278, +0x27C and +0x284, the VacuumBalloon
            # sp_counted_base_impl destructor, dispose and get_deleter (for example .data+0x278
            # targets .text+0x2198 in the original and .text+0x2194 here). The original's .sdata2
            # is allocatable (A, 0x2) and this object's writable and allocatable (WA, 0x3).
            Object(NonMatching, "Game/Balloon.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BlastBreak.cpp", cflags=cflags_game_cpp),
            # 8 of the 9 functions are identical in every word and relocation target. ProcessHits
            # has 136 words against the original's 138; by position 67 of the 136 common words
            # differ, plus 2 unpaired original words. Aligned, 4 words differ (branch displacements
            # at +0x40, +0x98 and +0x114, and the loop branch at +0x210, +0x208 here), 2 are
            # original-only and none source-only: after locking the character info, the original
            # tests the locked pointer a second time (cmplwi r3,0 and beq at USA (GUPE8P)
            # 0x800ED67C-0x800ED680, Japan (GUPJ8P) 0x800EDA28-0x800EDA2C and PAL (GUPP8P)
            # 0x800EDA64-0x800EDA68). The cause is not established (possibly an inline helper). The
            # 30 relocation targets of ProcessHits are the same, in the same order. With the copies
            # the link discards set aside (vtables, type records and name strings of std::exception,
            # boost::bad_weak_ptr and sp_counted_base: 4 .rodata, 4 .data and 3 .sdata objects), the
            # 25 data objects equal the original's in contents and relocations. The compiled .sdata2
            # is WA (0x3) where the original's is A (0x2). The comparison is the same in all three
            # regions.
            Object(NonMatching, "Game/BlockChange.cpp", cflags=cflags_game_cpp),
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
            Object(Matching, "Game/WatchCamera.cpp", cflags=cflags_game_cpp),
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
            # Relocatable-object comparison: 13 of the 14 functions have identical words and
            # relocation targets, and the data matches once the linker-discarded copies are set
            # aside (std::sqrt, sqrtf with its two weak locals and three literals, the scaled-vector
            # operator* and shared_ptr::operator->); .rodata is 0x3F bytes against 0x40 (alignment
            # padding), and the compiled .sdata2 is writable (WA) where the original's is
            # allocatable only (A). fn_802EB0C4 is 183 words against 182: by position 128 of the 182
            # paired words differ and 1 word is source-only; aligned, 9 paired words differ, 1 is
            # original-only and 2 are source-only, with the same relocation targets in the same
            # order. In the facing test the original builds the facing vector at r1+0x50 and passes
            # r1+0x50 and r1+0x44 to fn_8040CF90 directly, saving only r31. This build builds the
            # facing vector at r1+0x44 and keeps the normalised vector's address, r1+0x50, in r30
            # (addi r30,r1,0x50; stmw/lmw r30), passing it from there (mr r4,r30).
            # Normal builds link the original object.
            Object(NonMatching, "Game/Super.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/TimerCountMission.cpp", cflags=cflags_game_cpp),
            # The game code and the boost instances match: of the 71 functions, 54 have a
            # counterpart in this object and 49 of them are identical in instructions and
            # relocation targets (relocatable-object comparison; addresses are the USA
            # release's). The other five are instances of
            # std::vector<boost::shared_ptr<Mission::BasicMessage> > and std::remove_if from the
            # MSL headers. Word counts compare the raw words of the relocatable objects, so a
            # call to a different function is not counted (such callees are named separately).
            # They are given by position and aligned (instructions matched up, branch
            # displacements masked):
            # - the vector destructor, 0x50 bytes against 0x4C: the original tests this for null
            #   twice, as an expanded base-class destructor does; 14 of 20 words by position,
            #   aligned 1 original-only word;
            # - the vector constructor: the same 12 words, but it calls __vector_obj's
            #   constructor where the original calls a further layer (0x802EC088);
            # - insert, 0x310 against 0x31C bytes: 148 words by position, aligned 24 paired
            #   differing, 17 original-only and 20 source-only;
            # - reserve, 0x10C against 0xFC bytes: 55 words by position, aligned 7 paired
            #   differing, 6 original-only and 2 source-only. insert and reserve call the
            #   __vector_obj constructor and swap where the original calls small helpers for the
            #   capacity, the maximum size, a constructor from the allocator and swap;
            # - remove_if, 0x70 bytes: 11 words by position, aligned 3 paired differing, 1
            #   original-only and 1 source-only, register and scheduling only: the original
            #   compares find_if's result in r3 and copies it to r5 afterwards, this build copies
            #   it first; opcodes, relocation targets and load/store counts are identical.
            # 17 helpers of the original (146 words) have no counterpart here: the vector
            # constructor chain (4, from 0x802EC088), swap (5, from 0x802ECA44), the constructor
            # from the allocator (3, from 0x802ECB6C), the capacity (2, from 0x802ECCD8) and the
            # maximum size (3, from 0x802ECD00). The data match by contents, relocation targets
            # included, once the compiled copies that the original unit does not have are set
            # aside: 4 .rodata strings (the names and what() texts of std::exception and
            # std::bad_alloc), 4 .data objects (the std::exception, std::bad_alloc and
            # boost::detail::sp_counted_base vtables and the std::bad_alloc base list), 2 .sdata
            # RTTI records (std::exception, std::bad_alloc) and 8 .sbss objects (the boost
            # placeholders _2 to _9). Neither object has .sdata2.
            Object(NonMatching, "Game/BasicMessage.cpp", cflags=cflags_game_cpp),
            # Relocatable-object comparison (addresses are the USA release's): of the 72
            # functions, 55 have a counterpart in this object and 50 of them have identical words
            # and relocation targets: the other game code, std::sort with its helpers, std::ceil
            # and the boost::shared_ptr<Mission::CountMessage> members. Word counts compare raw
            # words, so a call to a different function is not counted there; relocation targets
            # are given separately. Two methods: by position (word i against word i over the
            # shorter of the two functions, with the longer one's extra words counted on their
            # own) and aligned (instructions matched up by a sequence diff, Python's difflib, with
            # branch displacements masked; paired differing, original-only and source-only words).
            # Sizes are the original's first:
            # - fn_802ED4E0, 0xD0 bytes against 0xCC: by position 45 of 51 words differ, plus 1
            #   original-only word; aligned 3 paired differing and 1 original-only. The relocation
            #   targets are the same, in the same order. The original loads the word at 0x14 again
            #   at the top of each pass (lwz r0,0x14(r29)) where this build compares the value its
            #   loop test has just loaded. It also keeps the record in r5 and loads the record's
            #   word at 0x14 again for the second fn_802EEAF0 call (lwz r4,0x14(r5)); this build
            #   keeps the record in r4 and passes the value it has just compared (mr r4,r0).
            # The other four are instances of std::vector<boost::shared_ptr<Mission::CountMessage> >
            # that differ as the BasicMessage instances above do:
            # - the vector destructor, 0x50 bytes against 0x4C: by position 13 of 19 words differ,
            #   plus 1 original-only word; aligned 1 original-only word (a second branch on the
            #   original's null test of this) and no other difference;
            # - the vector constructor: the same 12 words, but it calls __vector_obj's
            #   constructor where the original calls a further layer (0x802ED7C8);
            # - insert, 0x310 bytes against 0x31C: by position 145 of 196 words differ, plus 3
            #   source-only words; aligned 24 paired differing, 17 original-only and 20
            #   source-only;
            # - reserve, 0x10C bytes against 0xFC: by position 51 of 63 words differ, plus 4
            #   original-only words; aligned 7 paired differing, 6 original-only and 2
            #   source-only.
            # In total, by position 254 paired words differ, with 6 original-only and 3
            # source-only words; aligned, 34 paired differing, 25 original-only and 22 source-only.
            # Relocation targets: insert and reserve call __cap, __vector_obj's constructor and
            # swap where the original calls small helpers for the capacity, the maximum size, a
            # constructor from the allocator and swap. 17 helpers of the original (146 words)
            # have no counterpart here: the vector constructor chain (4, from 0x802ED7C8), swap
            # (5, from 0x802EE344), the constructor from the allocator (3, from 0x802EE46C), the
            # capacity (2, from 0x802EE618) and the maximum size (3, from 0x802EE640). This
            # object has 17 functions the original unit lacks: 13 weak copies that the link
            # discards (10 of them of functions the game defines elsewhere, such as
            # boost::detail::shared_count's destructor), and __vector_obj's constructor and swap
            # with the two __compressed_pair constructors they call. 33 of the 55 paired functions
            # are in the original's relative order: the original emits the vector's begin, end,
            # back, push_back, pop_back and clear next to their users, this build after the
            # unit's other functions. The data match by contents, relocation targets included,
            # once the compiled copies that the original unit does not have are set aside: 4
            # .rodata strings (the names and what() texts of std::exception and std::bad_alloc),
            # 5 .data objects (the Mission::MessageManage, std::exception, std::bad_alloc and
            # boost::detail::sp_counted_base vtables and the std::bad_alloc base list), 2 .sdata
            # RTTI records (std::exception, std::bad_alloc) and 2 .sdata2 doubles (the static
            # locals of <cmath>'s sqrtf). The original sections end in alignment padding that this
            # object lacks: 2 bytes of .rodata and 7 bytes of .sbss. The compiled .sdata2 is
            # writable (WA) where the original's is allocatable only (A). Normal builds link the
            # original object.
            Object(NonMatching, "Game/CountMessage.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Message.cpp", cflags=cflags_game_cpp),
            # Relocatable-object comparison (addresses are the USA release's): of the 70
            # functions, 53 have a counterpart in this object and 48 of them have identical words
            # and relocation targets: the other game code, std::sort with its helpers, and the
            # boost::shared_ptr<Mission::TimerMessage> members. Word counts compare raw words, so
            # a call to a different function is not counted there; relocation targets are given
            # separately. Two methods: by position (word i against word i over the shorter of the
            # two functions, with the longer one's extra words counted on their own) and aligned
            # (instructions matched up by a sequence diff, Python's difflib, with branch
            # displacements masked; paired differing, original-only and source-only words). Sizes
            # are the original's first:
            # - fn_802EF224, 0x118 bytes in both: by position 3 of 70 words differ; aligned 3
            #   paired differing. The relocation targets are the same, in the same order. The
            #   original keeps the record in r5 and loads its word at 0x14 again for the second
            #   fn_802EEAF0 call (lwz r4,0x14(r5)); this build keeps the record in r4 and passes the
            #   value it has just compared (mr r4,r0).
            # The other four are instances of std::vector<boost::shared_ptr<Mission::TimerMessage> >
            # that differ as the BasicMessage instances above do:
            # - the vector destructor, 0x50 bytes against 0x4C: by position 13 of 19 words differ,
            #   plus 1 original-only word; aligned 1 original-only word (a second branch on the
            #   original's null test of this) and no other difference;
            # - the vector constructor: the same 12 words, but it calls __vector_obj's
            #   constructor where the original calls a further layer (0x802EF564);
            # - insert, 0x310 bytes against 0x31C: by position 145 of 196 words differ, plus 3
            #   source-only words; aligned 24 paired differing, 17 original-only and 20
            #   source-only;
            # - reserve, 0x10C bytes against 0xFC: by position 51 of 63 words differ, plus 4
            #   original-only words; aligned 7 paired differing, 6 original-only and 2
            #   source-only.
            # In total, by position 212 paired words differ, with 5 original-only and 3
            # source-only words; aligned, 34 paired differing, 24 original-only and 22 source-only.
            # Relocation targets: insert and reserve call __cap, __vector_obj's constructor and
            # swap where the original calls small helpers for the capacity, the maximum size, a
            # constructor from the allocator and swap. 17 helpers of the original (146 words)
            # have no counterpart here: the vector constructor chain (4, from 0x802EF564), swap
            # (5, from 0x802EFFDC), the constructor from the allocator (3, from 0x802F0104), the
            # capacity (2, from 0x802F02B0) and the maximum size (3, from 0x802F02D8). This
            # object has 20 functions the original unit lacks: 16 weak copies that the link
            # discards (13 of them of functions the game defines elsewhere, such as
            # boost::detail::shared_count's destructor and fn_8000A950), and __vector_obj's
            # constructor and swap with the two __compressed_pair constructors they call. 31 of
            # the 53 paired functions are in the original's relative order: the original emits
            # the vector's begin, end, back, push_back, pop_back and clear next to their users,
            # this build after the unit's other functions. The data match by
            # contents, relocation targets included, once the compiled copies that the original
            # unit does not have are set aside: 4 .rodata strings (the names and what() texts of
            # std::exception and std::bad_alloc), 5 .data objects (the Mission::MessageManage,
            # std::exception, std::bad_alloc and boost::detail::sp_counted_base vtables and the
            # std::bad_alloc base list), 2 .sdata RTTI records (std::exception, std::bad_alloc)
            # and 3 .sdata2 constants (<cmath>'s two sqrtf statics and the 0.0001 of the
            # fn_8000A950 copy). The original sections end in alignment padding that this object
            # lacks: 2 bytes of .rodata and 7 bytes of .sbss. The compiled .sdata2 is writable
            # (WA) where the original's is allocatable only (A). Normal builds link the original
            # object.
            Object(NonMatching, "Game/TimerMessage.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/CityBombBig.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/CityGadget.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/CityBombSmall.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ChaosSpear.cpp", cflags=cflags_game_cpp),
            # Relocatable-object comparison: the 13 functions other than the start-up function
            # match, words and relocations. The original sets the name and the floats at 0x10 and
            # 0x14 (1.0 and 5.0, two .sdata2 literals) of the weapon description in the start-up
            # function (10 words) and has zeros there in .data; this build stores only the name at
            # start-up (6 words) and keeps 1.0 and 5.0 in .data, so .sdata2 is 0x18 bytes against
            # 0x20. Of the 6 paired start-up words 5 differ (the two lis in the other order, an stw
            # and a blr where the original has its two lfs, and an stw for the original's stwu),
            # and the original's last 4 words (an stw, two stfs and the blr) have no counterpart.
            # A constant initialiser is folded into the static data, as for Game/fn_801F6488.cpp
            # above. The original's .sdata also ends in 4 zero bytes that this object lacks (0x1C
            # bytes against 0x20). The rest of the data matches once Fn8040Vector3's default
            # constructor, a weak copy the linker discards, is set aside. Normal builds link the
            # original object. In a USA diagnostic link of this object, the .sdata tail comes back
            # as alignment fill and 1.0 and 5.0 stay in the linked description where the original
            # has zeros; the later code and data land lower, so 17 more words in 8 of the other
            # functions differ, all in relocated fields.
            Object(NonMatching, "Game/DarkHammer.cpp", cflags=cflags_game_cpp),
            # Relocatable-object comparison, the same in USA (GUPE8P), Japan (GUPJ8P) and PAL
            # (GUPP8P): 73 of the 75 functions have identical words and relocation targets once the
            # copies the linker discards (84 weak and 3 local functions, the _2 to _9 placeholders,
            # 13 local RTTI records and names, sqrtf's two weak locals and three weak vtables, which
            # stay zero-filled holes in .data) are set aside. OnCollision is 0x264 bytes against
            # 0x270: the original copies the inverse matrix into a second stack matrix (a
            # fn_8040C148 call, 3 words this source lacks) and transforms the hang position with
            # the copy, as an inline function returning the inverse by value would; aligned, 19
            # paired words differ (the frame size and stack offsets, the frame being 0x30 bytes
            # larger in the original, and branch displacements across the missing words) and 3
            # original words are unpaired (compared position by position, 51 paired positions differ
            # and the original's last 3 words are unpaired). The static initialiser
            # differs in 9 words: it addresses the four bar objects from the start of .bss, and
            # they start there here but 0xC bytes later in the original. Data: the original's .bss
            # is 0x168 bytes, with 0xC bytes before the bar objects and 0x114 after them that no
            # code in the file reads or writes (0x48 bytes here). The compiled .sdata2 is writable
            # (WA) where the original's is allocatable only (A). The other sections match by
            # contents, apart from section-end alignment. Normal builds link the original object.
            Object(NonMatching, "Game/Helicopter.cpp", cflags=cflags_game_cpp),
            # Compared as relocatable objects, 13 of the 16 functions have identical words and
            # relocation targets once the linker-discarded copies are set aside (28 weak boost and
            # std::exception instances and 22 locals: the RTTI records and name strings of their
            # classes, two local converting-constructor copies and a one-byte .sbss tag).
            # fn_802F299C (64 words) and fn_802F2A9C (41 words) each differ in 9 words, by position
            # and aligned (9 paired, none unpaired), in register choice only: the same instructions
            # in the row lookup, with the addresses of lbl_8057D6A0 and of the table in r4 and r5
            # the other way round and the sum built in r0 and r6 instead of r4 and r0. fn_802F2C1C
            # (70 words in the original, 69 here) differs by position in 20 paired raw words plus 1
            # original-only word; one more position has the same raw branch word but a different
            # relocation target, as the sequences are shifted. Aligned, 4 paired words differ and 1
            # word is original-only, with the same relocation targets: the original loads the
            # message id into r0, compares it and copies it to r31 after the early return, where
            # this build loads it straight into r31. The functions after fn_802F2C1C sit 4 bytes
            # lower because it is one word shorter.
            # Data, the same in all three regions: with the discarded copies set aside, the
            # retained contents match, but the sections end before the original's: .rodata at
            # 0x809 against 0x810, .data at 0x64 against 0x68 and .sdata2 at 0x14 against 0x18
            # (and .sbss at 0x11 against 0x18). The original's extra 7, 4 and 4 bytes are zero end
            # alignment, which the link supplies. The compiled .sdata2 is writable (WA) where the
            # original's is allocatable only (A). .sdata: the original has a zero word
            # (0x805ECA6C in USA (GUPE8P)) that no code references between the literal 2 of
            # fn_802F2C1C and s_name; this object lacks it, so s_name and the three RTTI records
            # sit 4 bytes lower and the section ends at 0x20 against the original's 0x24 plus 4
            # bytes of end alignment.
            Object(NonMatching, "Game/HintController.cpp", cflags=cflags_game_cpp),
            # Relocatable-object comparison: nine of 11 functions have identical words and
            # relocation targets. The start-up function and the description's floats at 0x10 and
            # 0x14 differ as in Game/DarkHammer.cpp above: 24 bytes against 40, with 5 of the 6
            # paired words different and 4 original-only words. .sdata2 is 0x10 bytes marked
            # writable (WA) against the original's 0x18 bytes, allocatable only (A); the original's
            # last 8 bytes are the 1.0 and 5.0 that its start-up function stores. PlayShotEffects
            # is 152 bytes against 156, with 28 of the 38 paired words different and 1
            # original-only word: the original copies the frame pointer that fn_80063234 returns
            # to r0, sets r3 to the result slot and then moves r0 to r4, while this build moves it
            # to r4 directly. The weak copies set aside are the weak_ptr<EffectElement> and
            # weak_count destructors and sp_counted_base's destructor, destruct and vtable with its
            # RTTI record and name string. Normal builds link the original object. In a USA
            # diagnostic link of this object, PlayShotEffects has 29 differing paired words and 1
            # original-only word, and 30 words of the other nine functions differ, all in
            # relocated fields.
            Object(NonMatching, "Game/HeavyShot.cpp", cflags=cflags_game_cpp),
            # The data match by contents once the 63 definitions the linker discards are set aside
            # (47 weak copies, such as boost::bad_weak_ptr's vtable, and 16 local ones, such as the
            # boost::bind placeholders _2 to _9); the original keeps zero-filled holes in .data where
            # three of those vtables were, and its .rodata and .sbss end with 1 and 3 bytes of
            # alignment padding. .sdata2 is writable (WA) here and allocatable only (A) in the
            # original.
            Object(Matching, "Game/BulletChaosSpear.cpp", cflags=cflags_game_cpp),
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
            # and 120.0f literals are pooled in the opposite order. Linked from source as a
            # diagnostic together with BkWorm (normal builds link the original object), the USA
            # (GUPE8P) DOL differs in 232 words, four of them in this file's ranges: the two
            # .sdata2 literals (0x805F53A0, 0x805F53A4) and the two loads that address them
            # (0x8019166C, 0x80191970). The other 228 are in BkWorm's ranges.
            Object(NonMatching, "Game/BkWingSmall.cpp", cflags=cflags_game_cpp),
            # The split covers the whole original file: USA (GUPE8P) .text 0x80191CF0-0x80193434 (67
            # functions: the static initialiser at 0x80193314 and 29 adjustment thunks among them)
            # with its .ctors, .rodata, .data, .bss, .sdata, .sbss and .sdata2. Relocatable
            # comparison: 66 functions have identical instructions and relocation targets;
            # fn_80192394 differs in 13 of its 95 words, register allocation only (this takes r27
            # where the original has r31, and the four loop values each sit one register higher).
            # Data by contents, with the linker-discarded copies set aside: .sdata2 pools 0.0f,
            # 0.5f, the empty string and -1.0f in another order (original 0.0f, 0.5f, "", -1.0f;
            # this build "", 0.0f, -1.0f, 0.5f), and .rodata has the BkWormBase type name before
            # the model tables, where this build places it among the other type names. The compiled
            # .sdata2 section is writable (WA) where the original's is allocatable only (A). Linked
            # from source as a diagnostic together with BkWingSmall (normal builds link the original
            # object), the USA DOL differs in 232 words, 228 of them in this file's ranges: 24 in
            # .text (fn_80192394 and 11 references to the moved literals and tables), 162 in
            # .rodata (the type name and the tables and strings after it), 13 in .data and 26 in
            # .sdata (pointers to those strings) and 3 in .sdata2. The other four are BkWingSmall's.
            Object(NonMatching, "Game/BkWorm.cpp", cflags=cflags_game_cpp),
            # The split covers the whole original file: USA (GUPE8P) .text 0x800CBC1C-0x800D216C
            # (255 functions, the static initialiser at 0x800D202C among them) with its .ctors,
            # .rodata, .data, .bss, .sdata, .sbss and .sdata2. Code in that range addresses the
            # .rodata from its start (0x804B6728), which holds the FleetHolder, EggCannonBullet and
            # EGGFLEET.BIN strings. The source has bodies for 19 of the 255 functions: FleetHolder's
            # methods, its descriptor's hooks and the template copies they use. Not reconstructed:
            # Gadget::fn_800CBC1C and fn_800CBC20 at the start of the range (only data refers to
            # them, and the lowest-addressed references are this file's vtable and parameter
            # table), the EggFleet, EggCannon and EggCannonBullet code, ~EggFleet (so
            # checked_delete<EggFleet> is inlined here) and the static initialiser. The normal
            # link uses the original object. Data of the compiled object, compared with the split
            # original by contents, relocation types and relocation target symbols within the
            # original's symbol boundaries, the same in USA (GUPE8P), Japan (GUPJ8P) and PAL
            # (GUPP8P): of its 41 data objects 32 equal a whole original object. 5 equal only part
            # of a larger original symbol: four .rodata strings inside the 0x7C-byte string block
            # at 0x804B6DD4 (USA), the sp_counted_base_impl<EggFleet *, ...> type name,
            # "exception", "std::bad_alloc" and "bad_alloc", and the std::bad_alloc base-class
            # list, whose 12 bytes and std::exception type-record target occur at 0x80522F54 and
            # 0x80522FA4 inside the .data symbols at 0x80522F3C and 0x80522F94. 4 have no
            # counterpart in the split: the weak vtables of std::exception, std::bad_alloc and
            # boost::detail::sp_counted_base (the original refers to an external copy of the last
            # one) and the std::bad_alloc type record. Section sizes (original / compiled): .rodata
            # 0x820 / 0x155, .data 0x3C8 / 0x1E4, .sdata 0xA8 / 0x44, .sbss 0x68 / 0x1, .sdata2
            # 0x80 / 0x18. The compiled object has no .ctors (4 bytes in the original) or .bss (24
            # bytes): the original static initialiser (0x800D202C) stores floats into two 12-byte
            # vectors in .bss, nine words in .sbss and the .data object at 0x80522C30, none of
            # which the source defines, so no initialiser is generated. Its .sdata2 is writable
            # (WA); the original's is read-only (A).
            Object(NonMatching, "Game/FleetHolder.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/PowerDeviceCage.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/PowerDeviceNaked.cpp", cflags=cflags_game_cpp),
            # All 95 functions have identical instruction words and relocation types. Selecting
            # this source produces the original DOL in all three regions, but ELF validation still
            # requires native zero-filled .data labels absent from the final symbol table. In USA
            # (GUPE8P), lbl_80523550 spans 12 zero bytes: the source's weak SetAdapterState vtable
            # is selected elsewhere, leaving the same zero hole without a symbol at that address.
            # The data-symbol and discarded-copy correspondence is not fully verified, so the unit
            # retains its original-object fallback.
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
            # "csdBase.h" is also 0x28 earlier: the shared reference destructor emits the original's
            # 0x20-byte deallocation error string. The original's .rodata also has strings that this
            # object lacks: "csdRCObject.h", the type names of the RCPtr holders of unsigned char
            # and CTexList, their CFunctorBase names and CResourceBase<Chao::CSD::Project>. The
            # nested CScene CFunctorBase precedes its holder name, as in the original; the CBase
            # name is also present. The holder's CBase base list is present. The functor's CBase
            # association remains inferred from the native name-pool order. This object has type
            # records, base lists and weak/local copies absent from the original unit (TNumEdit,
            # CharaColliBase, std::exception, boost::bad_weak_ptr, sp_counted_base, the boost
            # placeholders _2 to _9). It also emits weak copies of the 0x198 RCPtr<CScene>
            # destructor, the 56-byte reference-copy constructor, counted-reference Assign and
            # CMemoryAlloc::FreeObject. Weak CScene holder/CFunctorBase destructors
            # and their tables are also emitted, with CBase, holder and CFunctorBase type records.
            # Source .rodata is 867 bytes versus 1264 native bytes; .data is 412 versus 248. The
            # object comparison still matches 96 data bytes. Normal links use the original object.
            Object(NonMatching, "Game/Gong.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ShootingGameBalloon.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ShootingGameManagerTask.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BombingWall.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BombServer.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Researcher.cpp", cflags=cflags_game_cpp),
            # 216 of the 219 functions have identical instructions and relocations once the weak and
            # local copies that the linker discards are removed. The static initialiser differs in 5
            # words: two registers swapped and the three roll velocities addressed from the start of
            # .bss here, 0x3C bytes after it in the original (see below).
            # StonePole::CreateStones (5 words) and StonePole::Update (3 words) load a stone kind
            # from the step table differently: the original adds the step offset to the holder's
            # address and loads at 0x50 from it, this source adds 0x50 to the offset and uses an
            # indexed load (in CreateStones the argument set-up around it is also ordered
            # differently). These three bodies have 13 differing positional words in total.
            # The data match by contents (the original object's absent copies set aside); .sdata2
            # also has the original order once the weak sqrtf statics of <cmath> are discarded (the
            # weak fn_8000B044 copy places 0.0001 before 1 and 70). One difference remains: the
            # original's .bss starts with 0x3C bytes that no code references, relative to which its
            # static initialiser addresses the three vectors (their type is unknown).
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
            # Relocatable comparison with the split original object (local names paired by
            # position), the same in USA (GUPE8P), Japan (GUPJ8P) and PAL (GUPP8P): 81 of its 83
            # functions have identical instruction words and relocation targets.
            # - ContainerNormal::OnCommand (USA 0x800F7290) is 257 words against 258: in the
            #   VacuumCommand case the original copies the command pointer (mr r29,r31 at
            #   OnCommand+0x338, before the fn_80161880 call) and passes that copy to fn_80254384,
            #   where this build passes r31 itself. Positional: 62 of the 257 common words differ
            #   and 1 original word is unpaired; aligned: 12 pairs differ (11 branch displacements
            #   across the missing word and the register of that argument), 1 word is only in the
            #   original and none only in this build. The other calls have the original's targets.
            # - ContainerNormal::Update (USA 0x800F7890) is 218 words against 212: the original
            #   calls the out-of-line copy of ContainerBehavior::GetPosition (0x800EF448) with
            #   m_behavior and m_charInfo loaded directly (3 words at Update+0x148), where this
            #   build inlines GetPosition: it calls both shared pointers' operator-> out of line and
            #   then Fn8040Vector3::operator= (9 words). The original calls that copy from all six
            #   of its callers. Measured in USA (GUPE8P): with Fn8040Vector3::operator= defined in
            #   its class body, GetPosition stays a call and Update is identical to the original,
            #   but UpdateMotion (80 to 94 words), UpdateHeldPosition (32 to 36), OnCommand (258 to
            #   281) and the out-of-line GetPosition (7 words against the original's 11) then
            #   inline the assignment, which the original calls out of line in all four. So that is
            #   not the original's form, and why the original keeps the call is not established.
            #   Positional (word i against word i): 127 of the 212 common words differ and 6 build
            #   words are unpaired; aligned (instructions matched up): 5 pairs differ (the 2 loads
            #   and 3 branches across the extra words), no word is only in the original and 6 only
            #   in this build, and the matched call has operator= as its target instead of
            #   GetPosition. The other calls have the original's targets.
            # Data, compared object by object with the 131 compiled copies that are absent from the
            # original split set aside (94 weak and 3 local functions, 6 weak vtables, 4 .data, 9
            # .rodata, 7 .sdata and 8 .sbss locals): every original object matches in bytes and
            # relocation targets except the state jump table, whose entries for the states after
            # STATE_WAITING point 0x18 bytes later into Update.
            # A link with the compiled object discards 130 of those copies but keeps the weak
            # shared_ptr<ContainerBehavior>::operator-> (8 bytes, after Update), which only this
            # build's Update calls. Measured in that link: the four functions from SetTransform to
            # Update start 4 bytes earlier (OnCommand's missing word), the functions after Update
            # and the code after this file 0x1C bytes later (Update's extra 0x18 bytes and
            # operator->'s 8), and the later sections (.ctors, .rodata, .data, .sdata, .sbss,
            # .sdata2) 0x20 bytes later.
            Object(NonMatching, "Game/Container.cpp", cflags=cflags_game_cpp),
            # Relocatable comparison with the split original object, the same in USA (GUPE8P),
            # Japan (GUPJ8P) and PAL (GUPP8P): 15 of its 17 functions have identical instruction
            # words and relocation targets. The original's ProbeGround (USA 0x800F8D2C) calls the
            # out-of-line copy of UnkProbeNode_80021FE4::fn_800F8E88 (0x800F8E88, 6 words), which
            # undecompiled code at 0x801B9E54 and 0x802A2980 also calls; this build expands that
            # inline function, so the copy is absent and ProbeGround is 88 words against 87: the
            # call becomes five instructions, and one saved register fewer renumbers r28-r31.
            # Positional: 47 of the 87 paired words differ (no relocation target differs under
            # equal words) and 1 build word is unpaired; aligned: 16 of 84 pairs differ, 3 words
            # are only in the original and 4 only in this build, and the original's call to the
            # copy has no counterpart. A link with this object fails: the two callers of the
            # copy have no definition.
            # Data, compared object by object with the 47 compiled copies that are absent from the
            # original split set aside (35 weak and 1 local function, 3 weak vtables, 1 .data, 4
            # .rodata and 3 .sdata locals): every original object matches in bytes and relocation
            # targets.
            Object(NonMatching, "Game/ContainerBehavior.cpp", cflags=cflags_game_cpp),
            # Relocatable comparison with the split original object (local names paired by
            # position), the same in USA (GUPE8P), Japan (GUPJ8P) and PAL (GUPP8P): 134 of its 139
            # functions have identical instruction words and relocation targets when each body is
            # paired with its counterpart (the original's 0x58-byte fn_800F0404 with this build's
            # function1 copy assignment, which is equal). The differences are Bomb::Update, the
            # absent GetPosition copy and 0x30-byte function below, Initialize's call target and
            # function1's swap. Paired by symbol name only, 135 compare equal, because the
            # 0x30-byte function and this build's copy assignment share a name.
            # - Bomb::Update (USA 0x800EF098) is 226 words against 220: the original calls the
            #   out-of-line copy of ContainerBehavior::GetPosition (0x800EF448) with the m_behavior
            #   and m_charInfo pointers loaded inline; this build inlines GetPosition and calls the
            #   smart pointers' operator-> out of line for its arguments, as in Game/Container.cpp.
            #   Positional: 140 of the 220 paired words differ (132 raw words and 8 relocation
            #   targets under equal words) and 6 build words are unpaired; aligned (instructions
            #   matched up): 6 pairs differ (the 2 loads, the call target and 3 branches across the
            #   extra words), no word is only in the original and 6 only in this build. The
            #   GetPosition copy is therefore absent from this object.
            # - The 0x30-byte function at 0x800F03D4 is absent: the original's Initialize calls it
            #   to store the grab position handler, and it calls that boost::function1's copy
            #   assignment (0x800F0404) and returns its first argument; this build's Initialize
            #   calls the copy assignment directly (symbols.txt gives the 0x30-byte function the
            #   copy assignment's name, as at 0x80054E28 and 0x800729B0 for two other function1
            #   types, so the relocation names agree). Initialize's words are identical, but its
            #   call targets the copy assignment's body instead of the 0x30-byte function.
            # - That function1's swap (0x800F045C) has identical words, but its call resolves to
            #   this object's local copy of std::swap<boost::detail::function::any_pointer> (an
            #   inline template in this build's <utility>) where the original calls the shared
            #   copy at 0x80019164.
            # Data, compared object by object with the 95 compiled copies that are absent from the
            # original split set aside (67 weak and 2 local functions, 4 weak vtables, 8 .rodata, 6
            # .sdata and 8 .sbss locals), and with them the weak
            # shared_ptr<ContainerBehavior>::operator-> (8 bytes), also absent from it, which only
            # this build's Update calls: every original object matches in bytes and relocation
            # targets except the state jump table, whose entries for the states after STATE_WAITING
            # point 0x18 bytes later into Update.
            Object(NonMatching, "Game/Bomb.cpp", cflags=cflags_game_cpp),
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
            Object(Matching, "Game/PJSLauncher.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/SecretDisk.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/GiantLantern.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/GravityChangeSwitch.cpp", cflags=cflags_game_cpp),
            # 35 of 36 functions have identical words and relocations, and the data matches once
            # the linker-discarded weak and local copies are set aside. The constructor is 185
            # words in the original and 184 here: in the branch for a stored state of 3, the
            # original loads the com id into r30 before the fn_800CB69C call and zero-extends it
            # with clrlwi for fn_801690D0, where this build loads it into r4 after the call (a
            # local copy of m_comId moves the load but leaves mr for the clrlwi).
            Object(NonMatching, "Game/ElecAccessPanel.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/HeavyDog0510ModeCtrlUnit.cpp", cflags=cflags_game_cpp),
            # Compared as relocatable objects, 11 of 12 functions have identical words and
            # relocation targets, and the data contents match once the linker-discarded weak and
            # local copies are set aside (the compiled .sdata2 is writable, WA, where the original's
            # is allocatable only, A, and the compiled .rodata and .sdata2, 0xC9 and 0x2C bytes,
            # lack the original's section-end alignment to 0xD0 and 0x30). .bss: the original keeps
            # 0x30 bytes before the first sphere and 0x94 bytes after the vector that no code reads
            # or writes, so the static initialiser differs in 4 of its 32 words (the objects'
            # offsets from the .bss start) and the parameter tables' three pointers to those objects
            # in their relocation addends.
            Object(NonMatching, "Game/HeavyDog0510.cpp", cflags=cflags_game_cpp),
            # Compared as relocatable objects, 23 of 24 functions have identical words and
            # relocation targets, and the data contents match once the linker-discarded weak and
            # local copies are set aside, apart from two differences (the .sdata2 contents are the
            # same 0x78 bytes, but the compiled section is writable, WA, where the original's is
            # allocatable only, A). .bss: the original keeps 0x30 bytes before the first sphere and
            # 0x98 bytes after the last vector that no code reads or writes, so the static
            # initialiser differs in 6 words and the parameter tables' pointers to those objects in
            # their relocation addends (the objects' offsets from the .bss start). .rodata: the
            # class name "HeavyDog0410CtrlUnit" comes after "BossCtrlUnit" and "HeavyDogCtrlUnit" in
            # the original and before them here. A USA (GUPE8P) DOL linked from this source as a
            # diagnostic differs in nine words across three functions, with no unpaired words; the
            # normal build links the original object.
            Object(NonMatching, "Game/HeavyDog0410.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/HeavyDog0410ModeCtrlUnit.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/VacuumContainer.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BaseDoor.cpp", cflags=cflags_game_cpp),
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
            # 45 of 46 functions have identical words and relocation targets, and the data
            # matches once the linker-discarded weak and local copies are set aside. UpdateBody
            # (0x801036E4) is 141 words instead of 142: the original's dispatch on m_step compares
            # with 1 and 0 and is followed by a branch that nothing reaches (b at 0x80103760,
            # UpdateBody+0x7C). The branch is unexplained: a further case label is one possible
            # cause, but nothing stores a value other than 0 or 1, so the source has the two cases
            # and no such label. Compared position by position, 114 of the 141 words differ and
            # the original's last word has no counterpart; with the missing branch set aside,
            # only the four dispatch branches before it differ (beq, bge, bge and b at 0x8010374C,
            # 0x80103750, 0x80103758 and 0x8010375C, each 4 bytes shorter). The four
            # section-relative relocations to functions after UpdateBody (in CreateStage, Create,
            # CreateProbe and the vtable) have the same targets with addends 4 lower, and the
            # function-local static of the parameter label callback and its guard are numbered
            # s_form$601 and init$602 here (s_form$602 and init$603 in symbols.txt).
            Object(NonMatching, "Game/Fan.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Fire.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Fruit.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/GoalRing.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/HealingUnit.cpp", cflags=cflags_game_cpp),
            # Compared with the split original object (USA, Japan and PAL alike), with the 60
            # symbols the link discards set aside (46 weak copies, 14 locals), 57 of the 60
            # functions have identical words and relocations (targets resolved to their symbols).
            # UpdateBody (0x8010A1DC, USA) is 109 words against 108: the original's state dispatch
            # compares with 2 first and has one unreachable branch, as a switch with a fourth, empty
            # case 3 compiles, where this build compares with 1 first; nothing in the file stores 3,
            # so the source keeps the three cases. Raw words in USA: by position, 79 of the first
            # 108 differ and this build has 1 more; with the instructions aligned by mnemonic, 6
            # paired words differ, 1 is original-only and 2 are source-only. The vector destructor
            # of the manager that fn_80109D64 returns (0x80109E2C) is 19 words against 20: the
            # original tests the object pointer a second time before calling the base destructor. By
            # position, 13 of the 19 common words differ and the original has 1 more; aligned, 1
            # paired word differs and 1 is original-only. The constructor differs in 3 of its 181
            # words, all immediates that are offsets into .rodata (below). Layout: this build emits
            # the __vector_obj destructor, __alloc and __cap right after the vector destructor and
            # __data before the addressof functions, where the original emits them with clear after
            # the bind functions, so the functions from 0x80109E7C on sit 0x7C to 0x88 bytes later
            # here. Data, by contents with the discarded copies set aside: .data, .sdata and .sdata2
            # match; the original's .rodata has 0x1C zero bytes after the three axes (the strings
            # after them sit 0x1C lower here) and ends with the string "string constructor: n >
            # max_size\n", which this object lacks. In .sbss the original has both accessor guard
            # bytes after the last boost tag object and then both pointers, where this build puts
            # each guard after its pointer; in .bss it has both destructor-chain records before the
            # registry and the manager, where this build interleaves them. Section flags: the
            # original's .sdata2 is read-only (A), this object's is writable (WA), in all three
            # regions. Linked in place of the original (USA), this object keeps 0x320 bytes of
            # .rodata against the original's 0x360 and 0x30 bytes of .sbss (up to the next
            # 8-byte-aligned object) against 0x28, the extra alignment coming from the interleaved
            # guards and pointers; .data, .bss, .sdata and .sdata2 keep the original sizes.
            Object(NonMatching, "Game/HintRing.cpp", cflags=cflags_game_cpp),
            # 122 of 124 functions are identical in every word and relocation. The data sections
            # and their relocations match once the linker-discarded weak and local copies are set
            # aside; .sdata2 is allocatable (A, flags 0x2) in the original and writable and
            # allocatable (WA, 0x3) here, and its 0x6B bytes are padded to 0x70 in the original.
            # Two functions differ in instruction order (counts: positional = word i against word
            # i, raw word or relocation target; aligned = instructions matched up as sequences):
            # - ItemBox::OnCommand (0x8010C110), 64 words each: the original copies the command
            #   pointer to r3 before it compares m_type with 10 (0x8010C1D8), the compiled code
            #   after that test. Positional: 5 raw words differ, no further relocation-target
            #   difference. Aligned: 1 original-only and 1 source-only word (the moved copy); the
            #   bne after the test keeps its word and in both versions skips the optional icon load
            #   to the same lwz r5 (0x8010C1E8), which sits one word earlier here, so it pairs as
            #   differing when branch-target offsets are compared and as equal when raw words are.
            # - fn_8010DE60 (0x8010DE60), 92 words each: the original stores the zero vector for
            #   the HomingJumpCommand (0x8010DF70-0x8010DF80) before the allocation, the compiled
            #   code after the allocation and its null test. Positional: 10 raw words differ, no
            #   further relocation-target difference. Aligned: 0 paired-differing, 6 original-only,
            #   6 source-only words.
            Object(NonMatching, "Game/ItemBox.cpp", cflags=cflags_game_cpp),
            # 63 of the 66 functions of the original split object have identical words and
            # relocations (USA, Japan and PAL alike). Counts: positional = word i against word i;
            # aligned = instructions matched up, as paired-differing, original-only and
            # source-only words.
            # - UpdateBody (0x80113C3C), 109 words against 110: the original's dispatch on m_state
            #   compares with 2 (beq, then bge past the switch), then with 0, and ends with a branch
            #   nothing reaches, the compare tree MWCC builds for the case values 0, 1, 2 and 3.
            #   Nothing in the file stores 3 in m_state, so the source has only the three cases.
            #   Positional: 99 of the first 109 words differ and this build has 1 more; aligned: 7
            #   paired-differing, 0 original-only, 1 source-only.
            # - CollectTouchedRings (0x80112AF4), 96 words against 92: the original passes
            #   IsTouched a bool built with branches (li 0, then li 1 when Test(2) returns true) and
            #   kept in r28, so it saves r28-r31, where this build passes the result in place and
            #   saves r29-r31. Positional: 59 of the first 92 words differ and the original has 4
            #   more; aligned: 8 paired-differing, 4 original-only, 0 source-only.
            # - OnCommand (0x801124E8), 226 words against 227: for command 0x14 this build computes
            #   the address of the command's m_target once before the loop and keeps it in a saved
            #   register, so it saves r26-r31 and counts in r26, where the original adds 0x14 inside
            #   the loop (0x80112548), saves r27-r31 and counts in r27. Positional: 217 of the first
            #   226 words differ and this build has 1 more; aligned: 11 paired-differing, 1
            #   original-only, 2 source-only.
            # Data: once the linker-discarded weak and local copies are set aside, .rodata, .data,
            # .sdata, .sbss and .sdata2 match the original by contents, up to section-end
            # alignment; the original's .sdata2 is allocatable (A, 0x2) and this object's writable
            # and allocatable (WA, 0x3).
            Object(NonMatching, "Game/Ring.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Rocket.cpp", cflags=cflags_game_cpp),
            # 100 of the 104 functions are identical in every word and relocation target (local
            # names resolved). The four others, compared word by word (USA, GUPE8P):
            # - OnCancel (0x80117614) is 23 words against 21: the build saves and restores r31 and
            #   loads the 1 it stores at 0x30 of fn_800BADE8's object into r31 before that call,
            #   where the original loads it into r0 after the call. Position by position 17 of the
            #   21 words differ and 2 build words are unpaired; aligned, 2 words differ (li, stw)
            #   and 2 are build-only (stw/lwz r31).
            # - The function1<void, SavePointSelect::Args &> copy assignment is a 12-word function
            #   in the original (fn_8011806C, 0x8011806C) that calls the 22-word body
            #   (0x8011809C); boost/function.hpp has the body only, so UpdateBody's two
            #   assignments (UpdateBody+0x1CC, +0x258) call the body directly (2 relocation
            #   targets differ) and the 12-word function has no counterpart.
            # - That function1's swap (0x801180F4) calls a 32-byte local copy of
            #   std::swap<any_pointer> in this object, where the original calls the kept copy at
            #   0x80019164 (1 relocation target; the local copy is build-only, and a source-enabled
            #   link keeps it, at 0x8011812C in USA).
            # The ten tree helpers of the two std::map members of Unk801A2AF4 (__destroy,
            # deallocate, __node_alloc, destroy, __alloc) follow the trees' destructors here and
            # come last, before the static initialiser, in the original; a non-inline __destroy in
            # the shared map header gives the original order but changes five other objects.
            # Compared by contents, with the 85 copies that a source-enabled link discards set
            # aside (55 weak and 3 local functions; 3 weak vtables in .data, which the original
            # keeps as zero-filled holes; 8 .rodata, 7 .sdata and 9 .sbss locals), every original
            # data object matches in bytes, relocations and place; the retained local std::swap
            # copy above is not one of them. The compiled .bss and .sdata2 are 0x7C and 0x1C
            # bytes, 4 bytes shorter than the split's 0x80 and 0x20; in the source-enabled link
            # the next object starts 4 bytes after each of them (alignment padding), so there they
            # take 0x80 and 0x20 bytes, as in the split. .sdata2 is writable in the compiled
            # object where the split's is read-only.
            Object(NonMatching, "Game/SavePoint.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/SetCharacter.cpp", cflags=cflags_game_cpp),
            # Relocatable comparison, the same in USA (GUPE8P), Japan (GUPJ8P) and PAL (GUPP8P): 45
            # of the 49 functions of the original split have the original's words and relocation
            # targets. The constructor calls the out-of-line assignment of UnkRingParams_8011B170
            # (0x8011B0F0 in USA, 32 words) where this build expands it in place: the constructor is
            # 203 words against 184 (aligned: 19 words only in this build, none only in the
            # original, 3 paired words differ; positionally 150 of the 184 common words differ). Of
            # the 3, one is the call, whose target is Fn8040Vector3's assignment instead of the
            # record's; the other two load the address of @92 at the start of .data, which this
            # build relocates against the section's start symbol (same address and bytes). The
            # record's assignment has no counterpart. Create (124 words) loads record.num (lwz
            # r0,0xC(r24)) and compares it before the three stores of the speed vector where the
            # original stores first: 8 words differ positionally; aligned, 2 words are only in the
            # original and 2 only here. OnCommand (32 words) copies the rotation component by
            # component, storing each one right after its load, where the original loads all three
            # (0x68, 0x64, 0x60) before the first store: 8 words differ positionally; aligned, 3
            # paired words differ, 2 are only in the original and 2 only here, with the same stack
            # slot and the same call to fn_80238A78. Data, by contents with the 86 compiled symbols
            # absent from the original split set aside (27 local, 59 weak), matches in .rodata,
            # .data (the original has zero-filled words where the compiled object holds five weak
            # vtables), .sdata, .sbss and .sdata2. The compiled .rodata, .sdata and .sbss (0x354,
            # 0x84 and 0x3D bytes; 0x22F, 0x44 and 0x1D without the set-aside symbols) end without
            # the original's section-end alignment (0x230, 0x48 and 0x20), and the compiled .sdata2
            # is writable and allocatable (WA, 0x3) where the original's is allocatable (A, 0x2).
            Object(NonMatching, "Game/ScatteringRing.cpp", cflags=cflags_game_cpp),
            # 12 of the 13 functions are identical in every word and relocation. The data contents
            # and relocations match once the 44 linker-discarded copies (9 local, 35 weak) are set
            # aside, with these differences in USA (GUPE8P), Japan (GUPJ8P) and PAL (GUPP8P): the
            # original's .sdata2 is allocatable (A, flags 0x2) and the compiled one writable and
            # allocatable (WA, 0x3), and the compiled sections end without the original's alignment
            # padding (.rodata 0x3F bytes against 0x40, .data 0x64 against 0x68, .sbss 0x4 against
            # 0x8). Because RemoveLocks is 16 bytes shorter, the functions after it (UpdateBody, the
            # destructor, the constructor, RemoveLockPart, a weak_ptr constructor and the static
            # initialiser) start 0x10 lower in .text. RemoveLocks (USA 0x8011B754) is 14 words
            # against the original's 18: the original stores zero into both members of its
            # part-walk context (0x8011B76C and 0x8011B770) and then stores the arguments over
            # them, where this build stores only the arguments, in another order. Positional (word
            # i against word i): 12 of the 14 common words differ and the original's last 4 have
            # no counterpart; aligned (instructions matched up): no pair differs, 6 words are only
            # in the original and 2 only in this build.
            Object(NonMatching, "Game/SecretDoor.cpp", cflags=cflags_game_cpp),
            # Relocatable comparison (compiler literals paired by contents), the same in USA
            # (GUPE8P), Japan (GUPJ8P) and PAL (GUPP8P): 33 of the 34 functions of the original
            # split have the original's instructions and relocation targets. UpdateBody (USA
            # 0x8011C1E0) searches the key list with a flag that is cleared before the loop and set
            # at a match; the original sets 1 at the match and 0 when the loop ends, then tests that
            # value. Both are 61 words with the same relocation targets. Positional (word i against
            # word i): 12 words differ (0x8011C234-0x8011C264); aligned (instructions matched up): 7
            # pairs differ, 3 words are only in the original and 3 only in this build. The data
            # contents match once the 68 linker-discarded copies (50 weak, 18 local) are set aside,
            # with these differences: the original's .sdata2 is allocatable (A, flags 0x2) and the
            # compiled one writable and allocatable (WA, 0x3); the compiled .rodata, .bss, .sdata
            # and .sbss end without the original's alignment padding (0xF2 against 0xF8, 0x14
            # against 0x18, 0x24 against 0x28, 0xD against 0x10); and the RTTI name strings are
            # pooled in another order: the original pools "Task", "Gadget" and
            # "SecretKey::SecretKey" after the constructor's "col" and "mtn" and after the
            # allocator's "Memory allocation failure", where this build pools them first. Linked in
            # place of the original object (USA), with every function the same size, 38 words
            # differ: 16 in .text (the 12 UpdateBody words, 0x8011C588 in the allocator, 0x8011C76C,
            # 0x8011C814 and 0x8011C828 in the constructor), 13 in .rodata (0x804BD370-0x804BD3A0),
            # 3 in .sdata (the name words of the Task, Gadget and SecretKey RTTI records) and 6 in
            # .sdata2 (0x805F3D38-0x805F3D4C). A USA test build with a second class, whose virtual
            # destructor is defined after SecretKey's members, gives the original's string order,
            # leaving only the 12 UpdateBody words different. The PAL RC1 prototype has the RTTI
            # name "SecretKey::VacuumSecretKey" (0x804BA15C), whose record a table references; the
            # retail executables lack that name. That a class removed from retail explains the
            # pool order is inferred.
            Object(NonMatching, "Game/SecretKey.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/SetCollision.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/SetParticle.cpp", cflags=cflags_game_cpp),
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
            # All 31 functions have identical instructions, and the other sections match by
            # contents with the discarded weak and local copies set aside (50 weak, 18 local).
            # .sdata2 holds the same literals in another order: the original has 180.0f
            # (UpdateBody's angle limit) at +0x64, before 360.0f, 4 bytes of padding and the
            # int-to-float constant (0x80 bytes); this object creates 180.0f after them (+0x70,
            # no padding, 0x7C bytes), so the loads of those three constants and of "col" and
            # "mtn" (CopyFromOwner, UpdateBody, constructor) have other offsets. The original order
            # fits an out-of-line copy of an inline degrees-to-radians function (the body at
            # 0x8000AB90) emitted after UpdatePosition and discarded by the linker, where this
            # source calls fn_8000AB90 as an external function (inferred).
            Object(NonMatching, "Game/TargetSwitch.cpp", cflags=cflags_game_cpp),
            # 59 of the 60 functions have identical instructions and relocations, and the data
            # matches once the linker-discarded weak and local copies are set aside.
            # UpdateModelMatrix is 122 words against the original's 123 (USA, GUPE8P): position by
            # position 46 words differ and the original's last word has no counterpart. The
            # original loads the segment pointer m_line into r31 before each of the two transform
            # calls (0x80127940 and 0x80127954) and keeps the object in r30; this build reads the
            # pointer after each call, so it saves one register fewer and the object is in r31.
            # The functions after it start 4 bytes earlier, so the .ctors entry and three .data
            # relocations to them (OnCollision, UpdateBody and the destructor) have addends 4 lower.
            Object(NonMatching, "Game/UpDownReel.cpp", cflags=cflags_game_cpp),
            # 33 of the 35 functions have identical instructions and relocations. By contents,
            # with the 95 weak and local copies this object has and the original does not set
            # aside (75 weak, 20 local), the data, symbols and relocations match, except four
            # relocations that point 8 or 4 bytes lower: the jump table's entries for states 5 and
            # 6 point into the tail of UpdateBody, and the vtable's destructor entry and the
            # .ctors entry point after it. UpdateBody (USA (GUPE8P) 0x80122580) is 259 words
            # against the original's 261: in each of its two weapon put-down blocks the original
            # loads the "weapon" parameter into r30 before the position's copy construction
            # (0x800091EC) and moves it to r3 for fn_8011E938, where this build loads it into r3
            # just before that call, so each block is one word longer in the original. In the
            # relocatable objects 54 of the 259 common words differ positionally: 8 earlier
            # branches to the code after the switch, and 46 of the 48 words from the branch before
            # the first block to the end, which this build has one word earlier after the first
            # block and two after the second; 15 of the 54 original words are branches. The constructor (0x80122C5C) differs in 2 of its 226 words: for its
            # call of GadgetModelInfo::fn_801629D8 with the crash information's fn_802087D8 name,
            # the original loads m_crashInfo's pointer before m_model's and this build loads them
            # the other way round (the same instructions, registers and relocations otherwise), as
            # in BreakWall's and SuperComputer's constructors.
            Object(NonMatching, "Game/Stick.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/TurnOver.cpp", cflags=cflags_game_cpp),
            # 51 of 54 functions are identical in every word and relocation in USA, Japan and PAL.
            # The data contents and relocations match once the 97 linker-discarded copies (33 local,
            # 64 weak) are set aside, with two differences in every region: the original's .sdata2
            # is allocatable (A, flags 0x2) and the compiled one writable and allocatable (WA, 0x3),
            # and the compiled sections end without the original's alignment padding (.sdata2 0x1B
            # bytes against 0x20). Three functions keep their length but differ in register use or
            # order (positional: word i against word i; aligned: instructions matched up):
            # - SetWarpHole::UpdateBody (0x8012A1F8, 40 words): the original loads m_comId into a
            #   second saved register before it calls GetInstance (0x8012A24C) and saves r30-r31
            #   with stmw/lmw; compiled, the load comes after the call and the function saves only
            #   r31, so the this register differs in every member access: 13 words differ by
            #   position; aligned, 9 pairs differ and 2 words are only in each.
            # - The ManagerTask destructor (0x80129B50, 52 words; 51 in Japan and PAL): the original
            #   loads the 1 for field_0x30 of the fn_800BADE8 object after that call (0x80129BD8);
            #   compiled, it is loaded into a saved register before the call: 5 words differ by
            #   position; aligned, 1 pair differs and 2 words are only in each.
            # - ManagerTask::Update (0x80129640, 214 words): the same store at its start, made when
            #   fn_802E5810 is false (0x80129668): 3 words differ by position; aligned, 1 pair
            #   differs and 1 word is only in each.
            Object(NonMatching, "Game/WarpHole.cpp", cflags=cflags_game_cpp),
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
            # With the weak and local copies the linker discards set aside, all 56 functions have
            # identical relocations and 54 have identical instructions. The original's .bss is 0xF8
            # bytes with the three static size vectors at 0xC, 0x18 and 0x24. ApplyScale and the
            # static initialiser form the pooled base address 0x8057FFA8 (USA), the first byte of
            # the 0xC bytes before the vectors, but no instruction in the file reads or writes those
            # 0xC bytes or the 0xC8 bytes after the vectors, and what occupies them is unknown, so
            # the .bss here is 0x24 bytes with the vectors at 0x0, 0xC and 0x18. ApplyScale and the static initialiser address the vectors from the pooled
            # .bss base, so ApplyScale differs in three words (USA 0x8012AF58, 0x8012AFD8,
            # 0x8012B030) and the static initialiser in six (0x8012C8C4, 0x8012C8CC, 0x8012C8D0,
            # 0x8012C8D8, 0x8012C8E4, 0x8012C8F0), each a displacement 0xC lower. Compared by
            # contents with the discarded copies set aside (the dropped weak vtables as zero-filled
            # holes), .sdata2, .sbss and .ctors match. The original also has the RTTI base lists of
            # Player::PlayerCommand and of an unnamed class derived from it (0x20 bytes at
            # .data+0x26C) and a zero-filled hole 0x18 bytes longer after them, the CharCommand and
            # Player::PlayerCommand RTTI records (.sdata+0x4C, 0x10 bytes) and their names
            # (.rodata+0x24C, 0x24 bytes). No instruction in the file refers to these records, and
            # the declarations that emitted them are not identified, so the boost::bad_weak_ptr base
            # list, the std::exception RTTI record and its name sit lower here (.data 0x2B0, .sdata
            # 0x54 and .rodata 0x25B bytes against 0x2E8, 0x68 and 0x280).
            Object(NonMatching, "Game/Weight.cpp", cflags=cflags_game_cpp),
            # Relocatable comparison (local names resolved), the same in USA (GUPE8P), Japan
            # (GUPJ8P) and PAL (GUPP8P): 69 of the 71 functions of the original split have identical
            # instruction words, and all but function1<void, GadgetColliDetectArgs &>::swap have the
            # original's relocation targets.
            # - The hit callback (USA 0x800F5FAC) is 121 words against 120: it reloads the span's
            #   address for fn_8040CF50, where the original passes on the pointer that the span's
            #   copy constructor returns (as in ElecBar's hit callback). Positional (word i against
            #   word i): 73 of the 120 common words differ and 1 build word is unpaired; aligned
            #   (instructions matched up): 4 pairs differ (branches past the extra word), no word is
            #   only in the original and 1 only in this build.
            # - CatchStick::UpdateLine (USA 0x800F4CE4) is 60 words against 61: the original loads
            #   m_line into r31 before each matrix-vector product (0x800F4D84 and 0x800F4DA4), so it
            #   keeps this in r30 and saves r30-r31 with stmw/lmw; through the matrix-vector
            #   operator*, the compiled code loads m_line from this (r31) after each product and
            #   saves only r31. Positional: 27 of the 60 common words differ and 1 original word is
            #   unpaired; aligned: 9 paired-differing, 3 original-only and 2 source-only words.
            # - The swap calls a 32-byte local copy of std::swap<any_pointer> in this object, where
            #   the original calls the kept copy at 0x80019164 (1 relocation target; the local copy
            #   is build-only, and a source-enabled link keeps it).
            # - The function order differs in two places (see the note in the source): fn_800F5980
            #   and the four function1<void, GadgetColliDetectArgs &> members it calls (0x188 bytes)
            #   follow the constructor's other out-of-line functions instead of coming after the
            #   sp_counted_base_impl constructor, and the TouchLineCommand constructor follows the
            #   weak_ptr<CharInfo> conversion after the hit callback instead of preceding it.
            # Data, compared by contents with the 94 copies that a source-enabled link discards set
            # aside (61 weak and 2 local functions; 5 weak vtables in .data, which the original
            # keeps as zero-filled holes; 10 .rodata, 8 .sdata and 8 .sbss locals): every original
            # object matches in bytes, relocations and place except the last two .rodata strings,
            # the RTTI names of the segment's sp_counted_base_impl and checked_deleter. The original
            # names the segment class Sonicteam::System::Geometry::Line (0x84 and 0x3A bytes), where
            # UnkSegment_80020A20 gives 0x68 and 0x2C, so .rodata is 0x2C bytes shorter and the
            # checked_deleter type record's name pointer moves with it. The sp_counted_base_impl
            # vtable names the original's functions, which sit at other .text offsets because of the
            # moves above. The compiled .data, .sdata, .sbss and .sdata2 end without the original's
            # alignment padding (0x1C4 against 0x1C8, 0x74 against 0x78, 0x21 against 0x28, 0x34
            # against 0x38), and .sdata2 is writable in the compiled object where the split's is
            # read-only.
            Object(NonMatching, "Game/CatchStick.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_8005A300.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80054DD0.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80096B64.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_800532F8.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/fn_80422070.cpp", cflags=cflags_effect),
            Object(Matching, "Game/fn_804220CC.cpp", cflags=cflags_effect),
            # These slices cover USA (GUPE8P) 0x80425170-0x8042631C.
            # Original Remove and Create calls reach the tail's operator== and shared/weak pointer
            # constructors, which the current compiler emits locally in EffectManager.cpp. This
            # suggests a common original file; the original pre-link bindings and filename are
            # unknown. Keep the eight-byte Matching factory separate until the whole object matches.
            Object(Matching, "Game/fn_80425858.cpp", cflags=cflags_effect),
            # The six method bodies have identical instruction words and named call targets.
            # Their local template copies correspond to bodies in the original tail range, so
            # retaining the original tail does not replace these local definitions. The fifteen tail
            # exception records occur across both source objects; only the shared_count action
            # padding differs (original 0x0255). Original RTTI, vtables and the one-byte deleter
            # tag at USA 0x805E1500 remain in separate automatic data units.
            Object(NonMatching, "Game/EffectManager.cpp", cflags=cflags_effect),
            # Of the nine named bodies, eight have identical instruction words. The 748-byte
            # loader differs in four words that set up strncmp's arguments. The unit's EFFD string,
            # float literals and jump-table targets agree by contents. The other nine original
            # bodies are Boost helpers emitted by EffectManager.cpp; their instruction words
            # agree, but the separate objects do not reproduce the original code/data ownership.
            # The compiled object orders fn_80425C34 through fn_80425E68 before the loader;
            # the EffectManager destructor and constructor come last, after the EffectElement
            # constructor.
            Object(NonMatching, "Game/fn_80425860.cpp", cflags=cflags_effect),
            Object(Matching, "Game/fn_8042A2D8.cpp", cflags=cflags_effect),
            Object(Matching, "Game/EffectGlobalParam.cpp", cflags=cflags_effect),
            Object(Matching, "Game/EffectLensFlare.cpp", cflags=cflags_effect),
            Object(Matching, "Game/EffectShimmer.cpp", cflags=cflags_effect),
            # Update retains six register-word differences; the other seven bodies have
            # identical instruction words.
            Object(NonMatching, "Game/EffectLocus.cpp", cflags=cflags_effect),
            # Matching workaround: Render uses six independent literal-address pairs;
            # pooling introduces a shared base. Revisit when this file's original
            # compiler settings are recovered.
            # Render retains 132 paired word differences with equal 2148-byte bodies;
            # its frame, register allocation and vertex scheduling still differ.
            # Update retains five paired differences and ten native tail words
            # (source 32 bytes, native 72), without the original terminal tests,
            # which perform no further operation.
            Object(NonMatching, "Game/EffectRayRW3.cpp", cflags=cflags_effect),
            # Matching workaround: separate literal addresses include lis/addi pairs; pooling
            # shares a base. It changes the exact filter from 1380 to 1272 bytes, capture from
            # 1732 to 1636, constructor from 800 to 784, and compositor from 4232 to 4044 bytes.
            # Revisit when this file's original compiler settings are established.
            # The compositor lacks three address-add instructions and retains register/scheduling
            # differences (4232 source bytes versus 4244 original bytes); the other 14 bodies match.
            Object(NonMatching, "Game/fn_8042E208.cpp", cflags=cflags_effect),
            # Matching workaround: the constructor and compositor address literals independently;
            # pooling introduces a shared rodata base. Revisit when the original flags are known.
            Object(Matching, "Game/fn_8042D204.cpp", cflags=cflags_effect),
            # The vertex builder retains address-materialization, stack and instruction-layout
            # differences (2564 source bytes versus 2568 original bytes). Its exception
            # record and the parameter constructor's exception-record order also differ.
            Object(NonMatching, "Game/fn_80430A54.cpp", cflags=cflags_effect_rw3),
            # All ten relocatable bodies match, but the weak delete helper precedes the
            # destructor and constructor instead of following them. Text placement and
            # exception-index order differ; the kept text extent remains 1748 bytes.
            Object(NonMatching, "Game/EffectManagerRW3.cpp", cflags=cflags_effect_rw3),
            Object(Matching, "Game/EffectObjectRW3.cpp", cflags=cflags_effect_rw3),
            # Matching workaround: the EffectParticle3DRW3 constructor addresses each float
            # literal independently. Pooling adds a shared base and shortens it by 64 bytes.
            # Revisit when this original file's compiler settings are recovered.
            Object(NonMatching, "Game/fn_80432D58.cpp", cflags=cflags_effect),
            Object(Matching, "Game/fn_804340CC.cpp", cflags=cflags_effect_rw3),
            # Render retains 656 paired instruction-word differences and one source tail
            # word (source 3660 bytes, native 3656). The Data48 vertex builder retains 419
            # paired differences and two native tail words (source 1956 bytes, native 1964).
            # Both retain instruction-layout and register differences. The Shimmer ctor differs
            # by one instruction; fn_80436128 retains seventeen register-word differences.
            Object(NonMatching, "Game/fn_804342CC.cpp", cflags=cflags_effect_rw3),
            # Matching workaround: these two units address literals independently in the
            # original. Keep unpooled addressing until their original build options are known.
            # fn_804372A8 retains six register-word differences; fn_80437118 (400 bytes)
            # and the manager wrapper (44 bytes) have identical instruction words.
            # fn_804376EC is 1312 versus 1320 bytes, with 205 common-position word
            # differences and two unpaired original words. Scheduling and address selection
            # differ: the original uses two additional addi instructions for the 0.5f and
            # unsigned-conversion-bias addresses, which this build folds into loads.
            # The cause of the address-selection difference remains unresolved.
            Object(NonMatching, "Game/fn_80437118.cpp", cflags=cflags_effect),
            # Nine function bodies match. fn_80437C40 is 2584 bytes on both sides,
            # with 57 differing instruction words within +0x19C..+0x304 from literal
            # addressing, scheduling and register allocation. Eight literal relocation
            # sites move; their targets are unchanged.
            Object(NonMatching, "Game/fn_80437C40.cpp", cflags=cflags_effect),
            Object(Matching, "Game/EffectGlobalParamRW3.cpp", cflags=cflags_effect_rw3),
            # Four functions match; the vertex builder is 2556 versus 2736 bytes, with
            # vertex-copy, stack and instruction-layout differences (the original also copies the
            # point's position into a local component by component, where this build calls the
            # vector's copy constructor).
            Object(NonMatching, "Game/EffectLocusRW3.cpp", cflags=cflags_effect_rw3),
            Object(Matching, "Game/fn_80008D48.c"),
            Object(Matching, "Game/fn_80008DA4.c"),
            Object(Matching, "Game/fn_80008E1C.c"),
            Object(Matching, "Game/fn_800091D0.cpp"),
            Object(Matching, "Game/WeaponStickBase.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/FallBuildingHolder.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ChaoHolder.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/Cream.cpp", cflags=cflags_game_cpp),
            # Relocatable-object comparison: all functions but UpdateBody match, words and
            # relocations. UpdateBody is 65 words against 66: after the compare tree of the switch
            # on the state the original has an unreachable branch to the end of the switch (the
            # code of a case label with no statements, which this source does not have). By
            # position 54 of the 66 words differ (every word after it shifts by one). Pairing the
            # compare tree's branches by control flow, the four reachable branches (beq, two bge
            # and b) differ only in their displacements and the unreachable b (USA 0x801F7A90) is
            # original-only; every other word matches. Relocation targets are unchanged. Normal
            # builds link the original object.
            Object(NonMatching, "Game/ElecCircuit.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecCircuitEntrance.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecCristalWallSwitch.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecPillar.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecSmallPillar.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecWakka.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/GravityChangeCollision.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/GravitySignBoard.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/SpaceGadget.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BackDrop.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/EscapePlane.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/EscapePlaneRail.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/ElecCircuitExit.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/TestNakazawa.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BDHologram.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/BDTrue.cpp", cflags=cflags_game_cpp),
            Object(Matching, "Game/CSD/fn_80360528.cpp", cflags=cflags_csd),
            # Current file system and its directory (original file name unknown).
            Object(Matching, "Game/CSD/fn_80361738.cpp", cflags=cflags_csd),
            Object(Matching, "Game/CSD/fn_80361A24.cpp", cflags=cflags_csd),
            Object(Matching, "Game/CSD/fn_80361CF8.cpp", cflags=cflags_csd),
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
            # The units below that use -Cpp_exceptions on are those whose original object
            # carries extab/extabindex entries (C++ exception tables), plus Game/fn_8041583C.c,
            # the middle piece of a file whose other pieces carry them; the default is off.
            Object(
                Matching,
                "Game/fn_8040ABC8.cpp",
                cflags=cflags_game_cpp,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            # Two-component vector helpers. Their length helper fn_8040CCEC, like fn_8040CF50
            # (Game/fn_8040CD74.cpp) and Game/fn_8041116C.cpp, calls std::sqrt(float) from MSL
            # <cmath>. One level of inlining keeps that call out of line, because std::sqrt's
            # body calls the inline sqrtf; plain -inline auto expands both into the helper. Each
            # object also carries weak copies of std::sqrt and sqrtf with their literals, which
            # the link discards in favour of the copies at 0x8000FEE4 and 0x8000FF04. Those
            # copies match <cmath> compiled with fp_contract on (cflags_base).
            Object(
                Matching,
                "Game/fn_8040CC5C.cpp",
                cflags=cflags_game_cpp,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            # Three-component vector helpers, with the same flags as the two-component ones: the
            # length helper fn_8040CF50 calls std::sqrt(float) in the same way.
            Object(
                Matching,
                "Game/fn_8040CD74.cpp",
                cflags=cflags_game_cpp,
                extra_cflags=["-Cpp_exceptions on"],
            ),
            Object(
                Matching,
                "Game/fn_8040D044.cpp",
                # Matching workaround: the original saves its two callee-saved registers with
                # stmw/lmw, which this unit's code only gets when optimized for size with
                # -use_lmw_stmw; revisit when the original unit's flags are known.
                extra_cflags=["-Cpp_exceptions on", "-O4,s", "-use_lmw_stmw on"],
            ),
            Object(Matching, "Game/fn_8040D498.c"),
            Object(Matching, "Game/fn_8040D608.c"),
            Object(Matching, "Game/fn_8040D9D8.c"),
            Object(Matching, "Game/fn_8040DB6C.c"),
            # CRI middleware functions (MWSFD); the original source file names are not yet
            # established.
            Object(Matching, "Game/fn_8040E048.c"),
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
            # The middle piece of the control of motions (see Game/fn_804157E8.cpp
            # above); it takes the same flags as the other two pieces.
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
            Object(Matching, "Game/fn_8041FD6C.c"),
            Object(Matching, "Game/fn_8041FF08.c"),
            Object(Matching, "Game/fn_804202B8.c"),
            Object(Matching, "Game/fn_80420438.c"),
            # Support code of the same library after the quaternion helpers: the start-up and
            # shut-down wrappers with a name registry and the ARAM transfers and allocator. The
            # byte reversal between them (Game/fn_804210E4.c) is in the USA-only list above.
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
            Object(Matching, "Game/EffectArranger.cpp", cflags=cflags_effect),
            Object(Matching, "Game/gcaxSndMemory.c", cflags=cflags_game_gcax),
            Object(Matching, "Game/fn_8043B25C.c", cflags=cflags_game_gcax),
            Object(Matching, "Game/fn_8043D7B4.c", cflags=cflags_game_gcax),
            Object(Matching, "Game/fn_8043E2B4.c", cflags=cflags_game_gcax),
            Object(Matching, "Game/fn_8043FB58.c", cflags=cflags_game_gcax),
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
