// Spaceship — level 1
//
// `ccc run`    builds this file, runs every input, diffs the example.
// `ccc submit` does the same, then uploads the outputs.
#include "ccc.hpp"

constexpr int LEVEL = 1;

// >>> example input
//   3
//   5 5
//   5 4 4 4 5
//   5 4 3 2 3 4 5
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
        i64 answer = std::accumulate(c.begin(), c.end(), static_cast<long long>(0));
        out << answer << '\n';
    }
}

int main(int argc, char** argv) {
    return ccc::run_level<Input>(LEVEL, argc, argv, parse, solve);
}
