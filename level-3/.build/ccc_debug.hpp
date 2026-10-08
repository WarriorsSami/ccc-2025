// Force-included by `ccc run --debug` (regenerated on every debug build).
// Prints a demangled stack trace when the solution dies from a signal, e.g. a
// failed libc++ bounds check (SIGABRT) or a segfault.
#pragma once

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cxxabi.h>
#include <execinfo.h>
#include <initializer_list>

namespace ccc_debug {

// Pull the mangled symbol out of one backtrace_symbols() line.
//   macOS: "3   solution-debug   0x0000000104cf5030 _Z5solveRK5Input + 120"
//   Linux: "./solution-debug(_Z5solveRK5Input+0x78) [0x55d0c3a1b2c4]"
inline const char* mangled_name(char* line) {
    if (char* open = std::strchr(line, '(')) {
        char* plus = std::strchr(open, '+');
        if (plus && plus > open + 1) {
            *plus = '\0';
            return open + 1;
        }
        return line;
    }
    char* tok = std::strtok(line, " ");
    for (int i = 0; tok && i < 3; ++i) tok = std::strtok(nullptr, " ");
    return tok ? tok : line;
}

inline void on_crash(int sig) {
    std::signal(sig, SIG_DFL);
    void* frames[48];
    const int n = backtrace(frames, 48);
    char** syms = backtrace_symbols(frames, n);
    std::fprintf(stderr, "\n--- stack trace (%s) ---\n", strsignal(sig));
    for (int i = 1; syms && i < n; ++i) {  // frame 0 is this handler
        char buf[1024];
        std::strncpy(buf, syms[i], sizeof buf - 1);
        buf[sizeof buf - 1] = '\0';
        const char* name = mangled_name(buf);
        int status = 0;
        char* pretty = abi::__cxa_demangle(name, nullptr, nullptr, &status);
        std::fprintf(stderr, "  #%-2d %s\n", i - 1, status == 0 && pretty ? pretty : name);
        std::free(pretty);
    }
    std::fflush(stderr);
    std::raise(sig);
}

inline const bool installed = [] {
    for (int s : {SIGABRT, SIGSEGV, SIGFPE, SIGBUS, SIGILL}) std::signal(s, on_crash);
    return true;
}();

}  // namespace ccc_debug
