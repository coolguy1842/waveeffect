# Wave

Requires [hidapi](https://github.com/libusb/hidapi) from [libusb](https://github.com/libusb)<br>

I made this as a successor to my [RazerSteelseriesWaveEffect2](https://github.com/coolguy1842/RazerSteelseriesWaveEffect2) as I got a new Keychron V6<br>
this requires custom firmware changes to the V6 in QMK<br>

I have provided a snippet to add below

## QMK Firmware

If you have not done this run through the [QMK Setup Guide](https://docs.qmk.fm/#/newbs_getting_started)

### Firmware Changes

add the following code to your keymap.c

```CPP
enum custom_channel_ids {
    id_custom_set_effect_channel = 5,
    id_custom_array_led_channel = 6,
    id_custom_single_led_channel = 7,
    id_custom_single_col_channel = 8,
    id_custom_array_col_channel = 9,
};

#define WAVE_COLS 22

const uint16_t PROGMEM cols[WAVE_COLS][MATRIX_ROWS] = {
    { 0,  20, 41, 61, 78, 94  },
    { 1,  21, 42, 62, 0xFF, 95  },
    { 2,  22, 43, 63, 79, 96  },
    { 3,  23, 44, 64, 80, 0xFF  },
    { 4,  24, 45, 65, 81, 0xFF  },
    { 5,  25, 46, 66, 82, 0xFF  },
    { 6,  26, 47, 67, 83, 97  },
    { 7,  27, 48, 68, 84, 0xFF  },
    { 8,  28, 49, 69, 85, 0xFF  },
    { 9,  29, 50, 70, 86, 0xFF  },
    { 10, 30, 51, 71, 87, 98  },
    { 11, 31, 52, 72, 88, 99  },
    { 12, 32, 53, 0xFF, 0xFF, 100 },
    { 0xFF, 0xFF, 54, 73, 89, 101 },
    { 13, 33, 55, 0xFF, 0xFF, 102 },
    { 14, 34, 56, 0xFF, 90, 103 },
    { 15, 35, 57, 0xFF, 0xFF, 104 },
    { 16, 36, 58, 74, 91, 105 },
    { 17, 37, 59, 75, 92, 106 },
    { 18, 38, 60, 76, 93, 107 },
    { 19, 39, 0xFF, 77, 0xFF, 0xFF },
    { 0xFF, 40, 0xFF, 0xFF, 0xFF, 0xFF  }
};


void via_custom_value_command_kb(uint8_t* data, uint8_t length) {
    uint8_t* command_id = &(data[0]);
    uint8_t* channel_id = &(data[1]);

    switch(*channel_id) {
    case id_custom_set_effect_channel: {
        switch(rgb_matrix_get_mode()) {
        case RGB_MATRIX_CUSTOM_custom_frame_effect: break;
        default:
            rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_custom_frame_effect);
            break;
        }

        break;
    }
    case id_custom_array_led_channel: {
        switch (*command_id) {
            case id_custom_set_value: {
                for(int i = 2; i < length; i += 4) {
                    uint8_t index = data[i];
                    if(index >= RGB_MATRIX_LED_COUNT) break;
                    uint8_t* rgb = &(data[i + 1]);

                    rgb_matrix_set_color(index, rgb[0], rgb[1], rgb[2]);
                }

                break;
            }
            default: {
                // Unhandled message.
                *command_id = id_unhandled;
                break;
            }
        }

        break;
    }
    case id_custom_single_led_channel: {
        switch (*command_id) {
            case id_custom_set_value: {
                uint8_t index = data[2];
                if(index >= RGB_MATRIX_LED_COUNT) break;
                uint8_t* rgb = &(data[3]);

                rgb_matrix_set_color(index, rgb[0], rgb[1], rgb[2]);

                break;
            }
            default: {
                // Unhandled message.
                *command_id = id_unhandled;
                break;
            }
        }

        break;
    }
    case id_custom_array_col_channel: {
        switch (*command_id) {
            case id_custom_set_value: {
                for(int i = 2; i < length; i += 4) {
                    if(data[i] >= WAVE_COLS) break;
                    const uint16_t* col = cols[data[i]];
                    uint8_t* rgb = &(data[i + 1]);

                    for(uint8_t j = 0; j < MATRIX_ROWS; j++) {
                        if(col[j] > RGB_MATRIX_LED_COUNT) continue;

                        rgb_matrix_set_color(col[j], rgb[0], rgb[1], rgb[2]);
                    }
                }

                break;
            }
            default: {
                // Unhandled message.
                *command_id = id_unhandled;
                break;
            }
        }

        break;
    }
    case id_custom_single_col_channel: {
        switch (*command_id) {
            case id_custom_set_value: {
                if(data[2] >= WAVE_COLS) break;
                const uint16_t* col = cols[data[2]];
                uint8_t* rgb = &(data[3]);

                for(uint8_t i = 0; i < MATRIX_ROWS; i++) {
                    if(col[i] > RGB_MATRIX_LED_COUNT) continue;

                    rgb_matrix_set_color(col[i], rgb[0], rgb[1], rgb[2]);
                }

                break;
            }
            default: {
                // Unhandled message.
                *command_id = id_unhandled;
                break;
            }
        }

        break;
    }
    default:
        *command_id = id_unhandled;

        break;
    }
}
```

create a new file called rgb_matrix_user.inc in the folder containing keymap.c
and give it the following code

```C
// !!! DO NOT ADD #pragma once !!! //

RGB_MATRIX_EFFECT(custom_frame_effect)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static bool custom_frame_effect(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    return rgb_matrix_check_finished_leds(led_max);
}

#endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
```
