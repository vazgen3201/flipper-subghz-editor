# Sub-GHz Editor for Flipper Zero

A modular, native Flipper Zero application (`.fap`) to open saved Sub-GHz `.sub` files, adjust frequency using standard presets (including 433.88 MHz, 433.92 MHz, 315.00 MHz, 868.35 MHz, 915.00 MHz), edit key codes in HEX, and save modifications either by overwriting or creating a new copy.

## Features

- **File Browser Integration**: Easily browse and pick any `.sub` file stored on SD card (`/ext/subghz`).
- **Standard Frequency Presets**: Quick cycling between common Sub-GHz frequencies:
  - 315.00 MHz
  - 318.00 MHz
  - 345.00 MHz
  - 390.00 MHz
  - 433.07 MHz
  - 433.88 MHz
  - 433.92 MHz
  - 434.42 MHz
  - 434.77 MHz
  - 868.35 MHz
  - 868.85 MHz
  - 915.00 MHz
  - Preserves and identifies non-standard custom frequencies automatically.
- **HEX Key Editor**: Interactive text input to modify protocol transmission keys directly.
- **Safe Saving**:
  - Overwrite current file while maintaining file header integrity.
  - Save As Copy with `_edit.sub` suffix to prevent accidental loss of original captures.
- **Haptic & Sound Feedback**: Vibro and acoustic notification on successful operations.

## Architecture & Code Structure

The project strictly follows the Separation of Concerns principle:
- `application.fam`: Application metadata and manifest.
- `subghz_editor.png`: 10x10 monochrome application icon.
- `subghz_editor_model.h`: Core data types, frequency tables, and application state models.
- `subghz_editor_file.h` / `subghz_editor_file.c`: File loading, `FlipperFormat` parsing, and storage update logic.
- `subghz_editor_view.h` / `subghz_editor_view.c`: GUI views (`VariableItemList`, `TextInput`, `Widget` status dialogs).
- `subghz_editor.c`: Main application lifecycle, service record handles, and view dispatcher orchestrator.

## Building & Installation

To build using `ufbt` (micro Flipper Build Tool):
```bash
ufbt
```

To install directly to a connected Flipper Zero:
```bash
ufbt launch
```

Developed by Vazgen Grigoryan.
