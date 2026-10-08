// Spaceship — level 5
//
// `ccc run`    builds this file, runs every input, diffs the example.
// `ccc submit` does the same, then uploads the outputs.
#include "ccc.hpp"

constexpr int LEVEL = 6;

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

// Time a pace sequence takes: pace p costs |p| steps, a 0 (standing still) costs 1.
static i64 duration(const std::vector<i64>& pace) {
    i64 total = 0;
    for (const auto p : pace) total += p == 0 ? 1 : std::abs(p);
    return total;
}

// Both axes' paces for a route: one leg per pair of consecutive points, joined.
static std::pair<std::vector<i64>, std::vector<i64>> route_pace(const std::vector<Point>& route) {
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
    return {pace_x, pace_y};
}

// Ticks a route takes (the slower axis).
static i64 route_time(const std::vector<Point>& route) {
    const auto [pace_x, pace_y] = route_pace(route);
    return std::max(duration(pace_x), duration(pace_y));
}

// Like route_pace, but an axis that keeps moving the same way through a
// waypoint doesn't stop there: its consecutive legs in the same direction
// become one continuous move. Each run still starts at the tick its first leg
// starts in route_pace (so the axes stay in step wherever an axis turns or
// stands still); it only gets there sooner.
static std::pair<std::vector<i64>, std::vector<i64>> route_pace_no_stops(const std::vector<Point>& route) {
    if (route.size() < 2) return route_pace(route);
    const std::size_t legs = route.size() - 1;

    // Per-leg displacement, and the tick each leg starts at in route_pace
    // (every leg but the last lasts as long as its slower axis).
    std::vector<i64> dx(legs), dy(legs), leg_start(legs, 0);
    for (std::size_t k = 0; k < legs; ++k) {
        dx[k] = route[k + 1].x - route[k].x;
        dy[k] = route[k + 1].y - route[k].y;
        if (k + 1 < legs)
            leg_start[k + 1] = leg_start[k] + std::max(duration(generate_pace_one_dim(dx[k])),
                                                       duration(generate_pace_one_dim(dy[k])));
    }

    const auto axis = [&](const std::vector<i64>& d) {
        std::vector<i64> pace;
        i64 now = 0;  // ticks covered by `pace` so far
        for (std::size_t i = 0; i < legs;) {
            if (d[i] == 0) {  // this axis stands still during the leg
                ++i;
                continue;
            }
            // Run of consecutive legs moving this axis the same way.
            std::size_t j = i;
            i64 total = 0;
            while (j < legs && d[j] != 0 && (d[j] > 0) == (d[i] > 0)) total += d[j++];

            const auto wait = std::max<i64>(0, leg_start[i] - now);
            pace.insert(pace.end(), static_cast<std::size_t>(wait), 0);
            const auto run = generate_pace_one_dim(total);
            pace.insert(pace.end(), run.begin(), run.end());
            now += wait + duration(run);
            i = j;
        }
        if (pace.empty()) pace = {0, 0};  // this axis never moves
        return pace;
    };
    return {axis(dx), axis(dy)};
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
    auto clockwise = walk(+1), counter_clockwise = walk(-1);
    // Fewest points wins, wrapping around the hull as needed: for the clockwise
    // hull {c1, start, c2, c3, end} that's start -> c1 -> end, not start -> c2 -> c3 -> end.
    // Ties go to the shorter walk, then clockwise (what the first example does).
    if (counter_clockwise.size() != clockwise.size())
        return counter_clockwise.size() < clockwise.size() ? counter_clockwise : clockwise;
    // 2 points between start and end in both directions: build the pace for
    // both and keep the faster one.
    if (clockwise.size() == 4) {
        const auto t_cw = route_time(clockwise), t_ccw = route_time(counter_clockwise);
        if (t_cw != t_ccw) return t_ccw < t_cw ? counter_clockwise : clockwise;
    }
    return route_length(counter_clockwise) < route_length(clockwise) ? counter_clockwise : clockwise;
}

// Where line start-C1 (extended past C1) meets line end-C2 (extended past C2),
// i.e. the point P12 that replaces C1 and C2 in start, C1, C2, end. Rounded to
// whole cells away from `asteroid`, so the shortcut doesn't cut closer to it.
// nullopt if the lines are parallel or meet on the wrong side.
static std::optional<Point> intersection(const Point& start, const Point& c1, const Point& c2, const Point& end,
                                         const Point& asteroid) {
    const auto dx1 = static_cast<double>(c1.x - start.x), dy1 = static_cast<double>(c1.y - start.y);
    const auto dx2 = static_cast<double>(c2.x - end.x), dy2 = static_cast<double>(c2.y - end.y);
    const auto ex = static_cast<double>(end.x - start.x), ey = static_cast<double>(end.y - start.y);
    const double den = dx1 * dy2 - dy1 * dx2;
    if (den == 0) return std::nullopt;
    const double t = (ex * dy2 - ey * dx2) / den;  // along start -> C1, 1 = at C1
    const double u = (ex * dy1 - ey * dx1) / den;  // along end -> C2, 1 = at C2
    if (t < 1 || u < 1) return std::nullopt;
    const auto outward = [](double v, i64 from) {
        return static_cast<i64>(v >= static_cast<double>(from) ? std::ceil(v) : std::floor(v));
    };
    return Point{.x = outward(static_cast<double>(start.x) + t * dx1, asteroid.x),
                 .y = outward(static_cast<double>(start.y) + t * dy1, asteroid.y)};
}

// Position after every tick for one axis: pace p holds |p| ticks, then moves
// one cell in p's direction; a 0 waits one tick.
static std::vector<i64> track(const std::vector<i64>& pace) {
    std::vector<i64> pos{0};
    i64 at = 0;
    for (const auto p : pace) {
        for (i64 k = 1; k < std::abs(p); ++k) pos.push_back(at);
        if (p != 0) at += p > 0 ? 1 : -1;
        pos.push_back(at);
    }
    return pos;
}

// The paces reach the station without the asteroid ever entering the ship's
// safety square (radius 2), tick by tick.
static bool paces_safe(const std::vector<i64>& pace_x, const std::vector<i64>& pace_y, const TestCase& c) {
    auto xs = track(pace_x), ys = track(pace_y);
    const auto n = std::max(xs.size(), ys.size());
    const auto last_x = xs.back(), last_y = ys.back();
    xs.resize(n, last_x);
    ys.resize(n, last_y);
    for (std::size_t t = 0; t < n; ++t)
        if (std::abs(xs[t] - c.asteroid_x) <= 2 && std::abs(ys[t] - c.asteroid_y) <= 2) return false;
    return last_x == c.station_x && last_y == c.station_y;
}

// Same check for a route flown with route_pace.
static bool is_safe(const std::vector<Point>& route, const TestCase& c) {
    const auto [pace_x, pace_y] = route_pace(route);
    return paces_safe(pace_x, pace_y, c);
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

        auto route = ShortestRoute(corners, ship, station);

        // out << "route: " << std::endl;
        // for (const auto& p : route) {
        //     out << p.x << " " << p.y << std::endl;
        // }

        // start, C1, C2, end: also try start, P12, end (P12 = where lines
        // start-C1 and C2-end meet) and keep it if its pace is faster.
        if (route.size() == 4) {
            const Point asteroid{.x = block.asteroid_x, .y = block.asteroid_y};
            if (const auto p12 = intersection(route[0], route[1], route[2], route[3], asteroid)) {
                const std::vector<Point> shortcut = {route[0], *p12, route[3]};
                if (route_time(shortcut) < route_time(route) && is_safe(shortcut, block)) route = shortcut;
            }
        }

        auto [pace_x, pace_y] = route_pace(route);

        // Same route without stopping at waypoints an axis passes straight
        // through; used only if it stays clear of the asteroid and isn't slower.
        if (auto [no_stop_x, no_stop_y] = route_pace_no_stops(route);
            paces_safe(no_stop_x, no_stop_y, block) &&
            std::max(duration(no_stop_x), duration(no_stop_y)) <= std::max(duration(pace_x), duration(pace_y))) {
            pace_x = std::move(no_stop_x);
            pace_y = std::move(no_stop_y);
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
