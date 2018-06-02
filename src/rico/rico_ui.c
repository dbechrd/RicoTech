// TODO: Alignment
static u8 ui_stack[4098];
static u8 *ui_stack_ptr;

enum ui_rect_fill_mode
{
    UI_RECT_FILLED,
    UI_RECT_OUTLINE
};

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

static void ui_element_set_width(struct RICO_ui_element *element, s32 w)
{
    // HUD is root element; it has no parent
    if (element->type == RICO_UI_ELEMENT_HUD)
        return;

    // Already big enough to contain children
    if (w < element->location.w)
        return;

    // Resize element, and parent(s) if necessary
    element->location.w = w;
    if (element->parent)
    {
        ui_element_set_width(element->parent, element->margin.left +
                             element->location.w + element->margin.right);
    }
}

static void ui_element_set_height(struct RICO_ui_element *element, s32 h)
{
    // HUD is root element; it has no parent
    if (element->type == RICO_UI_ELEMENT_HUD)
        return;

    // Already big enough to contain children
    if (h < element->location.h)
        return;

    // Resize element, and parent(s) if necessary
    element->location.h = h;
    if (element->parent)
    {
        ui_element_set_height(element->parent, element->margin.top +
                              element->location.h + element->margin.bottom);
    }
}

static void *ui_push_element(u32 bytes, enum RICO_ui_element_type type,
                             const struct rect *location,
                             const struct rect *margin,
                             const struct rect *padding,
                             struct RICO_ui_element *parent)
{
    struct RICO_ui_element *element = ui_stack_push(bytes);
    element->type = type;
    element->location = RECT(location->x, location->y, 0, 0);
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
        }
        element->parent->last_child = element;
    }

    ui_element_set_width(element, location->w);
    ui_element_set_height(element, location->h);

    return element;
}

extern struct RICO_ui_hud *RICO_ui_push_hud(const struct rect *location,
                                            const struct rect *margin,
                                            const struct rect *padding)
{
    return ui_push_element(sizeof(struct RICO_ui_hud), RICO_UI_ELEMENT_HUD,
                           location, margin, padding, NULL);
}

extern struct RICO_ui_row *RICO_ui_push_row(struct RICO_ui_element *parent,
                                            const struct rect *location,
                                            const struct rect *margin,
                                            const struct rect *padding)
{
    return ui_push_element(sizeof(struct RICO_ui_row), RICO_UI_ELEMENT_ROW,
                           location, margin, padding, parent);
}

extern struct RICO_ui_label *RICO_ui_push_label(struct RICO_ui_element *parent,
                                                const struct rect *location,
                                                const struct rect *margin,
                                                const struct rect *padding)
{
    return ui_push_element(sizeof(struct RICO_ui_label), RICO_UI_ELEMENT_LABEL,
                           location, margin, padding, parent);
}

static void ui_draw_hud(struct RICO_ui_hud *hud)
{
    ui_draw_rect(&hud->element.location, UI_RECT_FILLED, &COLOR_RED);

    struct RICO_ui_element *child = hud->element.first_child;
    while (child)
    {
        RICO_ui_draw(child);
        child = child->next;
    }
}

static void ui_draw_row(struct RICO_ui_row *row)
{
    ui_draw_rect(&row->element.location, UI_RECT_FILLED, &COLOR_GREEN);

    struct RICO_ui_element *child = row->element.first_child;
    while (child)
    {
        RICO_ui_draw(child);
        child = child->next;
    }
}

static void ui_draw_label(struct RICO_ui_label *label)
{
    ui_draw_rect(&label->element.location, UI_RECT_FILLED, &COLOR_BLUE);

    struct RICO_ui_element *child = label->element.first_child;
    while (child)
    {
        RICO_ui_draw(child);
        child = child->next;
    }
}

extern void RICO_ui_draw(struct RICO_ui_element *element)
{
    switch (element->type)
    {
        case RICO_UI_ELEMENT_HUD:
            ui_draw_hud((struct RICO_ui_hud *)element);
            break;
        case RICO_UI_ELEMENT_ROW:
            ui_draw_row((struct RICO_ui_row *)element);
            break;
        case RICO_UI_ELEMENT_LABEL:
            ui_draw_label((struct RICO_ui_label *)element);
            break;
    }
}