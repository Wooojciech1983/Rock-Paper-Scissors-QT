from conan import ConanFile
from conan.tools.cmake import CMake

class HomeworkRPS(ConanFile):
    generators = "CMakeDeps", "CMakeToolchain"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("gtest/1.15.0")

    def build_requirements(self):
        self.tool_requires("cmake/3.31.6")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
