#include "subghz_editor_view.h"
#include "subghz_editor_file.h"

static void subghz_editor_freq_change(VariableItem* item) {
    SubGhzEditorApp* app = (SubGhzEditorApp*)variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    if(app->model.is_custom_freq) {
        if(index == 0) {
            variable_item_set_current_value_text(item, app->model.custom_freq_str);
            app->model.frequency = app->model.initial_frequency;
        } else {
            uint8_t p_idx = index - 1;
            variable_item_set_current_value_text(item, subghz_presets[p_idx].str);
            app->model.frequency = subghz_presets[p_idx].frequency;
        }
    } else {
        variable_item_set_current_value_text(item, subghz_presets[index].str);
        app->model.frequency = subghz_presets[index].frequency;
    }

    app->model.current_freq_index = index;
    app->model.modified = true;
}

static void subghz_editor_text_input_done(void* context) {
    SubGhzEditorApp* app = (SubGhzEditorApp*)context;
    snprintf(app->model.key_buf, sizeof(app->model.key_buf), "%s", app->model.temp_key_buf);
    app->model.modified = true;

    subghz_editor_view_refresh_list(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzEditorViewVariableList);
}

static void subghz_editor_list_enter(void* context, uint32_t index) {
    SubGhzEditorApp* app = (SubGhzEditorApp*)context;

    if(!app->model.is_file_loaded) {
        if(index == 0) {
            // Open File dialog
            DialogsFileBrowserOptions browser_options;
            dialog_file_browser_set_basic_options(
                &browser_options,
                SUBGHZ_EDITOR_APP_EXTENSION,
                NULL);
            browser_options.base_path = SUBGHZ_EDITOR_APP_PATH_FOLDER;
            browser_options.hide_ext = false;

            bool chosen = dialog_file_browser_show(
                app->dialogs,
                app->model.file_path,
                app->model.file_path,
                &browser_options);

            if(chosen) {
                if(subghz_editor_file_load(app, furi_string_get_cstr(app->model.file_path))) {
                    notification_message(app->notifications, &sequence_success);
                } else {
                    subghz_editor_show_message(app, "Error", "Failed to load .sub file");
                    return;
                }
            }
            subghz_editor_view_refresh_list(app);
        } else if(index == 1) {
            // About
            subghz_editor_show_message(
                app,
                "Sub-GHz Editor v1.0",
                "1. Select saved .sub\n2. Change frequency preset\n3. Edit Key in HEX\n4. Save copy or overwrite\n\nBy Jarvis");
        }
        return;
    }

    // File is loaded
    // Layout:
    // 0: File [Open another]
    // 1: Frequency [Preset selector]
    // 2 (if key): Key [Edit]
    // 3: Protocol
    // 4: Save (Overwrite)
    // 5: Save As Copy
    // 6: Close File

    uint32_t current_idx = 0;

    // Item 0: File
    if(index == current_idx) {
        DialogsFileBrowserOptions browser_options;
        dialog_file_browser_set_basic_options(
            &browser_options,
            SUBGHZ_EDITOR_APP_EXTENSION,
            NULL);
        browser_options.base_path = SUBGHZ_EDITOR_APP_PATH_FOLDER;
        browser_options.hide_ext = false;

        bool chosen = dialog_file_browser_show(
            app->dialogs,
            app->model.file_path,
            app->model.file_path,
            &browser_options);

        if(chosen) {
            subghz_editor_file_load(app, furi_string_get_cstr(app->model.file_path));
        }
        subghz_editor_view_refresh_list(app);
        return;
    }
    current_idx++;

    // Item 1: Frequency (enter does nothing or shows preset info)
    if(index == current_idx) {
        return;
    }
    current_idx++;

    // Item 2: Key (if available)
    if(app->model.has_key) {
        if(index == current_idx) {
            // Switch to TextInput
            snprintf(app->model.temp_key_buf, sizeof(app->model.temp_key_buf), "%s", app->model.key_buf);
            text_input_reset(app->text_input);
            text_input_set_header_text(app->text_input, "Edit Key (HEX):");
            text_input_set_result_callback(
                app->text_input,
                subghz_editor_text_input_done,
                app,
                app->model.temp_key_buf,
                sizeof(app->model.temp_key_buf),
                false);
            view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzEditorViewTextInput);
            return;
        }
        current_idx++;
    }

    // Protocol info item (read only)
    current_idx++;

    // Save Overwrite
    if(index == current_idx) {
        if(subghz_editor_file_save(app, furi_string_get_cstr(app->model.file_path))) {
            notification_message(app->notifications, &sequence_success);
            subghz_editor_show_message(app, "Success", "File updated\nsuccessfully!");
        } else {
            subghz_editor_show_message(app, "Error", "Failed to save file");
        }
        return;
    }
    current_idx++;

    // Save As Copy
    if(index == current_idx) {
        if(subghz_editor_file_save_copy(app)) {
            notification_message(app->notifications, &sequence_success);
            subghz_editor_show_message(app, "Success", "Saved as copy with\n_edit.sub suffix!");
        } else {
            subghz_editor_show_message(app, "Error", "Failed to save copy");
        }
        return;
    }
    current_idx++;

    // Close File
    if(index == current_idx) {
        subghz_editor_model_reset(&app->model);
        subghz_editor_view_refresh_list(app);
        return;
    }
}

void subghz_editor_show_message(SubGhzEditorApp* app, const char* title, const char* msg) {
    widget_reset(app->widget);
    widget_add_string_element(
        app->widget,
        64,
        8,
        AlignCenter,
        AlignTop,
        FontPrimary,
        title);
    widget_add_string_multiline_element(
        app->widget,
        64,
        24,
        AlignCenter,
        AlignTop,
        FontSecondary,
        msg);
    view_dispatcher_switch_to_view(app->view_dispatcher, SubGhzEditorViewWidget);
}

void subghz_editor_view_refresh_list(SubGhzEditorApp* app) {
    variable_item_list_reset(app->var_list);

    if(!app->model.is_file_loaded) {
        variable_item_list_add(app->var_list, "Open .sub File", 1, NULL, app);
        variable_item_list_add(app->var_list, "Help / About", 1, NULL, app);
        return;
    }

    // 0: File
    VariableItem* file_item = variable_item_list_add(
        app->var_list,
        "File:",
        1,
        NULL,
        app);
    variable_item_set_current_value_text(
        file_item,
        furi_string_get_cstr(app->model.file_name));

    // 1: Frequency presets
    uint8_t values_count = SUBGHZ_PRESETS_COUNT;
    if(app->model.is_custom_freq) {
        values_count = SUBGHZ_PRESETS_COUNT + 1;
    }
    VariableItem* freq_item = variable_item_list_add(
        app->var_list,
        "Freq:",
        values_count,
        subghz_editor_freq_change,
        app);

    variable_item_set_current_value_index(freq_item, app->model.current_freq_index);
    if(app->model.is_custom_freq && app->model.current_freq_index == 0) {
        variable_item_set_current_value_text(freq_item, app->model.custom_freq_str);
    } else {
        uint8_t p_idx = app->model.is_custom_freq ? (app->model.current_freq_index - 1) : app->model.current_freq_index;
        variable_item_set_current_value_text(freq_item, subghz_presets[p_idx].str);
    }

    // 2: Key (if available)
    if(app->model.has_key) {
        VariableItem* key_item = variable_item_list_add(
            app->var_list,
            "Key:",
            1,
            NULL,
            app);
        // Truncate preview if long
        char key_preview[20];
        snprintf(key_preview, sizeof(key_preview), "%.16s", app->model.key_buf);
        variable_item_set_current_value_text(key_item, key_preview);
    }

    // 3: Protocol info
    VariableItem* proto_item = variable_item_list_add(
        app->var_list,
        "Proto:",
        1,
        NULL,
        app);
    variable_item_set_current_value_text(
        proto_item,
        furi_string_get_cstr(app->model.protocol));

    // 4: Save Overwrite
    variable_item_list_add(app->var_list, "Save (Overwrite)", 1, NULL, app);

    // 5: Save As Copy
    variable_item_list_add(app->var_list, "Save As Copy", 1, NULL, app);

    // 6: Close File
    variable_item_list_add(app->var_list, "Close File", 1, NULL, app);
}

void subghz_editor_view_init(SubGhzEditorApp* app) {
    variable_item_list_set_enter_callback(
        app->var_list,
        subghz_editor_list_enter,
        app);
    subghz_editor_view_refresh_list(app);
}
