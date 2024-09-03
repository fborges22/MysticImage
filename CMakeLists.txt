cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(MysticImage VERSION 1.0)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Add the executable
add_executable(MysticImage main.cpp)

# Link OpenCV libraries
target_link_libraries(MysticImage ${OpenCV_LIBS})

# Installation configuration
install(TARGETS MysticImage
    RUNTIME DESTINATION bin
)

install(FILES README.md
    DESTINATION share/${PROJECT_NAME}
)

# CPack configuration
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "MysticImage")
set(CPACK_PACKAGE_VENDOR "BorgesHQ")
set(CPACK_PACKAGE_CONTACT "youremail@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A tool to convert between binary files and images.")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# CPack generator configurations
set(CPACK_GENERATOR "ZIP;TGZ;RPM;DEB")

# Set installation directory (for Unix systems)
if(UNIX)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
endif()

include(CPack)
