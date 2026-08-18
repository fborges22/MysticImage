cmake_minimum_required(VERSION 3.16)

# Set the project name and version
project(MysticImage VERSION 1.1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenCV package
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs)

# Add the STATIC library
add_library(mystic STATIC src/Standard.cpp src/Smoke.cpp)
target_include_directories(mystic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${OpenCV_INCLUDE_DIRS})
target_link_libraries(mystic PUBLIC ${OpenCV_LIBS})

# Add the executable
add_executable(mystique src/mystique.cpp)

# Link OpenCV libraries
target_link_libraries(mystique PRIVATE mystic)

# Installation configuration
install(TARGETS mystique mystic
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES src/Standard.hpp src/Smoke.hpp
    DESTINATION include/MysticImage
)

install(FILES README.md
    DESTINATION share/${PROJECT_NAME}
)

# CPack configuration
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "MysticImage")
set(CPACK_PACKAGE_VENDOR "BorgesHQ")
#set(CPACK_PACKAGE_CONTACT "youremail@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A tool and library to convert between binary files and images.")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# CPack generator configurations
set(CPACK_GENERATOR "ZIP;TGZ")

# Set installation directory (for Unix systems)
if(UNIX)
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
endif()

include(CPack)
