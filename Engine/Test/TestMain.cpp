#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    int result = Catch::Session().run(argc, argv);
    std::getchar();
    return result;
}
