from conan import ConanFile
from conan.errors import ConanException
from conan.tools.apple import is_apple_os
from conan.tools.files import apply_conandata_patches, export_conandata_patches, get, copy, replace_in_file, rmdir
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.scm import Version
import os


required_conan_version = ">=2.1"


class NsyncConan(ConanFile):
    name = "nsync"
    homepage = "https://github.com/google/nsync"
    description = "Library that exports various synchronization primitives"
    license = "Apache-2.0"
    url = "https://github.com/conan-io/conan-center-index"
    topics = ("c", "thread", "multithreading", "google")
    package_type = "library"
    settings = "os", "arch", "compiler", "build_type"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    def export_sources(self):
        export_conandata_patches(self)

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self, src_folder="src")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["NSYNC_ENABLE_TESTS"] = False
        if self.settings.os == "Windows" and self.options.shared:
            tc.variables["CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS"] = True
        # Relocatable shared libs on macOS
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0042"] = "NEW"
        tc.cache_variables["CMAKE_POLICY_VERSION_MINIMUM"] = "3.5" # CMake 4 support
        if Version(self.version) > "1.29.2": # pylint: disable=conan-unreachable-upper-version
            raise ConanException("CMAKE_POLICY_VERSION_MINIMUM hardcoded to 3.5, check if new version supports CMake 4")
        tc.generate()

    def _patch_sources(self):
        apply_conandata_patches(self)

        replace_in_file(
            self,
            os.path.join(self.source_folder, "CMakeLists.txt"),
            "set (CMAKE_POSITION_INDEPENDENT_CODE ON)",
            ""
        )

        if self.settings.os == "Windows" and self.options.shared:
            ar_dest = \
                "ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} " \
                "COMPONENT Development"
            rt_dest = 'RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"'
            replace_in_file(
                self,
                os.path.join(self.source_folder, "CMakeLists.txt"),
                f"{ar_dest})",
                f"{ar_dest}\n{rt_dest})"
            )

        if is_apple_os(self):
            # Following the Darwin codepath when the OS is an apple OS (e.g. iOS)
            # matches https://github.com/google/nsync/blob/99e0e9e1e32db42e88b98b96e874f4884620b755/BUILD#L131
            # (this is best effort)
            replace_in_file(self, os.path.join(self.source_folder, "CMakeLists.txt"),
             'elseif ("${CMAKE_SYSTEM_NAME}X" STREQUAL "DarwinX")',
             'elseif(1)')

    def build(self):
        self._patch_sources()
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, pattern="LICENSE", dst=os.path.join(self.package_folder, "licenses"), src=self.source_folder)
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "lib", "cmake"))

    def package_info(self):
        self.cpp_info.components["nsync_c"].libs = ["nsync"]
        self.cpp_info.components["nsync_cpp"].libs = ["nsync_cpp"]
        if self.settings.os in ["Linux", "FreeBSD"]:
            self.cpp_info.components["nsync_c"].system_libs.append("pthread")
            self.cpp_info.components["nsync_cpp"].system_libs.extend(["m", "pthread"])
