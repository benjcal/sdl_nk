#include "SDL2/SDL.h"
#include "cairo.h"

#define NK_IMPLEMENTATION
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "nuklear.h"

#define SDL_NK_IMPLEMENTATION
#include "sdl_nk.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 900

float text_width_calculation(nk_handle handle, float height, const char *text, int len) {
    cairo_text_extents_t extents;
    cairo_text_extents(handle.ptr, text, &extents);

    return extents.width;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    int window_flags = SDL_WINDOW_MOUSE_FOCUS | SDL_WINDOW_INPUT_FOCUS;
    SDL_Window *window = SDL_CreateWindow("chip-8 emu", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
    
    SDL_Surface *window_surf = SDL_GetWindowSurface(window);
    cairo_surface_t *cairo_surf = cairo_image_surface_create_for_data(window_surf->pixels, CAIRO_FORMAT_RGB24, window_surf->w, window_surf->h, window_surf->pitch);
    cairo_t *cr = cairo_create(cairo_surf);

    // cairo font
    char * font_name = "Cantarell";
    int font_size = 16;
    cairo_select_font_face(cr, font_name, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, font_size);

    // nuklear font
    struct nk_user_font font;
    font.userdata.ptr = cr; // point to the cairo context to use it during in text_width_calculation
    font.height = font_size;
    font.width = text_width_calculation;

    // nuklear init
    struct nk_context ctx;
    nk_init_default(&ctx, &font);

    // state flags
    int running = 1;
    SDL_Event evt;
    
    while (running)
    {
        // inputs
        nk_input_begin(&ctx);
        while (SDL_PollEvent(&evt))
        {
            if (evt.type == SDL_QUIT) 
                running = 0;

            sdl_nk_handle_event(&ctx, &evt);
        }
        nk_input_end(&ctx);

        // gui
        if (nk_begin(&ctx, "Shown", nk_rect(50, 50, 320, 220), NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE)) {
            nk_layout_row_dynamic(&ctx, 18, 1);
            nk_label(&ctx, "Hello World!", NK_TEXT_LEFT);
        }
        nk_end(&ctx); // gui end

        // draw gui
        SDL_FillRect(window_surf, NULL, SDL_MapRGB(window_surf->format, 118, 118, 118));
        sld_nk_draw(&ctx, cr);

        // end and render
        SDL_UpdateWindowSurface(window);
        SDL_Delay(1000/60);

        nk_clear(&ctx);
    }
}
