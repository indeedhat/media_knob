package runner

import (
	"errors"

	knoblet "github.com/indeedhat/media-knob/knoblet-macro/internal"
	"github.com/indeedhat/media-knob/knoblet-macro/internal/config"
)

type Runner struct {
	cfg *config.Config

	sideButtonPressed bool
	baseButtonPressed bool
}

func New(cfg *config.Config) *Runner {
	return &Runner{cfg: cfg}
}

func (r *Runner) HandleEvent(e knoblet.Event) error {
	if e.Log != "" {
		return nil
	}

	if e.Button != 0 {
		return r.handleButtonEvent(e)
	}

	if e.Angle != 0 {
		return r.handleAngleEvent(e)
	}

	return errors.New("invalid event")
}

func (r *Runner) handleButtonEvent(e knoblet.Event) error {
	switch e.Button {
	case knoblet.SideButtonId:
		r.sideButtonPressed = e.ButtonDown
	case knoblet.BaseButtonId:
		r.baseButtonPressed = e.ButtonDown
	}

	panic("not implemented")
}

func (r *Runner) handleAngleEvent(e knoblet.Event) error {
	if e.Angle < 0 && r.cfg.AntiClockwise.Enabled {
		return runCommand(
			r.cfg.AntiClockwise.Cmd,
			state(e.Angle, r.sideButtonPressed, r.baseButtonPressed),
		)
	}

	if e.Angle > 0 && r.cfg.Clockwise.Enabled {
		return runCommand(
			r.cfg.Clockwise.Cmd,
			state(e.Angle, r.sideButtonPressed, r.baseButtonPressed),
		)
	}

	return nil
}

func runCommand(cmd string, data ...any) error {
	panic("not implemented")
}
