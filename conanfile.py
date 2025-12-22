from conan import ConanFile
from conan.tools.cmake import cmake_layout

class FloodMonitorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("libcurl/8.17.0")
        self.requires("simdjson/4.2.2")
        self.test_requires("gtest/1.17.0")

    def layout(self):
        cmake_layout(self)
