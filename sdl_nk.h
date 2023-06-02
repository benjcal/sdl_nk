#include "SDL2/SDL.h"
#include "cairo/cairo.h"

/* references:
/   https://gitlab.com/riribreizh/nk_pugl_cairo/-/blob/main/nk_pugl_cairo.h
/   https://github.com/space222/silo/blob/master/silo.cpp
/   https://github.com/Immediate-Mode-UI/Nuklear/blob/master/demo/sdl_opengl2/nuklear_sdl_gl2.h
*/

#define NK_DEG_TO_RAD(x) ((double) x * NK_PI / 180.0)
#define NK_TO_CAIRO(x) ((double) x / 255.0)

void sdl_nk_handle_event(struct nk_context *ctx, SDL_Event *evt);
void sld_nk_draw(struct nk_context *ctx, cairo_t *cr);

#ifdef SDL_NK_IMPLEMENTATION

void sdl_nk_handle_event(struct nk_context *ctx, SDL_Event *evt) {
    switch (evt->type) {
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            int down = evt->type == SDL_KEYDOWN;
            
            switch(evt->key.keysym.sym) {
                case SDLK_RSHIFT: /* RSHIFT & LSHIFT share same routine */
                case SDLK_LSHIFT:    nk_input_key(ctx, NK_KEY_SHIFT, down); break;
                case SDLK_RETURN:    nk_input_key(ctx, NK_KEY_ENTER, down); break;
                case SDLK_BACKSPACE: nk_input_key(ctx, NK_KEY_BACKSPACE, down); break;
            }
        } break;
        

        case SDL_MOUSEMOTION: nk_input_motion(ctx, evt->motion.x, evt->motion.y); break;

        case SDL_MOUSEBUTTONUP:     
        case SDL_MOUSEBUTTONDOWN: {
            int down = evt->type == SDL_MOUSEBUTTONDOWN;
            int x = evt->button.x;
            int y = evt->button.y;

            switch (evt->button.button) {
                case SDL_BUTTON_LEFT: nk_input_button(ctx, NK_BUTTON_LEFT, x, y, down); break;
                case SDL_BUTTON_MIDDLE: nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down); break;
                case SDL_BUTTON_RIGHT:  nk_input_button(ctx, NK_BUTTON_RIGHT, x, y, down); break;
            }
        } break;

        case SDL_MOUSEWHEEL: nk_input_scroll(ctx, nk_vec2((float)evt->wheel.x, (float)evt->wheel.y)); break;

        case SDL_TEXTINPUT: {
            nk_glyph glyph;
            memcpy(glyph, evt->text.text, NK_UTF_SIZE);
            nk_input_glyph(ctx, glyph);
        } break;
    }
}

void sld_nk_draw(struct nk_context *ctx, cairo_t *cr) {
    const struct nk_command *cmd;
    nk_foreach(cmd, ctx) {
        switch (cmd->type) {
            case NK_COMMAND_NOP: break;

            case NK_COMMAND_SCISSOR: {
                const struct nk_command_scissor *s = (const struct nk_command_scissor *)cmd;

                cairo_reset_clip(cr);
                if (s->x >= 0) {
                    cairo_rectangle(cr, s->x - 1, s->y - 1, s->w + 2, s->h + 2);
                    cairo_clip(cr);
                }
            } break;

            case NK_COMMAND_LINE: {
                const struct nk_command_line *l = (const struct nk_command_line *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(l->color.r), NK_TO_CAIRO(l->color.g), NK_TO_CAIRO(l->color.b), NK_TO_CAIRO(l->color.a));
                cairo_set_line_width(cr, l->line_thickness);
                cairo_move_to(cr, l->begin.x, l->begin.y);
                cairo_line_to(cr, l->end.x, l->end.y);
                cairo_stroke(cr);
            } break;

            case NK_COMMAND_CURVE: {
                const struct nk_command_curve *q = (const struct nk_command_curve *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(q->color.r), NK_TO_CAIRO(q->color.g), NK_TO_CAIRO(q->color.b), NK_TO_CAIRO(q->color.a));
                cairo_set_line_width(cr, q->line_thickness);
                cairo_move_to(cr, q->begin.x, q->begin.y);
                cairo_curve_to(cr, q->ctrl[0].x, q->ctrl[0].y, q->ctrl[1].x, q->ctrl[1].y, q->end.x, q->end.y);
                cairo_stroke(cr);
            } break;

            case NK_COMMAND_RECT: {
                const struct nk_command_rect *r = (const struct nk_command_rect *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(r->color.r), NK_TO_CAIRO(r->color.g), NK_TO_CAIRO(r->color.b), NK_TO_CAIRO(r->color.a));
                cairo_set_line_width(cr, r->line_thickness);
                if (r->rounding == 0) {
                    cairo_rectangle(cr, r->x, r->y, r->w, r->h);
                } else {
                    int xl = r->x + r->w - r->rounding;
                    int xr = r->x + r->rounding;
                    int yl = r->y + r->h - r->rounding;
                    int yr = r->y + r->rounding;
                    cairo_new_sub_path(cr);
                    cairo_arc(cr, xl, yr, r->rounding, NK_DEG_TO_RAD(-90), NK_DEG_TO_RAD(0));
                    cairo_arc(cr, xl, yl, r->rounding, NK_DEG_TO_RAD(0), NK_DEG_TO_RAD(90));
                    cairo_arc(cr, xr, yl, r->rounding, NK_DEG_TO_RAD(90), NK_DEG_TO_RAD(180));
                    cairo_arc(cr, xr, yr, r->rounding, NK_DEG_TO_RAD(180), NK_DEG_TO_RAD(270));
                    cairo_close_path(cr);
                }
                cairo_stroke(cr);
            } break;

            case NK_COMMAND_RECT_FILLED: {
                const struct nk_command_rect_filled *r = (const struct nk_command_rect_filled *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(r->color.r), NK_TO_CAIRO(r->color.g), NK_TO_CAIRO(r->color.b), NK_TO_CAIRO(r->color.a));
                if (r->rounding == 0) {
                    cairo_rectangle(cr, r->x, r->y, r->w, r->h);
                } else {
                    int xl = r->x + r->w - r->rounding;
                    int xr = r->x + r->rounding;
                    int yl = r->y + r->h - r->rounding;
                    int yr = r->y + r->rounding;
                    cairo_new_sub_path(cr);
                    cairo_arc(cr, xl, yr, r->rounding, NK_DEG_TO_RAD(-90), NK_DEG_TO_RAD(0));
                    cairo_arc(cr, xl, yl, r->rounding, NK_DEG_TO_RAD(0), NK_DEG_TO_RAD(90));
                    cairo_arc(cr, xr, yl, r->rounding, NK_DEG_TO_RAD(90), NK_DEG_TO_RAD(180));
                    cairo_arc(cr, xr, yr, r->rounding, NK_DEG_TO_RAD(180), NK_DEG_TO_RAD(270));
                    cairo_close_path(cr);
                }
                cairo_fill(cr);
            } break;



            case NK_COMMAND_CIRCLE: {
                const struct nk_command_circle *c = (const struct nk_command_circle *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(c->color.r), NK_TO_CAIRO(c->color.g), NK_TO_CAIRO(c->color.b), NK_TO_CAIRO(c->color.a));
                cairo_set_line_width(cr, c->line_thickness);
                cairo_save(cr);
                cairo_translate(cr, c->x + c->w / 2.0, c->y + c->h / 2.0);
                cairo_scale(cr, c->w / 2.0, c->h / 2.0);
                cairo_arc(cr, 0, 0, 1, NK_DEG_TO_RAD(0), NK_DEG_TO_RAD(360));
                cairo_restore(cr);
                cairo_stroke(cr);
            } break;

            case NK_COMMAND_CIRCLE_FILLED: {
                const struct nk_command_circle_filled *c = (const struct nk_command_circle_filled *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(c->color.r), NK_TO_CAIRO(c->color.g), NK_TO_CAIRO(c->color.b), NK_TO_CAIRO(c->color.a));
                cairo_save(cr);
                cairo_translate(cr, c->x + c->w / 2.0, c->y + c->h / 2.0);
                cairo_scale(cr, c->w / 2.0, c->h / 2.0);
                cairo_arc(cr, 0, 0, 1, NK_DEG_TO_RAD(0), NK_DEG_TO_RAD(360));
                cairo_restore(cr);
                cairo_fill(cr);
            } break;

            case NK_COMMAND_ARC: {
                const struct nk_command_arc *a = (const struct nk_command_arc*) cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(a->color.r), NK_TO_CAIRO(a->color.g), NK_TO_CAIRO(a->color.b), NK_TO_CAIRO(a->color.a));
                cairo_set_line_width(cr, a->line_thickness);
                cairo_arc(cr, a->cx, a->cy, a->r, NK_DEG_TO_RAD(a->a[0]), NK_DEG_TO_RAD(a->a[1]));
                cairo_stroke(cr);
            } break;

            case NK_COMMAND_ARC_FILLED: {
                const struct nk_command_arc_filled *a = (const struct nk_command_arc_filled*)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(a->color.r), NK_TO_CAIRO(a->color.g), NK_TO_CAIRO(a->color.b), NK_TO_CAIRO(a->color.a));
                cairo_arc(cr, a->cx, a->cy, a->r, NK_DEG_TO_RAD(a->a[0]), NK_DEG_TO_RAD(a->a[1]));
                cairo_fill(cr);
            } break;

            case NK_COMMAND_TRIANGLE: {
                const struct nk_command_triangle *t = (const struct nk_command_triangle *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(t->color.r), NK_TO_CAIRO(t->color.g), NK_TO_CAIRO(t->color.b), NK_TO_CAIRO(t->color.a));
                cairo_set_line_width(cr, t->line_thickness);
                cairo_move_to(cr, t->a.x, t->a.y);
                cairo_line_to(cr, t->b.x, t->b.y);
                cairo_line_to(cr, t->c.x, t->c.y);
                cairo_close_path(cr);
                cairo_stroke(cr);
            } break;

            case NK_COMMAND_TRIANGLE_FILLED: {
                const struct nk_command_triangle_filled *t = (const struct nk_command_triangle_filled *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(t->color.r), NK_TO_CAIRO(t->color.g), NK_TO_CAIRO(t->color.b), NK_TO_CAIRO(t->color.a));
                cairo_move_to(cr, t->a.x, t->a.y);
                cairo_line_to(cr, t->b.x, t->b.y);
                cairo_line_to(cr, t->c.x, t->c.y);
                cairo_close_path(cr);
                cairo_fill(cr);
            } break;

            case NK_COMMAND_POLYGON: {
                const struct nk_command_polygon *p = (const struct nk_command_polygon *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(p->color.r), NK_TO_CAIRO(p->color.g), NK_TO_CAIRO(p->color.b), NK_TO_CAIRO(p->color.a));
                cairo_set_line_width(cr, p->line_thickness);
                cairo_move_to(cr, p->points[0].x, p->points[0].y);
                for (int i = 1; i < p->point_count; ++i) {
                    cairo_line_to(cr, p->points[i].x, p->points[i].y);
                }
                cairo_close_path(cr);
                cairo_stroke(cr);
            } break;

            case NK_COMMAND_POLYGON_FILLED: {
                const struct nk_command_polygon_filled *p = (const struct nk_command_polygon_filled *)cmd;

                cairo_set_source_rgba (cr, NK_TO_CAIRO(p->color.r), NK_TO_CAIRO(p->color.g), NK_TO_CAIRO(p->color.b), NK_TO_CAIRO(p->color.a));
                cairo_move_to(cr, p->points[0].x, p->points[0].y);
                for (int i = 1; i < p->point_count; ++i) {
                    cairo_line_to(cr, p->points[i].x, p->points[i].y);
                }
                cairo_close_path(cr);
                cairo_fill(cr);
            } break;

            case NK_COMMAND_POLYLINE: {
                const struct nk_command_polyline *p = (const struct nk_command_polyline *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(p->color.r), NK_TO_CAIRO(p->color.g), NK_TO_CAIRO(p->color.b), NK_TO_CAIRO(p->color.a));
                cairo_set_line_width(cr, p->line_thickness);
                cairo_move_to(cr, p->points[0].x, p->points[0].y);
                for (int i = 1; i < p->point_count; ++i) {
                    cairo_line_to(cr, p->points[i].x, p->points[i].y);
                }
                cairo_stroke(cr);
            } break;

            case NK_COMMAND_TEXT: {
                const struct nk_command_text *t = (const struct nk_command_text *)cmd;

                cairo_set_source_rgba(cr, NK_TO_CAIRO(t->background.r), NK_TO_CAIRO(t->background.g), NK_TO_CAIRO(t->background.b), NK_TO_CAIRO(t->background.a));
                cairo_rectangle(cr, t->x, t->y, t->w, t->h);
                cairo_fill(cr);
                cairo_set_source_rgba(cr, NK_TO_CAIRO(t->foreground.r), NK_TO_CAIRO(t->foreground.g), NK_TO_CAIRO(t->foreground.b), NK_TO_CAIRO(t->foreground.a));
                cairo_move_to(cr, t->x, t->y + 16);
                cairo_text_path(cr, t->string);
                cairo_fill(cr);
            } break;

            case NK_COMMAND_IMAGE: {
                // TODO
            } break;

            case NK_COMMAND_CUSTOM: {
                // TODO
            } break;

            case NK_COMMAND_RECT_MULTI_COLOR: {
                // TODO
            } break;

        } // end switch
        


    }
}

#endif
