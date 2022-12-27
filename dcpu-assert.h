#pragma once

#include <cassert>
#include <cstdio>

#define dcpu_assert(isValid, msg) \
    if (!(isValid)) { \
        printf(msg"\n"); \
        assert(isValid); \
    }

#define dcpu_assert_fmt(isValid, fmt, ...) \
    if (!(isValid)) { \
        printf("[DCPU ASSERT] " fmt "\n", __VA_ARGS__); \
        assert(isValid); \
    }
