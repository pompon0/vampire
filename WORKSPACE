load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

###############################
# LLVM

http_archive(
  # cannot change this name, because files in its workspace refer to it
  name = "com_grail_bazel_toolchain",
  strip_prefix = "bazel-toolchain-e608913c0e106da931234fdd37de9ee37e0b2541",
  urls = ["https://github.com/pompon0/bazel-toolchain/archive/e608913c0e106da931234fdd37de9ee37e0b2541.tar.gz"],
  sha256 = "e09e8ef8a5f97078da2961561e176a5bf461962683159bcbd81674052475cdd0",
)

load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
  name = "llvm_toolchain",
  distribution = "clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz",
  llvm_version = "10.0.0",
)

load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")

llvm_register_toolchains()


