from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeToolchain

class FloodMonitorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("libcurl/8.17.0")
        self.requires("nlohmann_json/3.12.0")
        #self.requires("qt/6.10.1")
        self.test_requires("gtest/1.17.0")

    '''def configure(self):
        self.options["qt"].with_pq = False
        self.options["qt"].with_odbc = False
        self.options["qt"].with_sqlite3 = False
        self.options["qt"].qtdeclarative = True
        self.options["qt"].qtpositioning = True
        self.options["qt"].qtlocation = False'''

    def layout(self):
        cmake_layout(self)


    def generate(self):
        tc = CMakeToolchain(self)
        tc.generator = "Ninja"
        tc.generate()
