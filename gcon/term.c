#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "term.h"

static const uint32_t col256[] = {
    0x000000, 0x00005f, 0x000087, 0x0000af, 0x0000d7, 0x0000ff, 0x005f00, 0x005f5f,
    0x005f87, 0x005faf, 0x005fd7, 0x005fff, 0x008700, 0x00875f, 0x008787, 0x0087af,
    0x0087d7, 0x0087ff, 0x00af00, 0x00af5f, 0x00af87, 0x00afaf, 0x00afd7, 0x00afff,
    0x00d700, 0x00d75f, 0x00d787, 0x00d7af, 0x00d7d7, 0x00d7ff, 0x00ff00, 0x00ff5f,
    0x00ff87, 0x00ffaf, 0x00ffd7, 0x00ffff, 0x5f0000, 0x5f005f, 0x5f0087, 0x5f00af,
    0x5f00d7, 0x5f00ff, 0x5f5f00, 0x5f5f5f, 0x5f5f87, 0x5f5faf, 0x5f5fd7, 0x5f5fff,
    0x5f8700, 0x5f875f, 0x5f8787, 0x5f87af, 0x5f87d7, 0x5f87ff, 0x5faf00, 0x5faf5f,
    0x5faf87, 0x5fafaf, 0x5fafd7, 0x5fafff, 0x5fd700, 0x5fd75f, 0x5fd787, 0x5fd7af,
    0x5fd7d7, 0x5fd7ff, 0x5fff00, 0x5fff5f, 0x5fff87, 0x5fffaf, 0x5fffd7, 0x5fffff,
    0x870000, 0x87005f, 0x870087, 0x8700af, 0x8700d7, 0x8700ff, 0x875f00, 0x875f5f,
    0x875f87, 0x875faf, 0x875fd7, 0x875fff, 0x878700, 0x87875f, 0x878787, 0x8787af,
    0x8787d7, 0x8787ff, 0x87af00, 0x87af5f, 0x87af87, 0x87afaf, 0x87afd7, 0x87afff,
    0x87d700, 0x87d75f, 0x87d787, 0x87d7af, 0x87d7d7, 0x87d7ff, 0x87ff00, 0x87ff5f,
    0x87ff87, 0x87ffaf, 0x87ffd7, 0x87ffff, 0xaf0000, 0xaf005f, 0xaf0087, 0xaf00af,
    0xaf00d7, 0xaf00ff, 0xaf5f00, 0xaf5f5f, 0xaf5f87, 0xaf5faf, 0xaf5fd7, 0xaf5fff,
    0xaf8700, 0xaf875f, 0xaf8787, 0xaf87af, 0xaf87d7, 0xaf87ff, 0xafaf00, 0xafaf5f,
    0xafaf87, 0xafafaf, 0xafafd7, 0xafafff, 0xafd700, 0xafd75f, 0xafd787, 0xafd7af,
    0xafd7d7, 0xafd7ff, 0xafff00, 0xafff5f, 0xafff87, 0xafffaf, 0xafffd7, 0xafffff,
    0xd70000, 0xd7005f, 0xd70087, 0xd700af, 0xd700d7, 0xd700ff, 0xd75f00, 0xd75f5f,
    0xd75f87, 0xd75faf, 0xd75fd7, 0xd75fff, 0xd78700, 0xd7875f, 0xd78787, 0xd787af,
    0xd787d7, 0xd787ff, 0xd7af00, 0xd7af5f, 0xd7af87, 0xd7afaf, 0xd7afd7, 0xd7afff,
    0xd7d700, 0xd7d75f, 0xd7d787, 0xd7d7af, 0xd7d7d7, 0xd7d7ff, 0xd7ff00, 0xd7ff5f,
    0xd7ff87, 0xd7ffaf, 0xd7ffd7, 0xd7ffff, 0xff0000, 0xff005f, 0xff0087, 0xff00af,
    0xff00d7, 0xff00ff, 0xff5f00, 0xff5f5f, 0xff5f87, 0xff5faf, 0xff5fd7, 0xff5fff,
    0xff8700, 0xff875f, 0xff8787, 0xff87af, 0xff87d7, 0xff87ff, 0xffaf00, 0xffaf5f,
    0xffaf87, 0xffafaf, 0xffafd7, 0xffafff, 0xffd700, 0xffd75f, 0xffd787, 0xffd7af,
    0xffd7d7, 0xffd7ff, 0xffff00, 0xffff5f, 0xffff87, 0xffffaf, 0xffffd7, 0xffffff,
    0x080808, 0x121212, 0x1c1c1c, 0x262626, 0x303030, 0x3a3a3a, 0x444444, 0x4e4e4e,
    0x585858, 0x626262, 0x6c6c6c, 0x767676, 0x808080, 0x8a8a8a, 0x949494, 0x9e9e9e,
    0xa8a8a8, 0xb2b2b2, 0xbcbcbc, 0xc6c6c6, 0xd0d0d0, 0xdadada, 0xe4e4e4, 0xeeeeee
};

// Tries to implement this standard for terminfo
// https://man7.org/linux/man-pages/man4/console_codes.4.html

#define CHARSET_DEFAULT 0
#define CHARSET_DEC_SPECIAL 1

void term_context_reinit(struct term_context *ctx) {
    ctx->tab_size = 8;
    ctx->autoflush = true;
    ctx->scroll_enabled = true;
    ctx->control_sequence = false;
    ctx->csi = false;
    ctx->escape = false;
    ctx->osc = false;
    ctx->osc_escape = false;
    ctx->rrr = false;
    ctx->discard_next = false;
    ctx->bold = false;
    ctx->bg_bold = false;
    ctx->reverse_video = false;
    ctx->dec_private = false;
    ctx->insert_mode = false;
    ctx->g_select = 0;
    ctx->charsets[0] = CHARSET_DEFAULT;
    ctx->charsets[1] = CHARSET_DEC_SPECIAL;
    ctx->current_charset = 0;
    ctx->escape_offset = 0;
    ctx->esc_values_i = 0;
    ctx->saved_cursor_x = 0;
    ctx->saved_cursor_y = 0;
    ctx->current_primary = (size_t)-1;
    ctx->current_bg = (size_t)-1;
    ctx->scroll_top_margin = 0;
    ctx->scroll_bottom_margin = ctx->rows;
}

static void term_putchar(struct term_context *ctx, uint8_t c);

void term_write(struct term_context *ctx, const char *buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
        term_putchar(ctx, buf[i]);
    }

    if (ctx->autoflush) {
        ctx->double_buffer_flush(ctx);
    }
}

static void sgr(struct term_context *ctx) {
    size_t i = 0;

    if (!ctx->esc_values_i)
        goto def;

    for (; i < ctx->esc_values_i; i++) {
        size_t offset;

        if (ctx->esc_values[i] == 0) {
def:
            if (ctx->reverse_video) {
                ctx->reverse_video = false;
                ctx->swap_palette(ctx);
            }
            ctx->bold = false;
            ctx->bg_bold = false;
            ctx->current_primary = (size_t)-1;
            ctx->current_bg = (size_t)-1;
            ctx->set_text_bg_default(ctx);
            ctx->set_text_fg_default(ctx);
            continue;
        }

        else if (ctx->esc_values[i] == 1) {
            ctx->bold = true;
            if (ctx->current_primary != (size_t)-1) {
                if (!ctx->reverse_video) {
                    ctx->set_text_fg_bright(ctx, ctx->current_primary);
                } else {
                    ctx->set_text_bg_bright(ctx, ctx->current_primary);
                }
            }
            continue;
        }

        else if (ctx->esc_values[i] == 5) {
            ctx->bg_bold = true;
            if (ctx->current_bg != (size_t)-1) {
                if (!ctx->reverse_video) {
                    ctx->set_text_bg_bright(ctx, ctx->current_bg);
                } else {
                    ctx->set_text_fg_bright(ctx, ctx->current_bg);
                }
            }
            continue;
        }

        else if (ctx->esc_values[i] == 22) {
            ctx->bold = false;
            if (ctx->current_primary != (size_t)-1) {
                if (!ctx->reverse_video) {
                    ctx->set_text_fg(ctx, ctx->current_primary);
                } else {
                    ctx->set_text_bg(ctx, ctx->current_primary);
                }
            }
            continue;
        }

        else if (ctx->esc_values[i] == 25) {
            ctx->bg_bold = false;
            if (ctx->current_bg != (size_t)-1) {
                if (!ctx->reverse_video) {
                    ctx->set_text_bg(ctx, ctx->current_bg);
                } else {
                    ctx->set_text_fg(ctx, ctx->current_bg);
                }
            }
            continue;
        }

        else if (ctx->esc_values[i] >= 30 && ctx->esc_values[i] <= 37) {
            offset = 30;
            ctx->current_primary = ctx->esc_values[i] - offset;

            if (ctx->reverse_video) {
                goto set_bg;
            }

set_fg:
            if ((ctx->bold && !ctx->reverse_video)
             || (ctx->bg_bold && ctx->reverse_video)) {
                ctx->set_text_fg_bright(ctx, ctx->esc_values[i] - offset);
            } else {
                ctx->set_text_fg(ctx, ctx->esc_values[i] - offset);
            }
            continue;
        }

        else if (ctx->esc_values[i] >= 40 && ctx->esc_values[i] <= 47) {
            offset = 40;
            ctx->current_bg = ctx->esc_values[i] - offset;

            if (ctx->reverse_video) {
                goto set_fg;
            }

set_bg:
            if ((ctx->bold && ctx->reverse_video)
             || (ctx->bg_bold && !ctx->reverse_video)) {
                ctx->set_text_bg_bright(ctx, ctx->esc_values[i] - offset);
            } else {
                ctx->set_text_bg(ctx, ctx->esc_values[i] - offset);
            }
            continue;
        }

        else if (ctx->esc_values[i] >= 90 && ctx->esc_values[i] <= 97) {
            offset = 90;
            ctx->current_primary = ctx->esc_values[i] - offset;

            if (ctx->reverse_video) {
                goto set_bg_bright;
            }

set_fg_bright:
            ctx->set_text_fg_bright(ctx, ctx->esc_values[i] - offset);
            continue;
        }

        else if (ctx->esc_values[i] >= 100 && ctx->esc_values[i] <= 107) {
            offset = 100;
            ctx->current_bg = ctx->esc_values[i] - offset;

            if (ctx->reverse_video) {
                goto set_fg_bright;
            }

set_bg_bright:
            ctx->set_text_bg_bright(ctx, ctx->esc_values[i] - offset);
            continue;
        }

        else if (ctx->esc_values[i] == 39) {
            ctx->current_primary = (size_t)-1;

            if (ctx->reverse_video) {
                ctx->swap_palette(ctx);
            }

            ctx->set_text_fg_default(ctx);

            if (ctx->reverse_video) {
                ctx->swap_palette(ctx);
            }

            continue;
        }

        else if (ctx->esc_values[i] == 49) {
            ctx->current_bg = (size_t)-1;

            if (ctx->reverse_video) {
                ctx->swap_palette(ctx);
            }

            ctx->set_text_bg_default(ctx);

            if (ctx->reverse_video) {
                ctx->swap_palette(ctx);
            }

            continue;
        }

        else if (ctx->esc_values[i] == 7) {
            if (!ctx->reverse_video) {
                ctx->reverse_video = true;
                ctx->swap_palette(ctx);
            }
            continue;
        }

        else if (ctx->esc_values[i] == 27) {
            if (ctx->reverse_video) {
                ctx->reverse_video = false;
                ctx->swap_palette(ctx);
            }
            continue;
        }

        // 256/RGB
        else if (ctx->esc_values[i] == 38 || ctx->esc_values[i] == 48) {
            bool fg = ctx->esc_values[i] == 38;

            i++;
            if (i >= ctx->esc_values_i) {
                break;
            }

            switch (ctx->esc_values[i]) {
                case 2: { // RGB
                    if (i + 3 >= ctx->esc_values_i) {
                        goto out;
                    }

                    uint32_t rgb_value = 0;

                    rgb_value |= ctx->esc_values[i + 1] << 16;
                    rgb_value |= ctx->esc_values[i + 2] << 8;
                    rgb_value |= ctx->esc_values[i + 3];

                    i += 3;

                    (fg ? ctx->set_text_fg_rgb : ctx->set_text_bg_rgb)(ctx, rgb_value);

                    break;
                }
                case 5: { // 256 colors
                    if (i + 1 >= ctx->esc_values_i) {
                        goto out;
                    }

                    uint32_t col = ctx->esc_values[i + 1];

                    i++;

                    if (col < 8) {
                        (fg ? ctx->set_text_fg : ctx->set_text_bg)(ctx, col);
                    } else if (col < 16) {
                        (fg ? ctx->set_text_fg_bright : ctx->set_text_bg_bright)(ctx, col - 8);
                    } else {
                        uint32_t rgb_value = col256[col - 16];
                        (fg ? ctx->set_text_fg_rgb : ctx->set_text_bg_rgb)(ctx, rgb_value);
                    }

                    break;
                }
                default: continue;
            }
        }
    }

out:;
}

static void dec_private_parse(struct term_context *ctx, uint8_t c) {
    ctx->dec_private = false;

    if (ctx->esc_values_i == 0) {
        return;
    }

    bool set;

    switch (c) {
        case 'h':
            set = true; break;
        case 'l':
            set = false; break;
        default:
            return;
    }

    switch (ctx->esc_values[0]) {
        case 25: {
            if (set) {
                ctx->enable_cursor(ctx);
            } else {
                ctx->disable_cursor(ctx);
            }
            return;
        }
    }

    if (ctx->callback != NULL) {
        ctx->callback(ctx, TERM_CB_DEC, ctx->esc_values_i, (uintptr_t)ctx->esc_values, c);
    }
}

static void linux_private_parse(struct term_context *ctx) {
    if (ctx->esc_values_i == 0) {
        return;
    }

    if (ctx->callback != NULL) {
        ctx->callback(ctx, TERM_CB_LINUX, ctx->esc_values_i, (uintptr_t)ctx->esc_values, 0);
    }
}

static void mode_toggle(struct term_context *ctx, uint8_t c) {
    if (ctx->esc_values_i == 0) {
        return;
    }

    bool set;

    switch (c) {
        case 'h':
            set = true; break;
        case 'l':
            set = false; break;
        default:
            return;
    }

    switch (ctx->esc_values[0]) {
        case 4:
            ctx->insert_mode = set; return;
    }

    if (ctx->callback != NULL) {
        ctx->callback(ctx, TERM_CB_MODE, ctx->esc_values_i, (uintptr_t)ctx->esc_values, c);
    }
}

static void osc_parse(struct term_context *ctx, uint8_t c) {
    if (ctx->osc_escape && c == '\\') {
        goto cleanup;
    }

    ctx->osc_escape = false;

    switch (c) {
        case 0x1b:
            ctx->osc_escape = true;
            break;
        case '\a':
            goto cleanup;
    }

    return;

cleanup:
    ctx->osc_escape = false;
    ctx->osc = false;
    ctx->escape = false;
}

static void control_sequence_parse(struct term_context *ctx, uint8_t c) {
    if (ctx->escape_offset == 2) {
        switch (c) {
            case '[':
                ctx->discard_next = true;
                goto cleanup;
            case '?':
                ctx->dec_private = true;
                return;
        }
    }

    if (c >= '0' && c <= '9') {
        if (ctx->esc_values_i == TERM_MAX_ESC_VALUES) {
            return;
        }
        ctx->rrr = true;
        ctx->esc_values[ctx->esc_values_i] *= 10;
        ctx->esc_values[ctx->esc_values_i] += c - '0';
        return;
    }

    if (ctx->rrr == true) {
        ctx->esc_values_i++;
        ctx->rrr = false;
        if (c == ';')
            return;
    } else if (c == ';') {
        if (ctx->esc_values_i == TERM_MAX_ESC_VALUES) {
            return;
        }
        ctx->esc_values[ctx->esc_values_i] = 0;
        ctx->esc_values_i++;
        return;
    }

    size_t esc_default;
    switch (c) {
        case 'J': case 'K': case 'q':
            esc_default = 0; break;
        default:
            esc_default = 1; break;
    }

    for (size_t i = ctx->esc_values_i; i < TERM_MAX_ESC_VALUES; i++) {
        ctx->esc_values[i] = esc_default;
    }

    if (ctx->dec_private == true) {
        dec_private_parse(ctx, c);
        goto cleanup;
    }

    bool r = ctx->scroll_enabled;
    ctx->scroll_enabled = false;
    size_t x, y;
    ctx->get_cursor_pos(ctx, &x, &y);

    switch (c) {
        case 'F':
            x = 0;
            // FALLTHRU
        case 'A': {
            if (ctx->esc_values[0] > y)
                ctx->esc_values[0] = y;
            size_t orig_y = y;
            size_t dest_y = y - ctx->esc_values[0];
            bool will_be_in_scroll_region = false;
            if ((ctx->scroll_top_margin >= dest_y && ctx->scroll_top_margin <= orig_y)
             || (ctx->scroll_bottom_margin >= dest_y && ctx->scroll_bottom_margin <= orig_y)) {
                will_be_in_scroll_region = true;
            }
            if (will_be_in_scroll_region && dest_y < ctx->scroll_top_margin) {
                dest_y = ctx->scroll_top_margin;
            }
            ctx->set_cursor_pos(ctx, x, dest_y);
            break;
        }
        case 'E':
            x = 0;
            // FALLTHRU
        case 'e':
        case 'B': {
            if (y + ctx->esc_values[0] > ctx->rows - 1)
                ctx->esc_values[0] = (ctx->rows - 1) - y;
            size_t orig_y = y;
            size_t dest_y = y + ctx->esc_values[0];
            bool will_be_in_scroll_region = false;
            if ((ctx->scroll_top_margin >= orig_y && ctx->scroll_top_margin <= dest_y)
             || (ctx->scroll_bottom_margin >= orig_y && ctx->scroll_bottom_margin <= dest_y)) {
                will_be_in_scroll_region = true;
            }
            if (will_be_in_scroll_region && dest_y >= ctx->scroll_bottom_margin) {
                dest_y = ctx->scroll_bottom_margin - 1;
            }
            ctx->set_cursor_pos(ctx, x, dest_y);
            break;
        }
        case 'a':
        case 'C':
            if (x + ctx->esc_values[0] > ctx->cols - 1)
                ctx->esc_values[0] = (ctx->cols - 1) - x;
            ctx->set_cursor_pos(ctx, x + ctx->esc_values[0], y);
            break;
        case 'D':
            if (ctx->esc_values[0] > x)
                ctx->esc_values[0] = x;
            ctx->set_cursor_pos(ctx, x - ctx->esc_values[0], y);
            break;
        case 'c':
            if (ctx->callback != NULL) {
                ctx->callback(ctx, TERM_CB_PRIVATE_ID, 0, 0, 0);
            }
            break;
        case 'd':
            ctx->esc_values[0] -= 1;
            if (ctx->esc_values[0] >= ctx->rows)
                ctx->esc_values[0] = ctx->rows - 1;
            ctx->set_cursor_pos(ctx, x, ctx->esc_values[0]);
            break;
        case 'G':
        case '`':
            ctx->esc_values[0] -= 1;
            if (ctx->esc_values[0] >= ctx->cols)
                ctx->esc_values[0] = ctx->cols - 1;
            ctx->set_cursor_pos(ctx, ctx->esc_values[0], y);
            break;
        case 'H':
        case 'f':
            ctx->esc_values[0] -= 1;
            ctx->esc_values[1] -= 1;
            if (ctx->esc_values[1] >= ctx->cols)
                ctx->esc_values[1] = ctx->cols - 1;
            if (ctx->esc_values[0] >= ctx->rows)
                ctx->esc_values[0] = ctx->rows - 1;
            ctx->set_cursor_pos(ctx, ctx->esc_values[1], ctx->esc_values[0]);
            break;
        case 'M':
            for (size_t i = 0; i < ctx->esc_values[0]; i++) {
                ctx->scroll(ctx);
            }
            break;
        case 'L': {
            size_t old_scroll_top_margin = ctx->scroll_top_margin;
            ctx->scroll_top_margin = y + 1;
            for (size_t i = 0; i < ctx->esc_values[0]; i++) {
                ctx->revscroll(ctx);
            }
            ctx->scroll_top_margin = old_scroll_top_margin;
            break;
        }
        case 'n':
            switch (ctx->esc_values[0]) {
                case 5:
                    if (ctx->callback != NULL) {
                        ctx->callback(ctx, TERM_CB_STATUS_REPORT, 0, 0, 0);
                    }
                    break;
                case 6:
                    if (ctx->callback != NULL) {
                        ctx->callback(ctx, TERM_CB_POS_REPORT, x + 1, y + 1, 0);
                    }
                    break;
            }
            break;
        case 'q':
            if (ctx->callback != NULL) {
                ctx->callback(ctx, TERM_CB_KBD_LEDS, ctx->esc_values[0], 0, 0);
            }
            break;
        case 'J':
            switch (ctx->esc_values[0]) {
                case 0: {
                    size_t rows_remaining = ctx->rows - (y + 1);
                    size_t cols_diff = ctx->cols - (x + 1);
                    size_t to_clear = rows_remaining * ctx->cols + cols_diff + 1;
                    for (size_t i = 0; i < to_clear; i++) {
                        ctx->raw_putchar(ctx, ' ');
                    }
                    ctx->set_cursor_pos(ctx, x, y);
                    break;
                }
                case 1: {
                    ctx->set_cursor_pos(ctx, 0, 0);
                    bool b = false;
                    for (size_t yc = 0; yc < ctx->rows; yc++) {
                        for (size_t xc = 0; xc < ctx->cols; xc++) {
                            ctx->raw_putchar(ctx, ' ');
                            if (xc == x && yc == y) {
                                ctx->set_cursor_pos(ctx, x, y);
                                b = true;
                                break;
                            }
                        }
                        if (b == true)
                            break;
                    }
                    break;
                }
                case 2:
                case 3:
                    ctx->clear(ctx, false);
                    break;
            }
            break;
        case '@':
            for (size_t i = ctx->cols - 1; ; i--) {
                ctx->move_character(ctx, i + ctx->esc_values[0], y, i, y);
                ctx->set_cursor_pos(ctx, i, y);
                ctx->raw_putchar(ctx, ' ');
                if (i == x) {
                    break;
                }
            }
            ctx->set_cursor_pos(ctx, x, y);
            break;
        case 'P':
            for (size_t i = x + ctx->esc_values[0]; i < ctx->cols; i++)
                ctx->move_character(ctx, i - ctx->esc_values[0], y, i, y);
            ctx->set_cursor_pos(ctx, ctx->cols - ctx->esc_values[0], y);
            // FALLTHRU
        case 'X':
            for (size_t i = 0; i < ctx->esc_values[0]; i++)
                ctx->raw_putchar(ctx, ' ');
            ctx->set_cursor_pos(ctx, x, y);
            break;
        case 'm':
            sgr(ctx);
            break;
        case 's':
            ctx->get_cursor_pos(ctx, &ctx->saved_cursor_x, &ctx->saved_cursor_y);
            break;
        case 'u':
            ctx->set_cursor_pos(ctx, ctx->saved_cursor_x, ctx->saved_cursor_y);
            break;
        case 'K':
            switch (ctx->esc_values[0]) {
                case 0: {
                    for (size_t i = x; i < ctx->cols; i++)
                        ctx->raw_putchar(ctx, ' ');
                    ctx->set_cursor_pos(ctx, x, y);
                    break;
                }
                case 1: {
                    ctx->set_cursor_pos(ctx, 0, y);
                    for (size_t i = 0; i < x; i++)
                        ctx->raw_putchar(ctx, ' ');
                    break;
                }
                case 2: {
                    ctx->set_cursor_pos(ctx, 0, y);
                    for (size_t i = 0; i < ctx->cols; i++)
                        ctx->raw_putchar(ctx, ' ');
                    ctx->set_cursor_pos(ctx, x, y);
                    break;
                }
            }
            break;
        case 'r':
            ctx->scroll_top_margin = 0;
            ctx->scroll_bottom_margin = ctx->rows;
            if (ctx->esc_values_i > 0) {
                ctx->scroll_top_margin = ctx->esc_values[0] - 1;
            }
            if (ctx->esc_values_i > 1) {
                ctx->scroll_bottom_margin = ctx->esc_values[1];
            }
            if (ctx->scroll_top_margin >= ctx->rows
             || ctx->scroll_bottom_margin > ctx->rows
             || ctx->scroll_top_margin >= (ctx->scroll_bottom_margin - 1)) {
                ctx->scroll_top_margin = 0;
                ctx->scroll_bottom_margin = ctx->rows;
            }
            ctx->set_cursor_pos(ctx, 0, 0);
            break;
        case 'l':
        case 'h':
            mode_toggle(ctx, c);
            break;
        case ']':
            linux_private_parse(ctx);
            break;
    }

    ctx->scroll_enabled = r;

cleanup:
    ctx->control_sequence = false;
    ctx->escape = false;
}

static void restore_state(struct term_context *ctx) {
    ctx->bold = ctx->saved_state_bold;
    ctx->bg_bold = ctx->saved_state_bg_bold;
    ctx->reverse_video = ctx->saved_state_reverse_video;
    ctx->current_charset = ctx->saved_state_current_charset;
    ctx->current_primary = ctx->saved_state_current_primary;
    ctx->current_bg = ctx->saved_state_current_bg;

    ctx->restore_state(ctx);
}

static void save_state(struct term_context *ctx) {
    ctx->save_state(ctx);

    ctx->saved_state_bold = ctx->bold;
    ctx->saved_state_bg_bold = ctx->bg_bold;
    ctx->saved_state_reverse_video = ctx->reverse_video;
    ctx->saved_state_current_charset = ctx->current_charset;
    ctx->saved_state_current_primary = ctx->current_primary;
    ctx->saved_state_current_bg = ctx->current_bg;
}

static void escape_parse(struct term_context *ctx, uint8_t c) {
    ctx->escape_offset++;

    if (ctx->osc == true) {
        osc_parse(ctx, c);
        return;
    }

    if (ctx->control_sequence == true) {
        control_sequence_parse(ctx, c);
        return;
    }

    if (ctx->csi == true) {
        ctx->csi = false;
        goto is_csi;
    }

    size_t x, y;
    ctx->get_cursor_pos(ctx, &x, &y);

    switch (c) {
        case ']':
            ctx->osc_escape = false;
            ctx->osc = true;
            return;
        case '[':
is_csi:
            for (size_t i = 0; i < TERM_MAX_ESC_VALUES; i++)
                ctx->esc_values[i] = 0;
            ctx->esc_values_i = 0;
            ctx->rrr = false;
            ctx->control_sequence = true;
            return;
        case '7':
            save_state(ctx);
            break;
        case '8':
            restore_state(ctx);
            break;
        case 'c':
            term_context_reinit(ctx);
            ctx->clear(ctx, true);
            break;
        case 'D':
            if (y == ctx->scroll_bottom_margin - 1) {
                ctx->scroll(ctx);
                ctx->set_cursor_pos(ctx, x, y);
            } else {
                ctx->set_cursor_pos(ctx, x, y + 1);
            }
            break;
        case 'E':
            if (y == ctx->scroll_bottom_margin - 1) {
                ctx->scroll(ctx);
                ctx->set_cursor_pos(ctx, 0, y);
            } else {
                ctx->set_cursor_pos(ctx, 0, y + 1);
            }
            break;
        case 'M':
            // "Reverse linefeed"
            if (y == ctx->scroll_top_margin) {
                ctx->revscroll(ctx);
                ctx->set_cursor_pos(ctx, 0, y);
            } else {
                ctx->set_cursor_pos(ctx, 0, y - 1);
            }
            break;
        case 'Z':
            if (ctx->callback != NULL) {
                ctx->callback(ctx, TERM_CB_PRIVATE_ID, 0, 0, 0);
            }
            break;
        case '(':
        case ')':
            ctx->g_select = c - '\'';
            break;
        case 0x1b:
            if (ctx->in_bootloader == true) {
                ctx->raw_putchar(ctx, c);
            }
            break;
    }

    ctx->escape = false;
}

static uint8_t dec_special_to_cp437(uint8_t c) {
    switch (c) {
        case '`': return 0x04;
        case '0': return 0xdb;
        case '-': return 0x18;
        case ',': return 0x1b;
        case '.': return 0x19;
        case 'a': return 0xb1;
        case 'f': return 0xf8;
        case 'g': return 0xf1;
        case 'h': return 0xb0;
        case 'j': return 0xd9;
        case 'k': return 0xbf;
        case 'l': return 0xda;
        case 'm': return 0xc0;
        case 'n': return 0xc5;
        case 'q': return 0xc4;
        case 's': return 0x5f;
        case 't': return 0xc3;
        case 'u': return 0xb4;
        case 'v': return 0xc1;
        case 'w': return 0xc2;
        case 'x': return 0xb3;
        case 'y': return 0xf3;
        case 'z': return 0xf2;
        case '~': return 0xfa;
        case '_': return 0xff;
        case '+': return 0x1a;
        case '{': return 0xe3;
        case '}': return 0x9c;
    }

    return c;
}

static void term_putchar(struct term_context *ctx, uint8_t c) {
    if (ctx->discard_next || (ctx->in_bootloader == false && (c == 0x18 || c == 0x1a))) {
        ctx->discard_next = false;
        ctx->escape = false;
        ctx->csi = false;
        ctx->control_sequence = false;
        ctx->osc = false;
        ctx->osc_escape = false;
        ctx->g_select = 0;
        return;
    }

    if (ctx->escape == true) {
        escape_parse(ctx, c);
        return;
    }

    if (ctx->g_select) {
        ctx->g_select--;
        switch (c) {
            case 'B':
                ctx->charsets[ctx->g_select] = CHARSET_DEFAULT; break;
            case '0':
                ctx->charsets[ctx->g_select] = CHARSET_DEC_SPECIAL; break;
        }
        ctx->g_select = 0;
        return;
    }

    size_t x, y;
    ctx->get_cursor_pos(ctx, &x, &y);

    switch (c) {
        case 0x00:
        case 0x7f:
            return;
        case 0x9b:
            ctx->csi = true;
            // FALLTHRU
        case 0x1b:
            ctx->escape_offset = 0;
            ctx->escape = true;
            return;
        case '\t':
            if ((x / ctx->tab_size + 1) >= ctx->cols) {
                ctx->set_cursor_pos(ctx, ctx->cols - 1, y);
                return;
            }
            ctx->set_cursor_pos(ctx, (x / ctx->tab_size + 1) * ctx->tab_size, y);
            return;
        case 0x0b:
        case 0x0c:
        case '\n':
            if (y == ctx->scroll_bottom_margin - 1) {
                ctx->scroll(ctx);
                ctx->set_cursor_pos(ctx, 0, y);
            } else {
                ctx->set_cursor_pos(ctx, 0, y + 1);
            }
            return;
        case '\b':
            ctx->set_cursor_pos(ctx, x - 1, y);
            return;
        case '\r':
            ctx->set_cursor_pos(ctx, 0, y);
            return;
        case '\a':
            // The bell is handled by the kernel
            if (ctx->callback != NULL) {
                ctx->callback(ctx, TERM_CB_BELL, 0, 0, 0);
            }
            return;
        case 14:
            // Move to G1 set
            ctx->current_charset = 1;
            return;
        case 15:
            // Move to G0 set
            ctx->current_charset = 0;
            return;
    }

    if (ctx->insert_mode == true) {
        for (size_t i = ctx->cols - 1; ; i--) {
            ctx->move_character(ctx, i + 1, y, i, y);
            if (i == x) {
                break;
            }
        }
    }

    // Translate character set
    switch (ctx->charsets[ctx->current_charset]) {
        case CHARSET_DEFAULT:
            break;
        case CHARSET_DEC_SPECIAL:
            c = dec_special_to_cp437(c);
    }

    ctx->raw_putchar(ctx, c);
}
