// TODO: Alignment
static u8 ui_stack[KB(16)];
static u8 *ui_stack_start;
static u8 *ui_stack_ptr;
static u32 ui_stack_bytes_used;

static struct RICO_ui_hud *ui_debug_usage_hud;
static struct RICO_ui_label *ui_debug_usage_bar;

enum ui_rect_fill_mode
{
    UI_RECT_FILLED,
    UI_RECT_OUTLINE
};

static void rico_ui_init()
{
    ui_stack_ptr = ui_stack;
    ui_debug_usage_hud = RICO_ui_hud();
    ui_debug_usage_bar = RICO_ui_label(&ui_debug_usage_hud->element);
    ui_debug_stack_usage();

    ui_stack_start = ui_stack_ptr;
    rico_ui_reset();
}

static void rico_ui_reset()
{
    ui_stack_ptr = ui_stack_start;
}

static void ui_debug_stack_usage()
{
    u32 ui_stack_used = (ui_stack_ptr - ui_stack) * 100 / sizeof(ui_stack);

    u32 bar_pad = 2;
    ui_debug_usage_bar->element.min_size = VEC2I(ui_stack_used, 8);
    ui_debug_usage_bar->element.margin = RECT1(bar_pad);
    ui_debug_usage_bar->element.color_default = COLOR_DARK_YELLOW;

    ui_debug_usage_hud->element.min_size = VEC2I(100 + bar_pad * 2,
                                                 8 + bar_pad * 2);
    ui_debug_usage_hud->element.color_default = COLOR_DARK_WHITE_HIGHLIGHT;
}

static inline void *ui_stack_push(u32 bytes)
{
    RICO_ASSERT(ui_stack_ptr + bytes < ui_stack + sizeof(ui_stack));
    void *ptr = ui_stack_ptr;
    ui_stack_ptr += bytes;
    memset(ptr, 0, bytes);
    ui_stack_bytes_used += bytes;
    return ptr;
}

static inline void *ui_stack_pop(void *data)
{
    ui_stack_ptr = data;
}

static void ui_draw_rect(u32 x, u32 y, const struct rect *rect,
                         enum ui_rect_fill_mode fill_mode,
                         const struct vec4 *color)
{
    if (fill_mode == UI_RECT_OUTLINE)
    {
        float sx = SCREEN_X(rect->x + x);
        float sy = SCREEN_Y(rect->y + y);
        float sw = SCREEN_W(rect->w);
        float sh = SCREEN_H(rect->h);
        RICO_prim_draw_line2d(sx     , sy    ,  sx + sw,  sy     , color);
        RICO_prim_draw_line2d(sx + sw, sy    ,  sx + sw,  sy + sh, color);
        RICO_prim_draw_line2d(sx + sw, sy + sh, sx     ,  sy + sh, color);
        RICO_prim_draw_line2d(sx     , sy + sh, sx     ,  sy     , color);
    }
    else
    {
        RICO_prim_draw_rect(&RECT(rect->x + x, rect->y + y, rect->w, rect->h),
                            color);
    }
}

static void *ui_push_element(u32 bytes, enum RICO_ui_element_type type,
                             struct RICO_ui_element *parent)
{
    struct RICO_ui_element *element = ui_stack_push(bytes);
    element->type = type;
    element->parent = parent;
    
    if (parent)
    {
        if (!element->parent->first_child)
        {
            element->parent->first_child = element;
        }
        else
        {
            element->parent->last_child->next = element;
            element->prev = element->parent->last_child;
        }
        element->parent->last_child = element;
    }

    return element;
}

extern struct RICO_ui_hud *RICO_ui_hud()
{
    return ui_push_element(sizeof(struct RICO_ui_hud), RICO_UI_ELEMENT_HUD,
                           NULL);
}

extern struct RICO_ui_button *RICO_ui_button(struct RICO_ui_element *parent)
{
    return ui_push_element(sizeof(struct RICO_ui_button),
                           RICO_UI_ELEMENT_BUTTON, parent);
}

extern struct RICO_ui_label *RICO_ui_label(struct RICO_ui_element *parent)
{
    return ui_push_element(sizeof(struct RICO_ui_label), RICO_UI_ELEMENT_LABEL,
                           parent);
}

static bool ui_layout_element(struct RICO_ui_element *element, u32 x, u32 y,
                              u32 max_w, u32 max_h)
{
    u32 margin_w = element->margin.left + element->margin.right;
    u32 margin_h = element->margin.top + element->margin.bottom;

    u32 pad_w = element->padding.left + element->padding.right;
    u32 pad_h = element->padding.top + element->padding.bottom;

    struct rect *size = &element->size;
    size->x = x;
    size->y = y;
    size->w = margin_w + MAX(element->min_size.w, pad_w);
    size->h = margin_h + MAX(element->min_size.h, pad_h);

    u32 start_x = size->x + element->margin.left + element->padding.left;
    u32 start_y = size->y + element->margin.top + element->padding.top;
    u32 start_w = margin_w + pad_w;
    u32 start_h = margin_h + pad_h;

    ////////////////////////////////////////////////////////////////////////////
    
    u32 next_x = start_x;
    u32 next_y = start_y;

    // TODO: Use rect->w and rect->h directly?
    u32 max_row_w = start_w;
    u32 used_h = start_h;

    u32 row_used_w = start_w;
    u32 row_max_h = 0;

    struct RICO_ui_element *child = element->first_child;
    if (child)
    {
        while (child)
        {
            // Calculate child bounds
            bool fit = ui_layout_element(child, next_x, next_y,
                                         max_w - row_used_w, max_h - used_h);
            if (fit)
            {
                // Move to next column
                next_x += child->size.w;

                // Track max row width
                row_used_w += child->size.w;
                if (row_used_w > max_row_w)
                {
                    max_row_w = row_used_w;
                }

                // Track height of tallest child element in this row
                if (child->size.h > row_max_h)
                {
                    row_max_h = child->size.h;
                }

                child = child->next;
            }
            else if (row_used_w > 0 && row_max_h > 0)
            {
                // Move to next row
                next_x = start_x;
                next_y += row_max_h;

                used_h += row_max_h;

                row_used_w = start_w;
                row_max_h = 0;
            }
            else
            {
                return false;
            }
        }

        used_h += row_max_h;
    }

    if (max_row_w > size->w)
    {
        size->w = max_row_w;
    }
    if (used_h > size->h)
    {
        size->h = used_h;
    }

    bool fit = size->w <= max_w && size->h <= max_h;
    if (fit)
    {
        element->bounds = element->size;
        element->bounds.x += element->margin.left;
        element->bounds.y += element->margin.top;
        element->bounds.w -= margin_w;
        element->bounds.h -= margin_h;
    }
    return fit;
}

static inline bool rect_intersects(const struct rect *rect, s32 x, s32 y)
{
    bool hover = x >= rect->x &&
                 y >= rect->y &&
                 x < rect->x + (s32)rect->w &&
                 y < rect->y + (s32)rect->h;
    return hover;
}

static inline bool ui_element_hover(struct RICO_ui_element *element, s32 x,
                                    s32 y)
{
    return rect_intersects(&element->bounds, mouse_x - x, mouse_y - y);
}

static void ui_draw_hud(struct RICO_ui_hud *ctrl, u32 x, u32 y)
{
    ui_draw_rect(x, y, &ctrl->element.bounds, UI_RECT_FILLED,
                 &ctrl->element.color);
}

static void ui_draw_button(struct RICO_ui_button *ctrl, u32 x, u32 y)
{
    struct rect rect = ctrl->element.bounds;
    rect.x += x;
    rect.y += y;
    RICO_prim_draw_sprite(&rect, &ctrl->sprite, &ctrl->element.color);
}

static void ui_draw_label(struct RICO_ui_label *ctrl, u32 x, u32 y)
{
    ui_draw_rect(x, y, &ctrl->element.bounds, UI_RECT_FILLED,
                 &ctrl->element.color);
}

static void ui_draw_element(struct RICO_ui_element *element, u32 x, u32 y)
{
    bool hover = ui_element_hover(element, x, y);
    bool lmb_down = hover && chord_active(&CHORD_REPEAT1(RICO_SCANCODE_LMB));
    bool rmb_down = hover && chord_active(&CHORD_REPEAT1(RICO_SCANCODE_RMB));

    if ((lmb_down || rmb_down) && element->color_click.a)
    {
        element->color = element->color_click;
    }
    else if (hover && element->color_hover.a)
    {
        element->color = element->color_hover;
    }
    else
    {
        element->color = element->color_default;
    }

    if (element->event)
    {
        struct RICO_ui_event_data data = { 0 };
        data.element = element;

        // Mouse hover
        if (hover)
        {
            data.event_type = RICO_UI_EVENT_HOVER;
            element->event(&data);
        }

        // Left mouse button
        if (hover && chord_active(&CHORD1(RICO_SCANCODE_LMB)))
        {
            data.event_type = RICO_UI_EVENT_LMB_CLICK;
            element->event(&data);
        }
        if (lmb_down)
        {
            data.event_type = RICO_UI_EVENT_LMB_DOWN;
            element->event(&data);
        }
        if (hover && chord_active(&CHORD_UP1(RICO_SCANCODE_LMB)))
        {
            data.event_type = RICO_UI_EVENT_LMB_UP;
            element->event(&data);
        }

        // Middle mouse button
        if (hover && chord_active(&CHORD1(RICO_SCANCODE_MMB)))
        {
            data.event_type = RICO_UI_EVENT_MMB_CLICK;
            element->event(&data);
        }
        if (hover && chord_active(&CHORD_REPEAT1(RICO_SCANCODE_MMB)))
        {
            data.event_type = RICO_UI_EVENT_MMB_DOWN;
            element->event(&data);
        }
        if (hover && chord_active(&CHORD_UP1(RICO_SCANCODE_MMB)))
        {
            data.event_type = RICO_UI_EVENT_MMB_UP;
            element->event(&data);
        }

        // Right mouse button
        if (hover && chord_active(&CHORD1(RICO_SCANCODE_RMB)))
        {
            data.event_type = RICO_UI_EVENT_RMB_CLICK;
            element->event(&data);
        }
        if (rmb_down)
        {
            data.event_type = RICO_UI_EVENT_RMB_DOWN;
            element->event(&data);
        }
        if (hover && chord_active(&CHORD_UP1(RICO_SCANCODE_RMB)))
        {
            data.event_type = RICO_UI_EVENT_RMB_UP;
            element->event(&data);
        }
    }

    switch (element->type)
    {
        case RICO_UI_ELEMENT_HUD:
            ui_draw_hud((struct RICO_ui_hud *)element, x, y);
            break;
        case RICO_UI_ELEMENT_BUTTON:
            ui_draw_button((struct RICO_ui_button *)element, x, y);
            break;
        case RICO_UI_ELEMENT_LABEL:
            ui_draw_label((struct RICO_ui_label *)element, x, y);
            break;
        default:
            RICO_ASSERT(0); // Unhandled element type
            break;
    }

    struct RICO_ui_element *child = element->first_child;
    while (child)
    {
        ui_draw_element(child, x, y);
        child = child->next;
    }
}

extern bool RICO_ui_layout(struct RICO_ui_element *element, u32 x, u32 y,
                           u32 max_w, u32 max_h)
{
    if (!max_w)
    {
        max_w = SCREEN_WIDTH - x;
    }
    if (!max_h)
    {
        max_h = SCREEN_HEIGHT - y;
    }

    bool fit = ui_layout_element(element, 0, 0, max_w, max_h);
    return fit;
}

extern void RICO_ui_draw(struct RICO_ui_element *element, u32 x, u32 y)
{
    ui_draw_element(element, x, y);
}