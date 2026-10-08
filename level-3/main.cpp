// Spaceship — level 3
//
// `ccc run`    builds this file, runs every input, diffs the example.
// `ccc submit` does the same, then uploads the outputs.
#include "ccc.hpp"

constexpr int LEVEL = 3;

// >>> example input
//   3
//   2 25
//   5 39
//   -7 46
// <<< example input

struct Input {
    // Guessed from the example: N, then N lines (one case per line).
    std::vector<std::vector<i64>> cases;
};

Input parse(Reader& r) {
    Input in;
    const auto n = r.next<i64>();
    for (i64 i = 0; i < n; ++i) in.cases.push_back(r.line_values<i64>());
    return in;
}

void solve(const Input& in, std::ostream& out) {
    for (const auto& c : in.cases) {
        const auto station_pos = c[0], time_limit [[maybe_unused]] = c[1];
        std::vector<i64> pace_freq(6);
        const auto sign = station_pos > 0 ? 1 : -1;

        pace_freq[0] = 2;
        i64 curr_pace = 5, curr_pos = abs(station_pos);
        while (curr_pos > 1 && curr_pace > 1) {
            pace_freq[curr_pace] = 2;
            curr_pos -= 2;
            --curr_pace;
        }
        pace_freq[curr_pace] = curr_pos;

        // for (auto freq: pace_freq) {
        //     out << freq << ' ';
        // }
        // out << std::endl;

        curr_pace = 0;
        out << curr_pace << ' ';

        for (curr_pace = 5; curr_pace > 1 && pace_freq[curr_pace] != 0; --curr_pace) {
            out << (sign > 0 ? curr_pace : -curr_pace) << ' ';
        }

        if (curr_pace == 1 && pace_freq[curr_pace] != 0) {
            for (auto i = 0; i < pace_freq[curr_pace]; ++i) {
                out << (sign > 0 ? curr_pace : -curr_pace) << ' ';
            }
            ++curr_pace;
        }

        for (; curr_pace <= 5; ++curr_pace) {
            if (pace_freq[curr_pace] == 2) {
                out << (sign > 0 ? curr_pace : -curr_pace) << ' ';
            }
        }

        curr_pace = 0;
        out << curr_pace << ' ';

        out << '\n';
    }
}

int main(int argc, char** argv) {
    return ccc::run_level<Input>(LEVEL, argc, argv, parse, solve);
}
