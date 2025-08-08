function(setup_vcpkg)
  if(DEFINED ENV{VCPKG_ROOT})
    set(VCPKG_ROOT "$ENV{VCPKG_ROOT}" CACHE PATH "VCPKG root directory")
  endif()
  
  if(NOT DEFINED VCPKG_ROOT)
    set(VCPKG_ROOT "C:/vcpkg" CACHE PATH "Default VCPKG root")
  endif()

  set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
  set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
  
  message(STATUS "Using vcpkg at: ${VCPKG_ROOT}")
  message(STATUS "VCPKG triplet: ${VCPKG_TARGET_TRIPLET}")
endfunction()
