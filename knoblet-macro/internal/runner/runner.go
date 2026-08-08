package runner

import (
	"context"
	"errors"
	"fmt"
	"os/exec"
	"time"

	knoblet "github.com/indeedhat/media-knob/knoblet-macro/internal"
	"github.com/indeedhat/media-knob/knoblet-macro/internal/config"

	shellwords "github.com/mattn/go-shellwords"
)

type Runner struct {
	Results chan Result
	cfg     *config.Config

	sideButtonLastKeyup time.Time
	baseButtonLastKeyup time.Time

	sideButtonHeld     bool
	baseButtonHeld     bool
	angleChangedOnHold bool

	scheduleCtxCancel context.CancelFunc
}

func New(cfg *config.Config) *Runner {
	return &Runner{
		Results: make(chan Result, 1),
		cfg:     cfg,
	}
}

func (r *Runner) HandleEvent(e knoblet.Event) {
	if e.Log != "" {
		return
	}

	if e.Button != 0 {
		r.handleButtonEvent(e)
	} else if e.Angle != 0 {
		r.handleAngleEvent(e)
	}

	r.Results <- Result{
		Evt: e,
		Err: errors.New("invalid event"),
	}
}

func (r *Runner) handleButtonEvent(e knoblet.Event) {
	defer r.updateLastKeyup(e)
	r.clearSchedule()

	var cmd, doubleCmd config.CommandConfig
	var lastKeyup time.Time

	switch e.Button {
	case knoblet.SideButtonId:
		r.sideButtonHeld = e.ButtonDown
		cmd = r.cfg.SideButton
		doubleCmd = r.cfg.SideButtonDouble
		lastKeyup = r.sideButtonLastKeyup
	case knoblet.BaseButtonId:
		r.baseButtonHeld = e.ButtonDown
		cmd = r.cfg.BaseButton
		doubleCmd = r.cfg.BaseButtonDouble
		lastKeyup = r.baseButtonLastKeyup
	}

	if e.ButtonDown {
		return
	}

	if r.rotationSkip() {
		r.angleChangedOnHold = r.baseButtonHeld || r.sideButtonHeld

		return
	}

	r.scheduleCommand(e, cmd, doubleCmd, lastKeyup)
}

func (r *Runner) handleAngleEvent(e knoblet.Event) {
	if r.angleWithinDeadzone(e) {
		return
	}

	r.angleChangedOnHold = r.sideButtonHeld || r.baseButtonHeld

	if e.Angle < 0 {
		r.run(e, r.cfg.AntiClockwise, r.stateString(e))
	} else {
		r.run(e, r.cfg.Clockwise, r.stateString(e))
	}
}

// angleWithinDeadzone checks if the events angle falls within the appropriate dead zone and should
// be ignored
func (r *Runner) angleWithinDeadzone(e knoblet.Event) bool {
	if e.Button == 0 && r.cfg.Meta.DeadZone != 0 {
		return between(int(e.Angle), r.cfg.Meta.DeadZone, -r.cfg.Meta.DeadZone)
	}

	if e.Button != 0 && r.cfg.Meta.KeyDownDeadZone != 0 {
		return between(int(e.Angle), r.cfg.Meta.KeyDownDeadZone, -r.cfg.Meta.KeyDownDeadZone)
	}

	return false
}

func (r *Runner) inDoublePressWIndow(last time.Time) bool {
	return time.Now().Sub(last) <= time.Duration(r.cfg.Meta.DoubleTapInterval)*time.Millisecond
}

// stateString builds a ; separated list of angle/button state to send to the macro command
func (r *Runner) stateString(e knoblet.Event) string {
	return fmt.Sprintf(`%d;%t;%t`, e.Angle, r.sideButtonHeld, r.baseButtonHeld)
}

func (r *Runner) scheduleCommand(e knoblet.Event, single, double config.CommandConfig, lastKeyup time.Time) {
	if !double.Enabled && !single.Enabled {
		return
	}

	if !double.Enabled {
		r.run(e, single)
		return
	}

	if !single.Enabled {
		if r.inDoublePressWIndow(lastKeyup) {
			r.run(e, double)
		}
		return
	}

	ctx, cancel := context.WithCancel(context.Background())
	r.cancelSchedule = cancel

	go func() {
		select {
		case <-time.After(time.Duration(r.cfg.Meta.DoubleTapInterval) * time.Millisecond):
			r.run(e, single)
		case <-ctx.Done():
			return
		}
	}()
}

func (r *Runner) rotationSkip() bool {
	if !r.cfg.Meta.DisableKeyUpAfterRotation {
		return false
	}

	return r.angleChangedOnHold
}

func (r *Runner) updateLastKeyup(e knoblet.Event) {
	if e.ButtonDown {
		return
	}

	switch e.Button {
	case knoblet.BaseButtonId:
		r.baseButtonLastKeyup = time.Now()
	case knoblet.SideButtonId:
		r.sideButtonLastKeyup = time.Now()
	}
}

func (r *Runner) run(e knoblet.Event, cmd config.CommandConfig, data ...any) {
	if !cmd.Enabled {
		return
	}

	if cmd.Cmd == "" {
		r.Results <- Result{
			Evt: e,
			Cmd: cmd,
			Err: errors.New("empty command"),
		}

		return
	}

	parts, err := shellwords.Parse(cmd.Cmd)
	if err != nil {
		r.Results <- Result{
			Evt: e,
			Cmd: cmd,
			Err: fmt.Errorf("failed to parse command string: %w", err),
		}
		return
	}

	start := time.Now()
	c := exec.Command(parts[0], parts[1:]...)
	_, err = c.Output()

	r.Results <- Result{
		Evt:      e,
		Cmd:      cmd,
		Err:      err,
		Duration: time.Now().Sub(start),
	}
}

func (r *Runner) clearSchedule() {
	if r.scheduleCtxCancel == nil {
		return
	}

	r.scheduleCtxCancel()
	r.scheduleCtxCancel = nil
}

// between checks if an angle falls between the min/max bounds
func between(angle, min, max int) bool {
	return angle >= min && angle <= max
}
