
#define MEDIA_DEBOUNCE_TIME 300
#define MEDIA_DOUBLE_TAP_INTERVAL 500

#define SCROLL_POLL_DELAY 10
#define MEDIA_POLL_DELAY 160

#define SETTINGS_MODE "mode"
//#define SAVE_MODE_STATE


enum op_modes {
	MODE_SCROLL,
	MODE_MEDIA,
	MODE_COUNT,
};


typedef struct {
	const struct device *as5600;
} devices;
