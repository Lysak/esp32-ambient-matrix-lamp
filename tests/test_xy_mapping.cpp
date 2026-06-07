#include <cassert>
#include <cstdio>
#include "ambient_matrix/matrix.h"

using ambient_matrix::Matrix;

static void test_bottom_left_is_index_zero() {
    Matrix m(16, 16);
    assert(m.xy(0, 0) == 0);
}

static void test_row0_left_to_right() {
    Matrix m(16, 16);
    for (int x = 0; x < 16; x++) {
        assert(m.xy(x, 0) == (uint16_t)x);
    }
}

static void test_row1_right_to_left_serpentine() {
    Matrix m(16, 16);
    assert(m.xy(15, 1) == 16);
    assert(m.xy(14, 1) == 17);
    assert(m.xy(0,  1) == 31);
}

static void test_row2_left_to_right_again() {
    Matrix m(16, 16);
    assert(m.xy(0,  2) == 32);
    assert(m.xy(15, 2) == 47);
}

static void test_all_indices_unique_4x4() {
    Matrix m(4, 4);
    bool seen[16] = {};
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            uint16_t idx = m.xy(x, y);
            assert(idx < 16);
            assert(!seen[idx]);
            seen[idx] = true;
        }
    }
}

static void test_all_indices_in_bounds_16x16() {
    Matrix m(16, 16);
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            assert(m.xy(x, y) < 256);
        }
    }
}

int main() {
    test_bottom_left_is_index_zero();
    test_row0_left_to_right();
    test_row1_right_to_left_serpentine();
    test_row2_left_to_right_again();
    test_all_indices_unique_4x4();
    test_all_indices_in_bounds_16x16();
    printf("all tests passed\n");
    return 0;
}
