#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>


// Sprite Size select based on 0 -> 255 counter
//
// Range 0 -> 255 mapped to 0 -> 8
// Should roughly match Screen Y LUT
const uint8_t LUT_sprite_id_by_count[] = {
   // Num Sprites = 9, Scaler = 16
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

// Y Screen select based on 0 -> 255 counter
//
// Range 0 -> 255 mapped to (32 + 6) -> 143
const uint8_t LUT_y_pos_by_count[] = {
   // 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 38, 38, 38, 38, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 42, 42, 42, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 47, 47, 47, 47, 47, 48, 48, 48, 48, 49, 49, 50, 50, 50, 50, 50, 51, 51, 51, 51, 52, 52, 53, 54, 54, 54, 54, 54, 54, 55, 56, 56, 56, 56, 57, 57, 57, 57, 58, 59, 59, 60, 60, 60, 60, 60, 61, 61, 62, 62, 63, 63, 64, 64, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 70, 70, 71, 71, 71, 71, 72, 74, 74, 74, 74, 75, 75, 77, 77, 78, 78, 78, 80, 80, 81, 81, 82, 83, 83, 85, 85, 86, 86, 87, 87, 88, 90, 90, 92, 92, 93, 93, 95, 95, 96, 98, 98, 99, 99, 102, 102, 104, 106, 106, 108, 108, 111, 111, 113, 113, 116, 119, 119, 124, 124, 130, 130, 142, 143,
   38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 46, 46, 46, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 51, 51, 51, 52, 52, 52, 52, 52, 53, 53, 53, 53, 54, 54, 55, 55, 55, 55, 55, 56, 56, 56, 56, 57, 57, 58, 58, 58, 58, 58, 59, 59, 60, 60, 60, 60, 60, 61, 61, 62, 62, 63, 63, 63, 64, 64, 65, 65, 65, 66, 66, 66, 66, 67, 67, 68, 68, 69, 69, 69, 70, 70, 71, 71, 72, 72, 73, 74, 74, 74, 74, 75, 75, 76, 77, 77, 78, 78, 79, 79, 80, 80, 81, 82, 82, 83, 83, 84, 84, 85, 86, 86, 88, 88, 89, 89, 90, 90, 91, 93, 93, 94, 94, 96, 96, 97, 97, 99, 100, 100, 102, 102, 104, 104, 106, 107, 107, 110, 110, 113, 113, 115, 115, 118, 121, 121, 125, 125, 130, 130, 142, 143, 
};

