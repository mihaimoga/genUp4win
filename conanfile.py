from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy
import os


class GenericUpdaterConan(ConanFile):
    name = "genericupdater"
    version = "3.03"
    license = "MIT"
    author = "Stefan-Mihai MOGA"
    url = "https://github.com/mihaimoga/genUp4win"
    description = "Generic Updater Library for Windows Applications"
    topics = ("updater", "windows", "library")
    settings = "os", "compiler", "build_type", "arch"

    options = {"shared": [True, False], "fPIC": [True, False]}

    default_options = {"shared": True, "fPIC": True}

    exports_sources = (
        "CMakeLists.txt",
        "genUp4win/*",
        "DemoApp/*",
        "*.h",
        "*.cpp",
        "*.rc",
    )

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(
            self,
            "LICENSE*",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["genUp4win"]
        self.cpp_info.includedirs = ["include"]

        if self.settings.os == "Windows":
            self.cpp_info.system_libs = ["urlmon"]
            self.cpp_info.defines = ["UNICODE", "_UNICODE"]

        if self.settings.build_type == "Debug":
            self.cpp_info.defines.append("_DEBUG")
        else:
            self.cpp_info.defines.append("NDEBUG")
