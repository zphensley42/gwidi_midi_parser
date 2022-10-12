#include "GwidiOptions.h"
#include "spdlog/spdlog.h"

void printAttribs(const std::string& name, gwidi::options::InstrumentOptions::InstrumentNoteAttributes &attribs) {
    spdlog::debug("{} -> instrumentNoteNumber: {}, instrumentOctave: {}, key: {}, --letters--", name, attribs.instrumentNoteNumber, attribs.instrumentOctave, attribs.key);
    for(auto& l : attribs.letters) {
        spdlog::debug("\t{}", l);
    }
}

int main() {
    spdlog::set_level(spdlog::level::debug);

    auto &instrOpt = gwidi::options::InstrumentOptions::getInstance();

    auto attribs1 = instrOpt.instrumentNoteAttributesForMidi("harp", 3, "C#");
    auto attribs2 = instrOpt.instrumentNoteAttributesForMidi("harp", 4, "D");
    auto attribs3 = instrOpt.instrumentNoteAttributesForMidi("harp", 5, "C");
    auto attribs4 = instrOpt.instrumentNoteAttributesForMidi("harp", 6, "C");
    auto attribs5 = instrOpt.instrumentNoteAttributesForMidi("harp", 7, "C");

    printAttribs("attribs1", attribs1);
    printAttribs("attribs2", attribs2);
    printAttribs("attribs3", attribs3);
    printAttribs("attribs4", attribs4);
    printAttribs("attribs5", attribs5);

    return 0;
}