// Map-native spike: exact native replacement for TMapImage.DrawTerrain's
// composite stage. It deliberately does not decode cache data or build terrain
// planes; Simba still owns those semantics. This isolates the value of moving
// the 4x4 tile blits and BGRA conversion across the native boundary.
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#  define EXPORT __declspec(dllexport)
#else
#  define EXPORT __attribute__((visibility("default")))
#endif

enum { REGION = 64, SCALE = 4, CHUNK = 256, PLANE_SIZE = REGION * REGION };

typedef struct {
  uint8_t b, g, r, a;
} BGRA;

// The C ABI is intentionally all pointers + Int32. Simba passes @array[0] and
// TImage.Data directly, avoiding array copying and plugin-owned allocation.
EXPORT void MapNativeComposite(const int32_t *settings,
                               const int32_t *plane0,
                               const int32_t *plane1,
                               const int32_t *plane2,
                               const int32_t *plane3,
                               BGRA *dst, int32_t z) {
  const int32_t *planes[4] = { plane0, plane1, plane2, plane3 };
  for (int32_t x = 0; x < REGION; ++x) {
    for (int32_t y = 0; y < REGION; ++y) {
      const int32_t inverted_y = REGION - y - 1;
      const int32_t idx = x * REGION + inverted_y;
      const int32_t is_bridge = (settings[PLANE_SIZE + idx] & 2) != 0;
      int32_t tile_z = z + is_bridge;
      if (tile_z >= 4) continue;

      if ((settings[z * PLANE_SIZE + idx] & 24) == 0) {
        // Exactly the call order of DrawTerrain: bridge-underlay first, then
        // the effective plane. A source zero is transparent in both versions.
        if ((z == 0) && is_bridge) {
          const int32_t *src = planes[0];
          for (int32_t j = 0; j < SCALE; ++j) {
            int32_t p = (y * SCALE + j) * CHUNK + x * SCALE;
            for (int32_t i = 0; i < SCALE; ++i) {
              uint32_t argb = (uint32_t)src[p + i];
              if (argb) {
                dst[p + i].r = (uint8_t)(argb >> 16);
                dst[p + i].g = (uint8_t)(argb >> 8);
                dst[p + i].b = (uint8_t)argb;
                dst[p + i].a = 255;
              }
            }
          }
        }
        {
          const int32_t *src = planes[tile_z];
          for (int32_t j = 0; j < SCALE; ++j) {
            int32_t p = (y * SCALE + j) * CHUNK + x * SCALE;
            for (int32_t i = 0; i < SCALE; ++i) {
              uint32_t argb = (uint32_t)src[p + i];
              if (argb) {
                dst[p + i].r = (uint8_t)(argb >> 16);
                dst[p + i].g = (uint8_t)(argb >> 8);
                dst[p + i].b = (uint8_t)argb;
                dst[p + i].a = 255;
              }
            }
          }
        }
      }

      if ((tile_z < 3) && ((settings[(z + 1) * PLANE_SIZE + idx] & 8) != 0)) {
        const int32_t *src = planes[tile_z + 1];
        for (int32_t j = 0; j < SCALE; ++j) {
          int32_t p = (y * SCALE + j) * CHUNK + x * SCALE;
          for (int32_t i = 0; i < SCALE; ++i) {
            uint32_t argb = (uint32_t)src[p + i];
            if (argb) {
              dst[p + i].r = (uint8_t)(argb >> 16);
              dst[p + i].g = (uint8_t)(argb >> 8);
              dst[p + i].b = (uint8_t)argb;
              dst[p + i].a = 255;
            }
          }
        }
      }
    }
  }
}

EXPORT int32_t MapNativeAdd(int32_t a, int32_t b) { return a + b; }

// Simba's script plugin ABI: one C-ABI function declaration, no custom types,
// allocation hooks or lifecycle state. Returning an Int32 is sufficient; Simba
// reads the address/header out parameters.
EXPORT int32_t GetFunctionCount(void) { return 2; }

// Header is `var PChar` in Simba's ABI, hence a pointer to its allocated
// character-buffer pointer—not the buffer itself.
EXPORT int32_t GetFunctionInfo(int32_t index, void **address, char **header) {
  if (index == 0) {
    *address = (void *)&MapNativeComposite;
    strcpy(*header,
      "procedure MapNativeComposite(settings, plane0, plane1, plane2, plane3, dst: Pointer; z: Integer)");
    return 1;
  }
  if (index == 1) {
    *address = (void *)&MapNativeAdd;
    strcpy(*header, "function MapNativeAdd(a, b: Integer): Integer");
    return 1;
  }
  return 0;
}
