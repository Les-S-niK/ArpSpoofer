from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class mypkgRecipe(ConanFile):
    name = "ArpSpoofer"
    version = "0.1"
    package_type = "application"

    # Optional metadata
    license = "MIT"
    author = "Les-S-niK lessnik343@gmail.com"
    url = "https://github.com/Les-S-niK/ArpSpoofer"
    description = "Simple ArpSpoofer utilite."
    topics = ("Network", "C++", "CXX")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def requirements(self) -> None:
        self.requires("gtest/1.18.0")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
