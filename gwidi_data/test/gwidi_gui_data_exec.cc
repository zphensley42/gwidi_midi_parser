#include "GwidiGuiData.h"
#include "GwidiOptions2.h"
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cerrno>

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A); }

void test_time(int time, gwidi::data::gui::Octave &octave, gwidi::options2::Instrument &options) {
    assert(octave.notes[time].size() == options.octaves.at(octave.num).notes.size());

    int note_index = 0;
    for(auto &note : octave.notes[time]) {
        assert(note.measure == octave.measure);
        assert(note.time == time);
        assert(note.octave == octave.num);
        assert(note.key == options.octaves.at(octave.num).notes.at(note_index).key);
        int letter_index = 0;
        for(auto &letter : note.letters) {
            assert(letter == options.octaves.at(octave.num).notes.at(note_index).letters.at(letter_index));
            letter_index++;
        }
        note_index++;
    }
}

void test_octave(int index, gwidi::data::gui::Measure &measure, gwidi::options2::Instrument &options) {
    int notesPerMeasure = 16;   // support 16th notes
    assert(measure.octaves.at(index).notes.size() == notesPerMeasure);
    assert(measure.octaves.at(index).num == index);
    assert(measure.octaves.at(index).measure == measure.num);

    for(auto time = 0; time < notesPerMeasure; time++) {
        test_time(time, measure.octaves.at(index), options);
    }
}

void test_measure(int index, std::vector<gwidi::data::gui::Measure> &measures, gwidi::options2::Instrument &options) {
    assert(measures.at(index).octaves.size() == options.octaves.size());
    assert(measures.at(index).num == index);

    for(auto octave = 0; octave < measures.at(index).octaves.size(); octave++) {
        test_octave(octave, measures.at(index), options);
    }
}

void test_instrument(const std::string &instr) {
    auto &options = gwidi::options2::GwidiOptions2::getInstance().getMapping()[instr];

    gwidi::data::gui::GwidiGuiData data(instr);
    auto &measures = data.getMeasures();
    assert(measures.size() == 1);

    data.addMeasure();
    assert(measures.size() == 2);

    test_measure(0, measures, options);
    test_measure(1, measures, options);
}

int main() {
    test_instrument("default");
    test_instrument("harp");
    test_instrument("bell");
    test_instrument("flute");

    return 0;
}
