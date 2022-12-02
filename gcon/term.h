#ifndef _TERM_H
#define _TERM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define TERM_MAX_ESC_VALUES 16

#define TERM_CB_DEC 10
#define TERM_CB_BELL 20
#define TERM_CB_PRIVATE_ID 30
#define TERM_CB_STATUS_REPORT 40
#define TERM_CB_POS_REPORT 50
#define TERM_CB_KBD_LEDS 60
#define TERM_CB_MODE 70
#define TERM_CB_LINUX 80

struct term_context {
    /* internal use */

    size_t tab_size;
    bool autoflush;
    bool scroll_enabled;
    bool control_sequence;
    bool csi;
    bool escape;
    bool osc;
    bool osc_escape;
    bool rrr;
    bool discard_next;
    bool bold;
    bool bg_bold;
    bool reverse_video;
    bool dec_private;
    bool insert_mode;
    uint64_t code_point;
    size_t unicode_remaining;
    uint8_t g_select;
    uint8_t charsets[2];
    size_t current_charset;
    size_t escape_offset;
    size_t esc_values_i;
    size_t saved_cursor_x;
    size_t saved_cursor_y;
    size_t current_primary;
    size_t current_bg;
    size_t scroll_top_margin;
    size_t scroll_bottom_margin;
    uint32_t esc_values[TERM_MAX_ESC_VALUES];
    bool saved_state_bold;
    bool saved_state_bg_bold;
    bool saved_state_reverse_video;
    size_t saved_state_current_charset;
    size_t saved_state_current_primary;
    size_t saved_state_current_bg;

    /* to be set by backend */

    size_t rows, cols;
    bool in_bootloader;

    void (*raw_putchar)(struct term_context *, uint8_t c);
    void (*clear)(struct term_context *, bool move);
    void (*enable_cursor)(struct term_context *);
    bool (*disable_cursor)(struct term_context *);
    void (*set_cursor_pos)(struct term_context *, size_t x, size_t y);
    void (*get_cursor_pos)(struct term_context *, size_t *x, size_t *y);
    void (*set_text_fg)(struct term_context *, size_t fg);
    void (*set_text_bg)(struct term_context *, size_t bg);
    void (*set_text_fg_bright)(struct term_context *, size_t fg);
    void (*set_text_bg_bright)(struct term_context *, size_t bg);
    void (*set_text_fg_rgb)(struct term_context *, uint32_t fg);
    void (*set_text_bg_rgb)(struct term_context *, uint32_t bg);
    void (*set_text_fg_default)(struct term_context *);
    void (*set_text_bg_default)(struct term_context *);
    void (*move_character)(struct term_context *, size_t new_x, size_t new_y, size_t old_x, size_t old_y);
    void (*scroll)(struct term_context *);
    void (*revscroll)(struct term_context *);
    void (*swap_palette)(struct term_context *);
    void (*save_state)(struct term_context *);
    void (*restore_state)(struct term_context *);
    void (*double_buffer_flush)(struct term_context *);
    void (*full_refresh)(struct term_context *);
    void (*deinit)(struct term_context *, void (*)(void *, size_t));

    /* to be set by client */

    void (*callback)(struct term_context *, uint64_t, uint64_t, uint64_t, uint64_t);
};

void term_context_reinit(struct term_context *ctx);
void term_write(struct term_context *ctx, const char *buf, size_t count);

#ifdef __cplusplus
}
#endif

#endif
