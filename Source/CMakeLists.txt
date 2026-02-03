include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
        OSInterface
        GIT_REPOSITORY  git@github.com:PurifyMyWater/OSInterface.git
        GIT_TAG         v0.3.1
)
FetchContent_MakeAvailable(OSInterface)

add_library(LinuxOSInterface STATIC)

file(GLOB LinuxOSInterface_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(LinuxOSInterface PRIVATE ${LinuxOSInterface_SOURCES})
target_include_directories(LinuxOSInterface PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(LinuxOSInterface PUBLIC OSInterface)
