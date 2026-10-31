configure_file(testdir.h.in ${CMAKE_CURRENT_BINARY_DIR}/testdir.h @ONLY)

# Set up the minimal fcitx data directory layout expected by the test.
add_custom_target(copy-test-data
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/addon
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/src/skk-addon.conf ${CMAKE_CURRENT_BINARY_DIR}/addon/skk.conf
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/inputmethod
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/src/skk.conf ${CMAKE_CURRENT_BINARY_DIR}/inputmethod/skk.conf
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/skk
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/src/dictionary_list ${CMAKE_CURRENT_BINARY_DIR}/skk/dictionary_list
    DEPENDS skk-addon.conf.in-fmt skk.conf.in-fmt)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(testskk testskk.cpp)
target_link_libraries(testskk Fcitx5::Core Fcitx5::Module::TestFrontend)
add_dependencies(testskk skk copy-test-data)
add_test(NAME testskk COMMAND testskk)
