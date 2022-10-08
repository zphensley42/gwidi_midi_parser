#ifndef GWIDI_MIDI_PARSER_UTIL_H
#define GWIDI_MIDI_PARSER_UTIL_H

std::vector<std::string_view> splitSV(std::string_view strv, std::string_view delims = " ") {
    std::vector<std::string_view> output;
    size_t first = 0;

    while (first < strv.size()) {
        const auto second = strv.find_first_of(delims, first);
        if (first != second) {
            output.emplace_back(strv.substr(first, second - first));
        }

        if (second == std::string_view::npos) {
            break;
        }

        first = second + 1;
    }

    return output;
}

#endif //GWIDI_MIDI_PARSER_UTIL_H
