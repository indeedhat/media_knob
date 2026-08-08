package runner

import (
	"time"

	knoblet "github.com/indeedhat/media-knob/knoblet-macro/internal"
	"github.com/indeedhat/media-knob/knoblet-macro/internal/config"
)

type Result struct {
	Evt      knoblet.Event
	Cmd      config.CommandConfig
	Err      error
	Duration time.Duration
}
