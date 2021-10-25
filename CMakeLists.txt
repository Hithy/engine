cmake_minimum_required(VERSION 3.10)
project(engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)
set(IMGUI_FILES
  "3rd/imgui/imgui.cpp"
  "3rd/imgui/imgui_draw.cpp"
  "3rd/imgui/imgui_tables.cpp"
  "3rd/imgui/imgui_widgets.cpp"
  "3rd/imgui/imgui_demo.cpp"
  "3rd/imgui/backends/imgui_impl_opengl3.cpp"
  "3rd/imgui/backends/imgui_impl_glfw.cpp"
  )
add_executable(engine
  ${SRC_FILES}
  ${IMGUI_FILES}
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
target_include_directories(engine PUBLIC "3rd/imgui")
target_include_directories(engine PUBLIC "3rd/physx/include")
target_include_directories(engine PUBLIC "3rd/physx/pxshared/include")
target_link_directories(engine PUBLIC "3rd/lib/compile")

if (UNIX)
  target_link_libraries(engine physx dl pthread util)
elseif(WIN32)
  target_link_libraries(engine physx_64 PhysXFoundation_64 PhysXCooking_64 PhysXPvdSDK_static_64 PhysXExtensions_static_64)
endif()

target_link_libraries(engine glfw assimp python3)

add_custom_target(run
  COMMAND engine
  DEPENDS engine
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )
