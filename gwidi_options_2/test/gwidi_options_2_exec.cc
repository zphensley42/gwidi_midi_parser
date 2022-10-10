#include "gwidi_options_2_parser.h"
#include <spdlog/spdlog.h>

int main() {
    spdlog::set_level(spdlog::level::debug);
    auto &options = GwidiOptions2::getInstance();
}