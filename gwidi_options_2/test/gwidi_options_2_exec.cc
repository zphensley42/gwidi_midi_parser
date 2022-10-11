#include "GwidiOptions2.h"
#include <spdlog/spdlog.h>

int main() {
    spdlog::set_level(spdlog::level::debug);
    auto &options = GwidiOptions2::getInstance();
    spdlog::debug("====Options====\n{}", (std::string)options);
}
