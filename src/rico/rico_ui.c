// TODO: Alignment
static u8 ui_stack[KB(32)];
static u8 *ui_stack_ptr;
static u32 ui_stack_bytes_used;

enum ui_rect_fill_mode
{
    UI_RECT_FILLED,
    UI_RECT_OUTLINE
};

static void ui_draw_element(struct RICO_ui_element *element, s32 x, s32 y);

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

static void ui_draw_rect(const struct rect *rect,
                         enum ui_rect_fill_mode fill_mode,
                         const struct vec4 *color)
{
    float x = SCREEN_X(rect->x);
    float y = SCREEN_Y(rect->y);
    float w = SCREEN_W(rect->w);
    float h = SCREEN_H(rect->h);

    // HACK: Override filled mode max_w/ outline
    // TODO: Draw filled rectangles
    if (fill_mode == UI_RECT_FILLED)
        fill_mode = UI_RECT_OUTLINE;

    if (fill_mode == UI_RECT_OUTLINE)
    {
        RICO_prim_draw_line2d(x    , y    , x + w,  y    , color);
        RICO_prim_draw_line2d(x + w, y    , x + w,  y + h, color);
        RICO_prim_draw_line2d(x + w, y + h, x    ,  y + h, color);
        RICO_prim_draw_line2d(x    , y + h, x    ,  y    , color);
    }
    else
    {
        RICO_ASSERT(0); // Unhandled fill_mode
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

static bool ui_layout_element(struct RICO_ui_element *element, s32 x, s32 y,
                              s32 max_w, s32 max_h)
{
    s32 margin_w = element->margin.left + element->margin.right;
    s32 margin_h = element->margin.top + element->margin.bottom;

    s32 pad_w = element->padding.left + element->padding.right;
    s32 pad_h = element->padding.top + element->padding.bottom;

    struct rect *rect = &element->rect;
    rect->x = x;
    rect->y = y;

    rect->w = margin_w + MAX(element->min_size.w, pad_w);
    rect->h = margin_h + MAX(element->min_size.h, pad_h);

    s32 start_x = rect->x + element->margin.left + element->padding.left;
    s32 start_y = rect->y + element->margin.top + element->padding.top;
    s32 start_w = margin_w + pad_w;
    s32 start_h = margin_h + pad_h;

    ////////////////////////////////////////////////////////////////////////////
    
    s32 next_x = start_x;
    s32 next_y = start_y;

    // TODO: Use rect->w and rect->h directly?
    s32 max_row_w = start_w;
    s32 used_h = start_h;

    s32 row_used_w = start_w;
    s32 row_max_h = 0;

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
                next_x += child->rect.w;

                // Track max row width
                row_used_w += child->rect.w;
                if (row_used_w > max_row_w)
                {
                    max_row_w = row_used_w;
                }

                // Track height of tallest child element in this row
                if (child->rect.h > row_max_h)
                {
                    row_max_h = child->rect.h;
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

    if (max_row_w > rect->w)
    {
        rect->w = max_row_w;
    }
    if (used_h > rect->h)
    {
        rect->h = used_h;
    }

    // Expand region to fit children, then enforce maximum min_size
    //if (rect->w > max_w) rect->w = MAX(max_w, 0);
    //if (rect->h > max_h) rect->h = MAX(max_h, 0);
    //if (min_w > rect->w) rect->w = min_w;
    //if (min_h > rect->h) rect->h = min_h;

    return (rect->w <= max_w && rect->h <= max_h);
}

static void ui_draw_hud(struct RICO_ui_hud *ctrl, s32 x, s32 y)
{
    struct rect rect = ctrl->element.rect;
    rect.x += x + ctrl->element.margin.left;
    rect.y += y + ctrl->element.margin.top;
    rect.w -= (ctrl->element.margin.left + ctrl->element.margin.right);
    rect.h -= (ctrl->element.margin.top + ctrl->element.margin.bottom);
    ui_draw_rect(&rect, UI_RECT_FILLED, &COLOR_DARK_RED);

    struct RICO_ui_element *child = ctrl->element.first_child;
    while (child)
    {
        ui_draw_element(child, x, y);
        child = child->next;
    }
}

static void ui_draw_row(struct RICO_ui_row *ctrl, s32 x, s32 y)
{
    struct rect rect = ctrl->element.rect;
    rect.x += x + ctrl->element.margin.left;
    rect.y += y + ctrl->element.margin.top;
    rect.w -= (ctrl->element.margin.left + ctrl->element.margin.right);
    rect.h -= (ctrl->element.margin.top + ctrl->element.margin.bottom);
    ui_draw_rect(&rect, UI_RECT_FILLED, &COLOR_DARK_GREEN);

    struct RICO_ui_element *child = ctrl->element.first_child;
    while (child)
    {
        ui_draw_element(child, x, y);
        child = child->next;
    }
}

static void ui_draw_label(struct RICO_ui_label *ctrl, s32 x, s32 y)
{
    struct rect rect = ctrl->element.rect;
    rect.x += x + ctrl->element.margin.left;
    rect.y += y + ctrl->element.margin.top;
    rect.w -= (ctrl->element.margin.left + ctrl->element.margin.right);
    rect.h -= (ctrl->element.margin.top + ctrl->element.margin.bottom);
    ui_draw_rect(&rect, UI_RECT_FILLED, &COLOR_DARK_YELLOW);

    struct RICO_ui_element *child = ctrl->element.first_child;
    while (child)
    {
        ui_draw_element(child, x, y);
        child = child->next;
    }
}

static void ui_draw_element(struct RICO_ui_element *element, s32 x, s32 y)
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
}

extern bool RICO_ui_draw(struct RICO_ui_element *element, s32 x, s32 y,
                         s32 max_w, s32 max_h)
{
    bool fit = ui_layout_element(element, x, y, max_w, max_h);
    if (fit)
    {
        ui_draw_element(element, x, y);
    }
    return fit;
}