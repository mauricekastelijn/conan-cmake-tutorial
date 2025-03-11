#include "vectormath2d.h"

#include <spdlog/spdlog.h>    // using spdlog from Conan
#include <spdlog/fmt/fmt.h>  // for formatting (if needed)

std::string dot2d(int x1, int y1, int x2, int y2) {
    int result = x1 * x2 + y1 * y2;
    spdlog::info("Computed 2D dot product: {}", result);
    return fmt::format("({},{})·({},{}) = {}", x1, y1, x2, y2, result);
}
