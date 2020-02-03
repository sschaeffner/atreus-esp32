#ifndef keys_h
#define keys_h

//see https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf

#define MOD_L1      0xFF00

#define MOD         0xFE
#define MOD_CTRL    0xFE01
#define MOD_SHIFT   0xFE02
#define MOD_ALT     0xFE04
#define MOD_GUI     0xFE08
#define MOD_R_CTRL  0xFE10
#define MOD_R_SHIFT 0xFE20
#define MOD_R_ALT   0xFE40
#define MOD_R_GUI   0xFE80

#define KEY_A   0x04
#define KEY_B   0x05
#define KEY_C   0x06
#define KEY_D   0x07
#define KEY_E   0x08
#define KEY_F   0x09
#define KEY_G   0x0A
#define KEY_H   0x0B
#define KEY_I   0x0C
#define KEY_J   0x0D
#define KEY_K   0x0E
#define KEY_L   0x0F
#define KEY_M   0x10
#define KEY_N   0x11
#define KEY_O   0x12
#define KEY_P   0x13
#define KEY_Q   0x14
#define KEY_R   0x15
#define KEY_S   0x16
#define KEY_T   0x17
#define KEY_U   0x18
#define KEY_V   0x19
#define KEY_W   0x1A
#define KEY_X   0x1B
#define KEY_Y   0x1C
#define KEY_Z   0x1D

#define KEY_1   0x1E
#define KEY_2   0x1F
#define KEY_3   0x20
#define KEY_4   0x21
#define KEY_5   0x22
#define KEY_6   0x23
#define KEY_7   0x24
#define KEY_8   0x25
#define KEY_9   0x26
#define KEY_0   0x27

#define KEY_EXCLAMATION         0x021E
#define KEY_AT                  0x021F
#define KEY_HASH                0x0220
#define KEY_DOLLAR              0x0221
#define KEY_PERCENT             0x0222
#define KEY_CARET               0x0223
#define KEY_AMPERSAND           0x0224
#define KEY_TIMES               0x0225
#define KEY_ROUND_BRACKET_OPEN  0x0226
#define KEY_ROUND_BRACKET_CLOSE 0x0227

#define KEY_ENTER                   0x28
#define KEY_ESC                     0x29
#define KEY_BACKSPACE               0x2A
#define KEY_TAB                     0x2B
#define KEY_SPACE                   0x2C
#define KEY_MINUS                   0x2D
#define KEY_EQUALS                  0x2E
#define KEY_SQUARE_BRACKET_OPEN     0x2F
#define KEY_SQUARE_BRACKET_CLOSE    0x30
#define KEY_BACKSLASH               0x31
#define KEY_SEMICOLON               0x33
#define KEY_SINGLE_APOSTR           0x34
#define KEY_GRACE_ACCENT            0x35
#define KEY_COMMA                   0x36
#define KEY_DOT                     0x37
#define KEY_SLASH                   0x38

#define KEY_PLUS                   0x022E
#define KEY_CURLY_BRACKET_OPEN     0x022F
#define KEY_CURLY_BRACKET_CLOSE    0x0230

#define KEY_F1  0x3A
#define KEY_F2  0x3B
#define KEY_F3  0x3C
#define KEY_F4  0x3D
#define KEY_F5  0x3E
#define KEY_F6  0x3F
#define KEY_F7  0x40
#define KEY_F8  0x41
#define KEY_F9  0x42
#define KEY_F10 0x43
#define KEY_F11 0x44
#define KEY_F12 0x45

#define KEY_PAGE_UP     0x4B
#define KEY_PAGE_DOWN   0x4E
#define KEY_ARROW_RIGHT 0x4F
#define KEY_ARROW_LEFT  0x50
#define KEY_ARROW_DOWN  0x51
#define KEY_ARROW_UP    0x52

#define CONSUMER                        0xF0
#define CONSUMER_AUDIO_MUTE             0xF0E2
#define CONSUMER_AUDIO_VOL_UP           0xF0E9
#define CONSUMER_AUDIO_VOL_DOWN         0xF0EA
#define CONSUMER_TRANSPORT_NEXT_TRACK   0xF0B5
#define CONSUMER_TRANSPORT_PREV_TRACK   0xF0B6
#define CONSUMER_TRANSPORT_STOP         0xF0B7
#define CONSUMER_TRANSPORT_STOP_EJECT   0xF0CC
#define CONSUMER_TRANSPORT_PLAY_PAUSE   0xF0CD
#define CONSUMER_BRIGHTNESS_UP          0xF06F
#define CONSUMER_BRIGHTNESS_DOWN        0xF070

#endif //keys_h