// Spaceship — level 4
//
// `ccc run`    builds this file, runs every input, diffs the example.
// `ccc submit` does the same, then uploads the outputs.
#include "ccc.hpp"

constexpr int LEVEL = 4;

// >>> example input
//   3
//   3,-3 16
//   7,5 28
//   -4,-9 31
// <<< example input

struct Input {
    // Guessed from the example: N, then N lines (one case per line).
    std::vector<std::vector<std::string>> cases;
};

Input parse(Reader& r) {
    Input in;
    const auto n = r.next<i64>();
    for (i64 i = 0; i < n; ++i) in.cases.push_back(r.line_values<std::string>());
    return in;
}

void print_pace(long long x, std::ostream& out) {
    std::stack<int> ox;
    int sgn = x > 0 ? 1 : -1;
    x += 2*sgn;
    int num = 0;
    for (int idx = 0; idx < abs(x/2); ++idx) {
        ox.push(sgn *num);
        out << ox.top() << " ";
        if (num == 0)
            num = 5;
        else if (num == 1)
            num = 1;
        else
            num--;
    }

    if (x % 2 !=0 ) {
        out << sgn * num << " ";
    }

    while (!ox.empty()) {
        out << ox.top() << " ";
        ox.pop();
    }
    out << std::endl;
}

void solve(const Input& in, std::ostream& out) {
    for (const auto& c : in.cases) {
        std::stringstream pos(c[0]);
        long long x,y;
        char dum;
        pos >> x >> dum >> y;

        print_pace(x, out);
        print_pace(y, out);
        out << std::endl;
    }
}

int main(int argc, char** argv) {
    return ccc::run_level<Input>(LEVEL, argc, argv, parse, solve);
}
