#include "subghz_editor_model.h"
#include "subghz_editor_file.h"
#include "subghz_editor_view.h"

static uint32_t subghz_editor_nav_back_to_list(void* context) {
    UNUSED(context);
    return SubGhzEditorViewVariableList;
}

static uint32_t subghz_editor_nav_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

static SubGhzEditorApp* subghz_editor_app_alloc(void) {
    SubGhzEditorApp* app = malloc(sizeof(SubGhzEditorApp));
    memset(app, 0, sizeof(SubGhzEditorApp));

    subghz_editor_model_init(&app->model);

    // Open Records
    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->dialogs = furi_record_open(RECORD_DIALOGS);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // Alloc Views
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(
        app->view_dispatcher,
        app->gui,
        ViewDispatcherTypeFullscreen);

    app->var_list = variable_item_list_alloc();
    View* var_list_view = variable_item_list_get_view(app->var_list);
    view_set_previous_callback(var_list_view, subghz_editor_nav_exit);
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubGhzEditorViewVariableList,
        var_list_view);

    app->text_input = text_input_alloc();
    View* text_input_view = text_input_get_view(app->text_input);
    view_set_previous_callback(text_input_view, subghz_editor_nav_back_to_list);
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubGhzEditorViewTextInput,
        text_input_view);

    app->widget = widget_alloc();
    View* widget_view = widget_get_view(app->widget);
    view_set_previous_callback(widget_view, subghz_editor_nav_back_to_list);
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubGhzEditorViewWidget,
        widget_view);

    subghz_editor_view_init(app);

    return app;
}

static void subghz_editor_app_free(SubGhzEditorApp* app) {
    // Remove views
    view_dispatcher_remove_view(app->view_dispatcher, SubGhzEditorViewVariableList);
    view_dispatcher_remove_view(app->view_dispatcher, SubGhzEditorViewTextInput);
    view_dispatcher_remove_view(app->view_dispatcher, SubGhzEditorViewWidget);

    variable_item_list_free(app->var_list);
    text_input_free(app->text_input);
    widget_free(app->widget);
    view_dispatcher_free(app->view_dispatcher);

    // Close Records
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_DIALOGS);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);

    subghz_editor_model_free(&app->model);
    free(app);
}

int32_t subghz_editor_app(void* p) {
    UNUSED(p);
    SubGhzEditorApp* app = subghz_editor_app_alloc();

    // Check if path passed as argument (e.g. opened from Archive / File Manager)
    if(p && strlen((const char*)p) > 0) {
        subghz_editor_file_load(app, (const char*)p);
        subghz_editor_view_refresh_list(app);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzEditorViewVariableList);
    view_dispatcher_run(app->view_dispatcher);

    subghz_editor_app_free(app);
    return 0;
}
