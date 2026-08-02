package main

import (
	"errors"
	"log"
	"os"
	"path"
	"runtime"

	"github.com/indeedhat/icl"
)

const ConfigFileName = "/knoblet/config.icl"

type Config struct {
	Version     int  `icl:"version"`
	LogsEnabled bool `icl:"log_enabled"`

	Form FormConfig `icl:"form"`
}

type FormConfig struct {
	// form checkboxes
	ClockwiseEnabled        bool `icl:"clockwise_enabled"`
	AntiClockwiseEnabled    bool `icl:"anti_clockwise_enabled"`
	SideButtonEnabled       bool `icl:"s_ide_button_enabled"`
	SideButtonDoubleEnabled bool `icl:"side_button_double_enabled"`
	BaseButtonEnabled       bool `icl:"base_button_enabled"`
	BaseButtonDoubleEnabled bool `icl:"base_button_double_enabled"`

	// form data
	ClockwiseCmd        string `icl:"clockwise_cmd"`
	AntiClockwiseCmd    string `icl:"anti_clockwise_cmd"`
	SideButtonCmd       string `icl:"s_ide_button_cmd"`
	SideButtonDoubleCmd string `icl:"side_button_double_cmd"`
	BaseButtonCmd       string `icl:"base_button_cmd"`
	BaseButtonDoubleCmd string `icl:"base_button_double_cmd"`
}

func LoadConfig() (*Config, error) {
	var config Config

	if _, err := os.Stat(configHome() + ConfigFileName); errors.Is(err, os.ErrNotExist) {
		config = Config{Version: 1, Form: FormConfig{}}

		os.MkdirAll(path.Dir(configHome()+ConfigFileName), os.ModePerm)
		log.Print(SaveConfig(&config))

		return &config, nil
	}

	err := icl.UnMarshalFile(configHome()+ConfigFileName, &config)
	if err != nil {
		return nil, err
	}

	return &config, nil
}

func SaveConfig(cfg *Config) error {
	return icl.MarshalFile(*cfg, configHome()+ConfigFileName)
}

func configHome() string {
	switch runtime.GOOS {
	case "windows":
		return os.Getenv("APPDATA")
	case "darwin":
		return os.Getenv("HOME") + "/Library/Application Support"
	default:
		if xdg := os.Getenv("XDG_CONFIG_HOME"); xdg != "" {
			return xdg
		}

		return os.Getenv("HOME")
	}
}
