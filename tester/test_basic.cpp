#define CATCH_CONFIG_MAIN
#include "../tester/catch.hpp"

TEST_CASE("Basic check") {
    REQUIRE(1 + 1 == 2);
}
