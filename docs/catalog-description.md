# Sub-GHz Editor

Sub-GHz file frequency and transmission key code editor for Flipper Zero.

Open any captured or saved Sub-GHz `.sub` file directly on your Flipper Zero, adjust frequency to standard regional presets or set a custom frequency, edit the hex key payload, and save changes in place or to a separate backup file.

## Features

- Native File Browser: Browse and open any `.sub` file stored on SD card (/ext/subghz).
- Standard Presets: Quick cycling between common frequencies (315.00 MHz, 433.07 MHz, 433.88 MHz, 433.92 MHz, 868.35 MHz, 915.00 MHz, and more).
- Custom Frequency Input: Enter exact target frequency with automatic hardware band validation (300-348 MHz, 387-464 MHz, 779-928 MHz).
- HEX Key Editor: Direct interactive hexadecimal editor for protocol payload keys.
- Safe Saving: Overwrite current file or Save As Copy with _edit.sub suffix.
- File Manager Integration: Associate and launch directly with .sub file arguments.

## Navigation and Controls

- Press OK on Open .sub File to select a file via file browser.
- Press Left / Right on Freq to cycle standard frequency presets.
- Press OK on Freq to enter manual frequency mode.
- Press OK on Key to open hexadecimal key editor.
- Select Save (Overwrite) or Save As Copy to write modifications.
- Press Back to return or close current file.
