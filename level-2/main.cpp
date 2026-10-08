// Spaceship — level 2
//
// `ccc run`    builds this file, runs every input, diffs the example.
// `ccc submit` does the same, then uploads the outputs.
#include "ccc.hpp"

constexpr int LEVEL = 2;

// >>> example input
//   3
//   0 5 5 0
//   0 5 5 0 -5 0
//   0 -5 -4 -3 -2 -3 -4 -5 0
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
    for (auto& test: in.cases) {
        long long time = 0;
        long long position = 0;
        for (auto& step: test) {
            time += step? abs(step) : 1;
            position += step? (step > 0 ? 1 : -1) : 0;
        }
        out << position << " " << time << '\n';
    }
}

int main(int argc, char** argv) {
    return ccc::run_level<Input>(LEVEL, argc, argv, parse, solve);
}
