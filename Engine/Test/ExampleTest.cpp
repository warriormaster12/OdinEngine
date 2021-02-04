#include "catch.hpp"

TEST_CASE("Example test", "[Tag]")
{
    int a = 1;
    int b = 2;
    REQUIRE(a + b == 3);
}
