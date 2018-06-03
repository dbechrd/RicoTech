// TODO: Alignment
static u8 ui_stack[4098];
static u8 *ui_stack_ptr;

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
    void *ptr = ui_stack_ptr;
    ui_stack_ptr += bytes;
    memset(ptr, 0, bytes);
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

    // HACK: Override filled mode w/ outline
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

static void ui_layout_element(struct RICO_ui_element *element, s32 x, s32 y)
{
    struct rect *rect = &element->rect;
    rect->x = x + element->margin.left;
    rect->y = y + element->margin.top;
    rect->w = element->size.w;
    rect->h = element->size.h;

    s32 min_width = 0;
    s32 min_height = 0;

    struct RICO_ui_element *child = element->first_child;
    if (child)
    {
        s32 current_x = rect->x + element->padding.left;
        s32 current_y = rect->y + element->padding.top;
        s32 child_w;
        s32 child_h;

        while (child)
        {
            // Calculate child bounds
            ui_layout_element(child, current_x, current_y);

            child_w = child->margin.left + child->rect.w + child->margin.right;
            child_h = child->margin.top + child->rect.h + child->margin.bottom;
        
            current_x += child_w;
            min_width += child_w;
        
            // TODO: Wrapping (check max_width, increment y by margins+height)
            //current_y += child_h;
            //min_height += child_h;

            if (child_h > min_height)
                min_height = child_h;

            child = child->next;
        }
    }

    min_width += element->padding.left + element->padding.right;
    min_height += element->padding.top + element->padding.bottom;

    // Expand region by child's margins
    if (min_width > rect->w)
    {
        rect->w = min_width;
    }
    if (min_height > rect->h)
    {
        rect->h = min_height;
    }
}

static void ui_draw_hud(struct RICO_ui_hud *hud, s32 x, s32 y)
{
    struct rect rect = hud->element.rect;
    rect.x += x;
    rect.y += y;
    ui_draw_rect(&rect, UI_RECT_FILLED, &COLOR_DARK_RED);

    struct RICO_ui_element *child = hud->element.first_child;
    while (child)
    {
        ui_draw_element(child, rect.x, rect.y);
        child = child->next;
    }
}

static void ui_draw_row(struct RICO_ui_row *row, s32 x, s32 y)
{
    struct rect rect = row->element.rect;
    rect.x += x;
    rect.y += y;
    ui_draw_rect(&row->element.rect, UI_RECT_FILLED, &COLOR_DARK_GREEN);

    struct RICO_ui_element *child = row->element.first_child;
    while (child)
    {
        ui_draw_element(child, rect.x, rect.y);
        child = child->next;
    }
}

static void ui_draw_label(struct RICO_ui_label *label, s32 x, s32 y)
{
    struct rect rect = label->element.rect;
    rect.x += x;
    rect.y += y;
    ui_draw_rect(&label->element.rect, UI_RECT_FILLED, &COLOR_DARK_YELLOW);

    struct RICO_ui_element *child = label->element.first_child;
    while (child)
    {
        ui_draw_element(child, rect.x, rect.y);
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

extern void RICO_ui_draw(struct RICO_ui_element *element, s32 x, s32 y)
{
    ui_layout_element(element, x, y);
    ui_draw_element(element, x, y);
}