package config

import (
	"errors"
	"log"
	"os"
	"path"
	"runtime"

	knoblet "github.com/indeedhat/media-knob/knoblet-macro/internal"

	"github.com/indeedhat/icl"
)

type Config struct {
	Version int `icl:"version"`

	// debug
	LogsEnabled bool `icl:"log_enabled"`

	// commands
	Clockwise        CommandConfig `icl:"clockwise"`
	AntiClockwise    CommandConfig `icl:"anti_clockwise"`
	SideButton       CommandConfig `icl:"side_button"`
	SideButtonDouble CommandConfig `icl:"side_button_double"`
	BaseButton       CommandConfig `icl:"base_button"`
	BaseButtonDouble CommandConfig `icl:"base_button_double"`

	// meta
	Meta MetaConfig `icl:"meta"`
}

type CommandConfig struct {
	Enabled bool   `icl:"enabled"`
	Cmd     string `icl:"cmd"`
}

type MetaConfig struct {
	DeadZone                  int  `icl:"dead_zone"`
	KeyDownDeadZone           int  `icl:"key_down_dead_zone"`
	DoubleTapInterval         int  `icl:"double_tap_interval"`
	DisableKeyUpAfterRotation bool `icl:"disable_key_up_after_rotation"`
}

func defaultConfig() Config {
	return Config{
		Version: 1,
		Meta: MetaConfig{
			DeadZone:                  0,
			KeyDownDeadZone:           10,
			DoubleTapInterval:         1000,
			DisableKeyUpAfterRotation: true,
		},
	}
}

func Load() (*Config, error) {
	var config Config

	if _, err := os.Stat(savePath()); errors.Is(err, os.ErrNotExist) {
		config = defaultConfig()

		os.MkdirAll(path.Dir(savePath()), os.ModePerm)
		log.Print(Save(&config))

		return &config, nil
	}

	err := icl.UnMarshalFile(savePath(), &config)
	if err != nil {
		return nil, err
	}

	return &config, nil
}

func Save(cfg *Config) error {
	return icl.MarshalFile(*cfg, savePath())
}

func savePath() string {
	var home string
	switch runtime.GOOS {
	case "windows":
		home = os.Getenv("APPDATA")
	case "darwin":
		home = os.Getenv("HOME") + "/Library/Application Support"
	default:
		if xdg := os.Getenv("XDG_CONFIG_HOME"); xdg != "" {
			home = xdg
		} else {
			home = os.Getenv("HOME")
		}

	}

	return path.Join(home, knoblet.ConfigDir, knoblet.ConfigFileName)
}
