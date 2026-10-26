#include "subghz_editor_file.h"

void subghz_editor_model_init(SubGhzEditorModel* model) {
    model->file_path = furi_string_alloc();
    model->file_name = furi_string_alloc();
    model->protocol = furi_string_alloc();
    model->preset_str = furi_string_alloc();
    subghz_editor_model_reset(model);
}

void subghz_editor_model_reset(SubGhzEditorModel* model) {
    furi_string_set_str(model->file_path, SUBGHZ_EDITOR_APP_PATH_FOLDER);
    furi_string_reset(model->file_name);
    furi_string_reset(model->protocol);
    furi_string_reset(model->preset_str);
    memset(model->key_buf, 0, sizeof(model->key_buf));
    memset(model->temp_key_buf, 0, sizeof(model->temp_key_buf));
    memset(model->custom_freq_str, 0, sizeof(model->custom_freq_str));
    model->frequency = 433920000;
    model->initial_frequency = 433920000;
    model->current_freq_index = 6; // default 433.92
    model->is_custom_freq = false;
    model->is_file_loaded = false;
    model->has_key = false;
    model->has_frequency = false;
    model->modified = false;
}

void subghz_editor_model_free(SubGhzEditorModel* model) {
    furi_string_free(model->file_path);
    furi_string_free(model->file_name);
    furi_string_free(model->protocol);
    furi_string_free(model->preset_str);
}

bool subghz_editor_file_load(SubGhzEditorApp* app, const char* path) {
    if(!path || !app) return false;

    FlipperFormat* fff = flipper_format_file_alloc(app->storage);
    bool success = false;

    do {
        if(!flipper_format_file_open_existing(fff, path)) {
            FURI_LOG_E("SubGhzEditor", "Failed to open file: %s", path);
            break;
        }

        FuriString* header_type = furi_string_alloc();
        uint32_t version = 0;
        if(!flipper_format_read_header(fff, header_type, &version)) {
            FURI_LOG_E("SubGhzEditor", "Invalid header in file");
            furi_string_free(header_type);
            break;
        }
        furi_string_free(header_type);

        // Reset previous loaded state
        subghz_editor_model_reset(&app->model);
        furi_string_set_str(app->model.file_path, path);
        path_extract_filename(app->model.file_path, app->model.file_name, false);

        // Read Protocol
        flipper_format_rewind(fff);
        if(flipper_format_read_string(fff, "Protocol", app->model.protocol)) {
            // Protocol read successfully
        } else {
            furi_string_set_str(app->model.protocol, "Unknown");
        }

        // Read Preset
        flipper_format_rewind(fff);
        flipper_format_read_string(fff, "Preset", app->model.preset_str);

        // Read Frequency
        flipper_format_rewind(fff);
        if(flipper_format_read_uint32(fff, "Frequency", &app->model.frequency, 1)) {
            app->model.has_frequency = true;
            app->model.initial_frequency = app->model.frequency;

            // Match frequency against presets
            bool match = false;
            for(size_t i = 0; i < SUBGHZ_PRESETS_COUNT; i++) {
                if(subghz_presets[i].frequency == app->model.frequency) {
                    app->model.current_freq_index = (uint8_t)i;
                    app->model.is_custom_freq = false;
                    match = true;
                    break;
                }
            }
            if(!match) {
                app->model.is_custom_freq = true;
                snprintf(
                    app->model.custom_freq_str,
                    sizeof(app->model.custom_freq_str),
                    "%lu.%02lu MHz",
                    (unsigned long)(app->model.frequency / 1000000),
                    (unsigned long)((app->model.frequency % 1000000) / 10000));
            }
        }

        // Read Key
        flipper_format_rewind(fff);
        FuriString* key_temp = furi_string_alloc();
        if(flipper_format_read_string(fff, "Key", key_temp)) {
            const char* key_cstr = furi_string_get_cstr(key_temp);
            if(strlen(key_cstr) > 0) {
                snprintf(
                    app->model.key_buf,
                    sizeof(app->model.key_buf),
                    "%s",
                    key_cstr);
                app->model.has_key = true;
            }
        }
        furi_string_free(key_temp);

        app->model.is_file_loaded = true;
        app->model.modified = false;
        success = true;
    } while(0);

    flipper_format_free(fff);
    return success;
}

bool subghz_editor_file_save(SubGhzEditorApp* app, const char* target_path) {
    if(!app || !app->model.is_file_loaded || !target_path) return false;

    FlipperFormat* fff = flipper_format_file_alloc(app->storage);
    bool success = false;

    do {
        if(!flipper_format_file_open_existing(fff, target_path)) {
            FURI_LOG_E("SubGhzEditor", "Save: Failed to open %s", target_path);
            break;
        }

        // Update Frequency
        if(app->model.has_frequency) {
            flipper_format_rewind(fff);
            if(!flipper_format_update_uint32(fff, "Frequency", &app->model.frequency, 1)) {
                FURI_LOG_W("SubGhzEditor", "Could not update Frequency key");
            }
        }

        // Update Key
        if(app->model.has_key && strlen(app->model.key_buf) > 0) {
            flipper_format_rewind(fff);
            if(!flipper_format_update_string_cstr(fff, "Key", app->model.key_buf)) {
                FURI_LOG_W("SubGhzEditor", "Could not update Key");
            }
        }

        success = true;
    } while(0);

    flipper_format_free(fff);
    return success;
}

bool subghz_editor_file_save_copy(SubGhzEditorApp* app) {
    if(!app || !app->model.is_file_loaded) return false;

    const char* src_path = furi_string_get_cstr(app->model.file_path);
    FuriString* dst_path = furi_string_alloc();
    FuriString* dirname = furi_string_alloc();
    FuriString* basename = furi_string_alloc();

    path_extract_dirname(src_path, dirname);
    path_extract_filename_no_ext(src_path, basename);

    furi_string_printf(
        dst_path,
        "%s/%s_edit%s",
        furi_string_get_cstr(dirname),
        furi_string_get_cstr(basename),
        SUBGHZ_EDITOR_APP_EXTENSION);

    // Copy original file first to keep all headers and comments intact
    FS_Error fs_res = storage_common_copy(
        app->storage,
        src_path,
        furi_string_get_cstr(dst_path));

    bool success = false;
    if(fs_res == FSE_OK) {
        success = subghz_editor_file_save(app, furi_string_get_cstr(dst_path));
    } else {
        FURI_LOG_E("SubGhzEditor", "Copy failed: %d", fs_res);
    }

    furi_string_free(dst_path);
    furi_string_free(dirname);
    furi_string_free(basename);

    return success;
}
