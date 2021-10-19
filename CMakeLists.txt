cmake_minimum_required(VERSION 3.10)
project(engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)
add_executable(engine
  ${SRC_FILES}
  "3rd/glad/glad.c")

function(fetch_git_repo path name)
  if (NOT EXISTS "${CMAKE_SOURCE_DIR}/3rd/${name}")
    execute_process(COMMAND git clone ${path} WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/3rd")
  endif()
  add_subdirectory("3rd/${name}")
endfunction()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # msvc compile from source
  fetch_git_repo("https://github.com/g-truc/glm.git" "glm")
  fetch_git_repo("https://github.com/glfw/glfw.git" "glfw")
  fetch_git_repo("https://github.com/assimp/assimp.git" "assimp")

  target_include_directories(engine PUBLIC "3rd/glm")
  target_include_directories(engine PUBLIC "3rd/assimp/include")
else()
  find_package(glfw3 REQUIRED)
  find_package(glm REQUIRED)
  find_package(assimp REQUIRED)
endif()

target_include_directories(engine PUBLIC
  "3rd/python/Include"
  )
if (UNIX)
  target_include_directories(engine PUBLIC
      "3rd/python/Linux"
    )
  add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/3rd/lib/runtime/linux"
        $<TARGET_FILE_DIR:engine>)
elseif(WIN32)
  target_include_directories(engine PUBLIC
      "3rd/python/PC"
    )
  add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/3rd/lib/runtime/windows"
        $<TARGET_FILE_DIR:engine>)
endif()

target_include_directories(engine PUBLIC "src")
target_include_directories(engine PUBLIC "3rd/include")
target_link_directories(engine PUBLIC "3rd/lib/compile")

if (UNIX)
  target_link_libraries(engine dl)
endif(UNIX)

target_link_libraries(engine glfw assimp python3)

add_custom_target(run
  COMMAND engine
  DEPENDS engine
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )
