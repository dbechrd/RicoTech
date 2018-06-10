// TODO: Alignment
static u8 ui_stack[KB(32)];
static u8 *ui_stack_ptr;
static u32 ui_stack_bytes_used;

enum ui_rect_fill_mode
{
    UI_RECT_FILLED,
    UI_RECT_OUTLINE
};

static void ui_draw_element(struct RICO_ui_element *element, u32 x, u32 y);

static void rico_ui_init()
{
    ui_stack_ptr = ui_stack;
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
        RICO_prim_draw_line2d(sx    , sy    , sx + sw,  sy    , color);
        RICO_prim_draw_line2d(sx + sw, sy    , sx + sw,  sy + sh, color);
        RICO_prim_draw_line2d(sx + sw, sy + sh, sx    ,  sy + sh, color);
        RICO_prim_draw_line2d(sx    , sy + sh, sx    ,  sy    , color);
    }
    else
    {
        RICO_prim_draw_rect(&RECT(rect->x + x, rect->y + y, rect->w, rect->h),
                            color);
    }
}

static void *ui_push_element(u32 bytes, enum RICO_ui_element_type type,
                             const struct vec2i *min_size,
                             const struct rect *margin,
                             const struct rect *padding,
                             struct RICO_ui_element *parent)
{
    struct RICO_ui_element *element = ui_stack_push(bytes);
    element->type = type;
    element->min_size = *min_size;
    element->margin = *margin;
    element->padding = *padding;
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

extern struct RICO_ui_hud *RICO_ui_push_hud(const struct vec2i *min_size,
                                            const struct rect *margin,
                                            const struct rect *padding)
{
    return ui_push_element(sizeof(struct RICO_ui_hud), RICO_UI_ELEMENT_HUD,
                           min_size, margin, padding, NULL);
}

extern struct RICO_ui_row *RICO_ui_push_row(struct RICO_ui_element *parent,
                                            const struct vec2i *min_size,
                                            const struct rect *margin,
                                            const struct rect *padding)
{
    return ui_push_element(sizeof(struct RICO_ui_row), RICO_UI_ELEMENT_ROW,
                           min_size, margin, padding, parent);
}

extern struct RICO_ui_label *RICO_ui_push_label(struct RICO_ui_element *parent,
                                                const struct vec2i *min_size,
                                                const struct rect *margin,
                                                const struct rect *padding)
{
    return ui_push_element(sizeof(struct RICO_ui_label), RICO_UI_ELEMENT_LABEL,
                           min_size, margin, padding, parent);
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

static inline bool ui_element_hover(struct RICO_ui_element *element)
{
    return rect_intersects(&element->bounds, mouse_x, mouse_y);
}

static inline bool ui_element_left_click(struct RICO_ui_element *element)
{
    bool left_click = rect_intersects(&element->bounds, mouse_x, mouse_y) &&
                      chord_active(&CHORD_REPEAT1(RICO_SCANCODE_LMB));
    return left_click;
}

static inline bool ui_element_right_click(struct RICO_ui_element *element)
{
    bool left_click = rect_intersects(&element->bounds, mouse_x, mouse_y) &&
        chord_active(&CHORD_REPEAT1(RICO_SCANCODE_RMB));
    return left_click;
}

static void ui_draw_hud(struct RICO_ui_hud *ctrl, u32 x, u32 y)
{
    const struct vec4 *color = ui_element_hover(&ctrl->element)
        ? &COLOR_RED
        : &COLOR_DARK_RED;
    ui_draw_rect(x, y, &ctrl->element.bounds, UI_RECT_FILLED, color);
}

static void ui_draw_row(struct RICO_ui_row *ctrl, u32 x, u32 y)
{
    const struct vec4 *color = ui_element_hover(&ctrl->element)
        ? &COLOR_GREEN
        : &COLOR_DARK_GREEN;
    ui_draw_rect(x, y, &ctrl->element.bounds, UI_RECT_FILLED, color);
}

static void ui_draw_label(struct RICO_ui_label *ctrl, u32 x, u32 y)
{
    const struct vec4 *color = ui_element_hover(&ctrl->element)
        ? &COLOR_YELLOW
        : &COLOR_DARK_YELLOW;
    if (ui_element_left_click(&ctrl->element))
    {
        color = &COLOR_DODGER;
    }
    ui_draw_rect(x, y, &ctrl->element.bounds, UI_RECT_FILLED, color);
}

static void ui_draw_element(struct RICO_ui_element *element, u32 x, u32 y)
{
    switch (element->type)
    {
        case RICO_UI_ELEMENT_HUD:
            ui_draw_hud((struct RICO_ui_hud *)element, x, y);
            break;
        case RICO_UI_ELEMENT_ROW:
            ui_draw_row((struct RICO_ui_row *)element, x, y);
            break;
        case RICO_UI_ELEMENT_LABEL:
            ui_draw_label((struct RICO_ui_label *)element, x, y);
            break;
    }

    struct RICO_ui_element *child = element->first_child;
    while (child)
    {
        ui_draw_element(child, x, y);
        child = child->next;
    }
}

extern bool RICO_ui_draw(struct RICO_ui_element *element, u32 x, u32 y,
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

    bool fit = ui_layout_element(element, x, y, max_w, max_h);
    if (fit)
    {
        ui_draw_element(element, 0, 0);
    }
    return fit;
}