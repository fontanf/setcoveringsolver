add_executable(SetCoveringSolver_local_search_row_weighting_test)
target_sources(SetCoveringSolver_local_search_row_weighting_test PRIVATE
    local_search_row_weighting_test.cpp)
target_link_libraries(SetCoveringSolver_local_search_row_weighting_test
    SetCoveringSolver_local_search_row_weighting
    Boost::filesystem
    GTest::gtest_main)
gtest_discover_tests(SetCoveringSolver_local_search_row_weighting_test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
