from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, cmake_layout


class FloodMonitorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        #self.requires("libcurl/8.17.0")
        self.requires("nlohmann_json/3.12.0")
        #self.requires("qt/6.8.3")
        self.test_requires("gtest/1.17.0")
       
    def configure(self):
        '''self.options["qt"].gui = True
        self.options["qt"].shared = True
        self.options["qt"].qtdeclarative = True
        self.options["qt"].qtpositioning = True
        self.options["qt"].qtlocation = True
        self.options["qt"].widgets = False
        self.options["qt"].with_odbc = False
        self.options["qt"].with_sqlite3 = False
        self.options["qt"].with_pq=False'''

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generator = "Ninja"
        tc.generate()
