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
                             const struct vec2i *size,
                             const struct rect *margin,
                             const struct rect *padding,
                             struct RICO_ui_element *parent)
{
    struct RICO_ui_element *element = ui_stack_push(bytes);
    element->type = type;
    element->size = *size;
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

extern struct RICO_ui_hud *RICO_ui_push_hud(const struct vec2i *size,
                                            const struct rect *margin,
                                            const struct rect *padding)
{
    return ui_push_element(sizeof(struct RICO_ui_hud), RICO_UI_ELEMENT_HUD,
                           size, margin, padding, NULL);
}

extern struct RICO_ui_row *RICO_ui_push_row(struct RICO_ui_element *parent,
                                            const struct vec2i *size,
                                            const struct rect *margin,
                                            const struct rect *padding)
{
    return ui_push_element(sizeof(struct RICO_ui_row), RICO_UI_ELEMENT_ROW,
                           size, margin, padding, parent);
}

extern struct RICO_ui_label *RICO_ui_push_label(struct RICO_ui_element *parent,
                                                const struct vec2i *size,
                                                const struct rect *margin,
                                                const struct rect *padding)
{
    return ui_push_element(sizeof(struct RICO_ui_label), RICO_UI_ELEMENT_LABEL,
                           size, margin, padding, parent);
}

static void ui_layout_element(struct RICO_ui_element *element, s32 x, s32 y,
                              s32 max_w, s32 max_h)
{
    s32 margin_w = element->margin.left + element->margin.right;
    s32 margin_h = element->margin.top + element->margin.bottom;

    s32 pad_w = element->padding.left + element->padding.right;
    s32 pad_h = element->padding.top + element->padding.bottom;

    struct rect *rect = &element->rect;
    rect->x = x;// + element->margin.left;
    rect->y = y;// + element->margin.top;
    rect->w = margin_w + (element->size.w >= pad_w ? element->size.w : pad_w);
    rect->h = margin_h + (element->size.h >= pad_h ? element->size.h : pad_h);

    s32 min_w = rect->w;
    s32 min_h = rect->h;

    struct RICO_ui_element *child = element->first_child;
    if (child)
    {
        s32 start_x = rect->x + element->margin.left + element->padding.left;
        s32 start_y = rect->y + element->margin.top + element->padding.top;

        s32 max_child_w = MAX(max_w - pad_w, 0);
        s32 max_child_h = MAX(max_h - pad_h, 0);

        s32 current_x = start_x;
        s32 current_y = start_y;
        s32 current_w = rect->w;
        s32 current_h = rect->h;

        while (child)
        {
            // Calculate child bounds
            ui_layout_element(child, current_x, current_y, max_child_w,
                              max_child_h);

            // Ran out of width, wrap layout for this element
            bool wrap = current_w + child->rect.w > max_w;
            if (!wrap)
            {
                current_x += child->rect.w;
                current_w += child->rect.w;

                if (current_w > min_w)
                    min_w = current_w;

                if (child->rect.h > current_h)
                    current_h = child->rect.h;
            }
            
            if (wrap)
            {
                // Ensure every row has some width/height > 0
                //RICO_ASSERT(current_w);
                //RICO_ASSERT(current_h);
                if (!current_w || !current_h)
                {
                    break;
                    //continue;
                }

                // Ran out of height, abort layout for this element
                if (min_h + current_h > max_h)
                    break;

                // Wrap to next row
                min_h += current_h;
                current_x = start_x;
                current_y += current_h;
                current_w = rect->w;
                current_h = rect->h;
            }

            //if (current_h > min_h)
            //    min_h = current_h;

            if (!wrap)
            {
                child = child->next;
            }
        }

        min_h += current_h;
    }

    //min_w += rect->w + pad_w;
    //min_h += rect->w + pad_h;
    //min_w += pad_w;
    //min_h += pad_h;

    // Expand region to fit children, then enforce maximum size
    //if (rect->w > max_w) rect->w = MAX(max_w, 0);
    //if (rect->h > max_h) rect->h = MAX(max_h, 0);
    if (min_w > rect->w) rect->w = min_w;
    if (min_h > rect->h) rect->h = min_h;
}

static void ui_draw_hud(struct RICO_ui_hud *hud, s32 x, s32 y)
{
    struct rect rect = hud->element.rect;
    rect.x += x + hud->element.margin.left;
    rect.y += y + hud->element.margin.top;
    ui_draw_rect(&rect, UI_RECT_FILLED, &COLOR_DARK_RED);

    struct RICO_ui_element *child = hud->element.first_child;
    while (child)
    {
        ui_draw_element(child, x, y);
        child = child->next;
    }
}

static void ui_draw_row(struct RICO_ui_row *row, s32 x, s32 y)
{
    struct rect rect = row->element.rect;
    rect.x += x + row->element.margin.left;
    rect.y += y + row->element.margin.top;
    ui_draw_rect(&rect, UI_RECT_FILLED, &COLOR_DARK_GREEN);

    struct RICO_ui_element *child = row->element.first_child;
    while (child)
    {
        ui_draw_element(child, x, y);
        child = child->next;
    }
}

static void ui_draw_label(struct RICO_ui_label *label, s32 x, s32 y)
{
    struct rect rect = label->element.rect;
    rect.x += x + label->element.margin.left;
    rect.y += y + label->element.margin.top;
    ui_draw_rect(&rect, UI_RECT_FILLED, &COLOR_DARK_YELLOW);

    struct RICO_ui_element *child = label->element.first_child;
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

extern void RICO_ui_draw(struct RICO_ui_element *element, s32 x, s32 y,
                         s32 max_w, s32 max_h)
{
    ui_layout_element(element, x, y, max_w, max_h);
    ui_draw_element(element, x, y);
}