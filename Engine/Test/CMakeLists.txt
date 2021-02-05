project(UnitTest)

# Create the executable
add_executable(${PROJECT_NAME} TestMain.cpp ExampleTest.cpp)


target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/../Source/Third-Party/catch2/
)
