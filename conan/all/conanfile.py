from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import get, copy, rmdir
import os

class VKbotConan(ConanFile):
    name = "vkbot"
    version = "0.1.0"
    url = "https://github.com/jetfire-oldsmobile27/vkbot"
    description = "C++17 ВКонтакте library"
    license = "Apache-2.0"
    settings = "os", "compiler", "build_type", "arch"
    topics = ("vk", "vkontakte-api", "vk-bot", "bot")
    options  = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("boost/1.83.0")
        self.requires("openssl/3.6.0")
        self.requires("nlohmann_json/3.11.3")

    def source(self):
        get(self,
            "https://github.com/jetfire-oldsmobile27/vkbot/archive/refs/heads/main.zip",
            strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_EXAMPLES"] = "OFF"
        tc.variables["CMAKE_POLICY_DEFAULT_CMP0077"] = "NEW"
        tc.generate()

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "bin"))

    def package_info(self):
        self.cpp_info.libs = ["vkbot"]
        self.cpp_info.includedirs = ["include/vkbot"]
        self.cpp_info.set_property("cmake_file_name",   "VKBOT")
        self.cpp_info.set_property("cmake_target_name", "vkbot::vkbot")
        self.cpp_info.requires = [
            "boost::system",
            "openssl::ssl",
            "openssl::crypto",
            "nlohmann_json::nlohmann_json",
        ]
