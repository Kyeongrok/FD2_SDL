// Demonstration: simple demo render using the image module (guarded)
#ifdef FD2_DEMO_RENDER
#include "../include/fd2_image.h"
/* Demo entry (not used by core build unless FD2_DEMO_RENDER is enabled) */
void fd2_demo_render_demo(byte* screen, int sw, int sh, const Palette* pal) {
    // Decode a tiny 2x2 bitmap from a static data and render to (0,0)
    const byte data[4] = {0, 128, 255, 60};
    Image* img = image_decode_bmp(data, 2, 2, pal);
    if (!img) return;
    image_render_to_screen(screen, sw, sh, 0, 0, img);
    image_free(img);
}
#endif
