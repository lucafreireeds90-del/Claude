#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdio.h>

#define PASSWORD_LEN 12
#define MENU_ITEMS   3

typedef enum {
    ScreenMenu,
    ScreenCounter,
    ScreenPassword,
    ScreenAbout,
} Screen;

typedef struct {
    Screen screen;
    uint8_t menu_index;
    int32_t counter;
    char password[PASSWORD_LEN + 1];
    bool running;
} AppState;

typedef struct {
    AppState state;
    FuriMutex* mutex;
    FuriMessageQueue* queue;
    Gui* gui;
    ViewPort* view_port;
} App;

static const char* menu_labels[MENU_ITEMS] = {"Contador", "Gerador de senha", "Sobre"};

static void generate_password(char* out) {
    static const char charset[] =
        "abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789!@#$%&*?";
    const uint32_t n = sizeof(charset) - 1;
    for(uint8_t i = 0; i < PASSWORD_LEN; i++) {
        out[i] = charset[furi_hal_random_get() % n];
    }
    out[PASSWORD_LEN] = '\0';
}

static void draw_callback(Canvas* canvas, void* ctx) {
    App* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);
    AppState* s = &app->state;

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    switch(s->screen) {
    case ScreenMenu:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 2, AlignCenter, AlignTop, "CYBER DEATH");
        canvas_set_font(canvas, FontSecondary);
        for(uint8_t i = 0; i < MENU_ITEMS; i++) {
            uint8_t y = 20 + i * 13;
            if(i == s->menu_index) {
                canvas_draw_box(canvas, 4, y - 1, 120, 12);
                canvas_set_color(canvas, ColorWhite);
            }
            canvas_draw_str(canvas, 10, y + 8, menu_labels[i]);
            canvas_set_color(canvas, ColorBlack);
        }
        break;

    case ScreenCounter: {
        char buf[24];
        snprintf(buf, sizeof(buf), "%ld", (long)s->counter);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 4, AlignCenter, AlignTop, "Contador");
        canvas_set_font(canvas, FontBigNumbers);
        canvas_draw_str_aligned(canvas, 64, 28, AlignCenter, AlignCenter, buf);
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(
            canvas, 64, 60, AlignCenter, AlignBottom, "Cima/Baixo +-  OK zera  Voltar");
        break;
    }

    case ScreenPassword:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 4, AlignCenter, AlignTop, "Senha");
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(
            canvas, 64, 30, AlignCenter, AlignCenter, s->password[0] ? s->password : "---");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 60, AlignCenter, AlignBottom, "OK gera nova  Voltar");
        break;

    case ScreenAbout:
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str_aligned(canvas, 64, 4, AlignCenter, AlignTop, "Cyber Death v0.1");
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str_aligned(canvas, 64, 24, AlignCenter, AlignTop, "App base para Flipper");
        canvas_draw_str_aligned(canvas, 64, 36, AlignCenter, AlignTop, "Adicione seus modulos!");
        canvas_draw_str_aligned(canvas, 64, 60, AlignCenter, AlignBottom, "Voltar");
        break;
    }

    furi_mutex_release(app->mutex);
}

static void input_callback(InputEvent* event, void* ctx) {
    App* app = ctx;
    furi_message_queue_put(app->queue, event, FuriWaitForever);
}

static void handle_input(App* app, InputEvent* e) {
    AppState* s = &app->state;
    if(e->type != InputTypeShort && e->type != InputTypeRepeat) return;

    switch(s->screen) {
    case ScreenMenu:
        if(e->key == InputKeyUp) {
            s->menu_index = (s->menu_index + MENU_ITEMS - 1) % MENU_ITEMS;
        } else if(e->key == InputKeyDown) {
            s->menu_index = (s->menu_index + 1) % MENU_ITEMS;
        } else if(e->key == InputKeyOk) {
            s->screen = (Screen)(ScreenCounter + s->menu_index);
            if(s->screen == ScreenPassword) generate_password(s->password);
        } else if(e->key == InputKeyBack) {
            s->running = false;
        }
        break;

    case ScreenCounter:
        if(e->key == InputKeyUp) s->counter++;
        else if(e->key == InputKeyDown) s->counter--;
        else if(e->key == InputKeyOk) s->counter = 0;
        else if(e->key == InputKeyBack) s->screen = ScreenMenu;
        break;

    case ScreenPassword:
        if(e->key == InputKeyOk) generate_password(s->password);
        else if(e->key == InputKeyBack) s->screen = ScreenMenu;
        break;

    case ScreenAbout:
        if(e->key == InputKeyBack) s->screen = ScreenMenu;
        break;
    }
}

int32_t cyber_death_app(void* p) {
    UNUSED(p);

    App* app = malloc(sizeof(App));
    memset(app, 0, sizeof(App));
    app->state.running = true;
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    app->view_port = view_port_alloc();
    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);

    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    InputEvent event;
    while(app->state.running) {
        if(furi_message_queue_get(app->queue, &event, 100) == FuriStatusOk) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            handle_input(app, &event);
            furi_mutex_release(app->mutex);
            view_port_update(app->view_port);
        }
    }

    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(app->view_port);
    furi_message_queue_free(app->queue);
    furi_mutex_free(app->mutex);
    free(app);
    return 0;
}
