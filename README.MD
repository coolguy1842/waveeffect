# Wave

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
    id_custom_single_led_channel = 7
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
