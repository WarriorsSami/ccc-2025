// Spaceship — level 5
//
// `ccc run`    builds this file, runs every input, diffs the example.
// `ccc submit` does the same, then uploads the outputs.
#include "ccc.hpp"

constexpr int LEVEL = 5;

// >>> example input
//   3
//   6,0 118
//   3,0
//   10,4 112
//   7,4
//   -7,-7 124
//   -4,-4
// <<< example input

// One case is two lines (PDF, "Input"):
//   "6,0 118"  space station X,Y, then the time limit
//   "3,0"      the asteroid's X,Y
struct TestCase {
    i64 station_x, station_y;
    i64 time_limit;
    i64 asteroid_x, asteroid_y;
};

struct Input {
    std::vector<TestCase> cases;
};

// Reader: next<T>(), line_values<T>(), r >> x, fields(), record<S, Ts...>(),
// set_separators(" ,") for comma-separated input; see Reader in ccc.hpp.
Input parse(Reader& r) {
    Input in;
    r.set_separators(" \t,");  // "6,0 118" now reads as 6 0 118
    const auto n = r.next<i64>();
    for (i64 i = 0; i < n; ++i)
        in.cases.push_back(r.as<TestCase, i64, i64, i64, i64, i64>());
    return in;
}

// The same parse, line by line, if you'd rather mirror the two-line layout:
//
//   for (i64 i = 0; i < n; ++i) {
//       TestCase c;
//       r.fields() >> c.station_x >> c.station_y >> c.time_limit;  // "6,0 118"
//       r.fields() >> c.asteroid_x >> c.asteroid_y;                 // "3,0"
//       in.cases.push_back(c);
//   }
//
// fields() splits on spaces, commas and semicolons regardless of
// set_separators, so this variant works even without that call.

struct Point {
    i64 x, y;
    bool operator==(const Point&) const = default;
    auto operator<=>(const Point&) const = default;
};

static std::vector<i64> generate_pace_one_dim(long long x) {
    std::vector<i64> pace;
    std::stack<int> ox;
    int sgn = x > 0 ? 1 : -1;
    x += 2*sgn;
    int num = 0;
    for (int idx = 0; idx < abs(x/2); ++idx) {
        ox.push(sgn *num);
        pace.emplace_back(ox.top());
        if (num == 0)
            num = 5;
        else if (num == 1)
            num = 1;
        else
            num--;
    }

    if (x % 2 !=0 ) {
        pace.emplace_back(sgn * num);
    }

    while (!ox.empty()) {
        pace.emplace_back(ox.top());
        ox.pop();
    }

    return pace;
}

static std::pair<std::vector<i64>, std::vector<i64>> generate_pace_two_dim(Point p) {
    return std::make_pair(generate_pace_one_dim(p.x), generate_pace_one_dim(p.y));
}

// > 0: o -> a -> b turns counter-clockwise, < 0: clockwise, 0: collinear.
static i64 cross(const Point& o, const Point& a, const Point& b) {
    return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

static double length(const Point& a, const Point& b) {
    return std::hypot(static_cast<double>(a.x - b.x), static_cast<double>(a.y - b.y));
}

// Convex hull of `points`, in clockwise order (monotone chain).
// Points lying on a hull edge are kept, so a start/end that sits exactly on the
// border of the asteroid's safety square still counts as "on the hull".
static std::vector<Point> convex_hull(std::vector<Point> points) {
    std::ranges::sort(points);
    points.erase(std::unique(points.begin(), points.end()), points.end());
    if (points.size() < 3) return points;

    // All collinear: the "hull" is the segment itself, end to end.
    const bool flat = std::ranges::all_of(points, [&](const Point& p) {
        return cross(points.front(), points.back(), p) == 0;
    });
    if (flat) return points;

    // Lower then upper chain gives counter-clockwise order; popping only on a
    // clockwise turn (cross < 0) keeps the collinear edge points.
    std::vector<Point> hull;
    for (int pass = 0; pass < 2; ++pass) {
        const auto start = hull.size();
        for (const auto& p : points) {
            while (hull.size() >= start + 2 && cross(hull[hull.size() - 2], hull.back(), p) < 0)
                hull.pop_back();
            hull.push_back(p);
        }
        hull.pop_back();  // each chain's last point starts the other chain
        std::ranges::reverse(points);
    }
    std::ranges::reverse(hull);  // counter-clockwise -> clockwise
    return hull;
}

// Shortest way from `start` to `end` around the obstacle `points`: both must
// be on the convex hull of everything; the route is then the shorter of the
// two hull walks between them. Otherwise just {start, end}.
static std::vector<Point> ShortestRoute(const std::vector<Point>& points, const Point& start, const Point& end) {
    std::vector<Point> all = points;
    all.push_back(start);
    all.push_back(end);
    const auto hull = convex_hull(all);

    const auto i = std::ranges::find(hull, start);
    const auto j = std::ranges::find(hull, end);
    if (i == hull.end() || j == hull.end() || start == end) return {start, end};

    // Walk the hull from start to end in one direction (+1 clockwise, -1 counter-clockwise).
    const auto n = static_cast<std::ptrdiff_t>(hull.size());
    const auto walk = [&](std::ptrdiff_t step) {
        std::vector<Point> route;
        for (auto k = i - hull.begin();; k = (k + step + n) % n) {
            route.push_back(hull[static_cast<std::size_t>(k)]);
            if (hull[static_cast<std::size_t>(k)] == end) break;
        }
        return route;
    };
    const auto route_length = [](const std::vector<Point>& route) {
        double total = 0;
        for (std::size_t k = 1; k < route.size(); ++k) total += length(route[k - 1], route[k]);
        return total;
    };
    // Fewest points wins, wrapping around the hull as needed: for the clockwise
    // hull {c1, start, c2, c3, end} that's start -> c1 -> end, not start -> c2 -> c3 -> end.
    // Ties go to the shorter walk, then clockwise (what the first example does).
    auto clockwise = walk(+1), counter_clockwise = walk(-1);
    if (counter_clockwise.size() != clockwise.size())
        return counter_clockwise.size() < clockwise.size() ? counter_clockwise : clockwise;
    return route_length(counter_clockwise) < route_length(clockwise) ? counter_clockwise : clockwise;
}

// Time a pace sequence takes: pace p costs |p| steps, a 0 (standing still) costs 1.
static i64 duration(const std::vector<i64>& pace) {
    i64 total = 0;
    for (const auto p : pace) total += p == 0 ? 1 : std::abs(p);
    return total;
}

static void print_pace(const std::vector<i64>& pace, std::ostream& out) {
    for (const auto& p : pace) {
        out << p << " ";
    }
    out << std::endl;
}

static void solve(const Input& in, std::ostream& out) {
    for (const auto& block : in.cases) {
        const Point ship{.x = 0, .y = 0};
        const Point station{.x = block.station_x, .y = block.station_y};
        // Corners 3 away from the asteroid on each axis: just outside the
        // ship's safety square (radius 2).
        const std::vector<Point> corners = {
            {.x = block.asteroid_x - 3, .y = block.asteroid_y - 3},
            {.x = block.asteroid_x - 3, .y = block.asteroid_y + 3},
            {.x = block.asteroid_x + 3, .y = block.asteroid_y + 3},
            {.x = block.asteroid_x + 3, .y = block.asteroid_y - 3},
        };

        const auto route = ShortestRoute(corners, ship, station);

        std::vector<i64> pace_x, pace_y;
        for (std::size_t k = 1; k < route.size(); ++k) {
            // Pace for this leg, from the displacement Point2 - Point1 (negative
            // components move left / down).
            const Point delta{.x = route[k].x - route[k - 1].x, .y = route[k].y - route[k - 1].y};
            auto [leg_x, leg_y] = generate_pace_two_dim(delta);

            // Let the faster axis wait at the waypoint, so both axes reach it
            // together and the next leg starts from the corner, not mid-air.
            // (Not needed after the last leg: each axis just stops at the station.)
            if (k + 1 < route.size()) {
                const auto dx = duration(leg_x), dy = duration(leg_y);
                leg_x.insert(leg_x.end(), static_cast<std::size_t>(std::max<i64>(0, dy - dx)), 0);
                leg_y.insert(leg_y.end(), static_cast<std::size_t>(std::max<i64>(0, dx - dy)), 0);
            }

            std::ranges::copy(leg_x, std::back_inserter(pace_x));
            std::ranges::copy(leg_y, std::back_inserter(pace_y));
        }

        if (std::max(duration(pace_x), duration(pace_y)) > block.time_limit)
            std::cerr << "warning: route takes " << std::max(duration(pace_x), duration(pace_y))
                      << " > time limit " << block.time_limit << '\n';

        print_pace(pace_x, out);
        print_pace(pace_y, out);
        out << std::endl;
    }
}

int main(int argc, char** argv) {
    return ccc::run_level<Input>(LEVEL, argc, argv, parse, solve);
}
