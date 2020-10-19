COPTS = [
  "-std=c++11",
  "-Wno-return-type",
  "-Wno-switch",
  "-Wno-unused-result",
  "-Wno-unused-value",
  "-Wno-unused-variable",
  "-Wno-uninitialized",
  "-Wno-narrowing",
  "-Wno-strict-aliasing",
]

DEFINES = [
  "VDEBUG=0",
  "GNUMP=0",
  "VZ3=0",
  "__STDC_LIMIT_MACROS",
  "__STDC_FORMAT_MACROS",
]

genrule(
  name = "version",
  outs = ["version.cpp"],
  cmd = "echo \'const char* VERSION_STRING = \"Vampire 4.4.0 (commit 3267e536 on 2019-11-19 16:17:57 +0000)\";' > $@",
)

cc_library(
  name = "semop_gvisor",
  hdrs = ["semop_gvisor.h"],
)

cc_library(
  name = "lib",
  copts = COPTS,
  # bazel now doesn't allow to add system root to includes and I'm definitely not fixing
  # all the imports in the vampire repo. As a workaround we use isystem flag directly.
  tags = ["manual"],
  defines = DEFINES,
  hdrs = glob([
    "Forwards.hpp",
    "Debug/**/*.hpp","Debug/**/*.h",
    "DP/**/*.hpp","DP/**/*.h",
    "FMB/**/*.hpp","FMB/**/*.h",
    "Indexing/**/*.hpp","Indexing/**/*.h",
    "Inferences/**/*.hpp","Inferences/**/*.h",
    "InstGen/**/*.hpp","InstGen/**/*.h",
    "Kernel/**/*.hpp","Kernel/**/*.h",
    "Lib/**/*.hpp","Lib/**/*.h",
    "Minisat/**/*.hpp","Minisat/**/*.h",
    "Parse/**/*.hpp","Parse/**/*.h",
    "SAT/**/*.hpp","SAT/**/*.h",
    "Saturation/**/*.hpp","Saturation/**/*.h",
    "Shell/**/*.hpp","Shell/**/*.h",
  ]),
  srcs = ["version.cpp"] + glob([
    "Global.cpp",
    "Debug/**/*.cpp","Debug/**/*.cc",
    "DP/**/*.cpp","DP/**/*.cc",
    "FMB/**/*.cpp","FMB/**/*.cc",
    "Indexing/**/*.cpp","Indexing/**/*.cc",
    "Inferences/**/*.cpp","Inferences/**/*.cc",
    "InstGen/**/*.cpp","InstGen/**/*.cc",
    "Kernel/**/*.cpp","Kernel/**/*.cc",
    "Lib/**/*.cpp","Lib/**/*.cc",
    "Minisat/**/*.cpp","Minisat/**/*.cc",
    "Parse/**/*.cpp","Parse/**/*.cc",
    "SAT/**/*.cpp","SAT/**/*.cc",
    "Saturation/**/*.cpp","Saturation/**/*.cc",
    "Shell/**/*.cpp","Shell/**/*.cc",
  ],exclude=[
    "Minisat/core/Main.cc",
    "Minisat/simp/Main.cc",
    "Indexing/FormulaIndex.cpp",
    "Inferences/SLQueryForwardSubsumption.cpp",
    "Inferences/ArrayTheoryISE.cpp",
    "Inferences/RefutationSeekerFSE.cpp",
    "Parse/SMTLIB.cpp",
    "SAT/TransparentSolver.cpp",
    "SAT/SingleWatchSAT.cpp",
    "Shell/LTB/**/*", 
    "Shell/EqualityAxiomatizer.cpp", 
    "Shell/Profile.cpp",
    "Shell/ProofSimplifier.cpp",
  ]),
  visibility = ["//visibility:public"],
  deps = [
    ":semop_gvisor",
  ],
)

cc_library(
  name = "api",
  copts = COPTS,
  defines = DEFINES,
  hdrs = glob(["Api/**/*.hpp","Api/**/*.h"]),
  srcs = glob(["Api/**/*.cpp","Api/**/*.cc"],exclude=[
      "Api/Problem.cpp",
      "Api/FormulaBuilder.cpp",
      "Api/Helper.cpp",
      "Api/Helper_Internal.cpp",
  ]),
  deps = [
    ":lib",
  ],
)

cc_library(
  name = "casc",
  # Schedule.cpp takes clang 15min to compile with -O3/-O2.
  # TODO: do sth about it
  defines = DEFINES,
  hdrs = glob(["CASC/**/*.hpp","CASC/**/*.h"]),
  srcs = glob(["CASC/**/*.cpp","CASC/**/*.cc"]),
  deps = [":lib"],
)

cc_binary(
  name = "vampire",
  srcs = ["vampire.cpp"], 
  deps = [":lib",":casc"],
  copts = COPTS,
  defines = DEFINES,
  visibility = ["//visibility:public"],
  # requires CLANG to work correctly
  features = ["fully_static_link"],
)
