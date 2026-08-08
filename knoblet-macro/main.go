package main

import (
	"log"
	"os"
	"os/signal"
	"syscall"

	knoblet "github.com/indeedhat/media-knob/knoblet-macro/internal"
	"github.com/indeedhat/media-knob/knoblet-macro/internal/bluetooth"
	"github.com/indeedhat/media-knob/knoblet-macro/internal/config"
	"github.com/indeedhat/media-knob/knoblet-macro/internal/runner"
	"github.com/indeedhat/media-knob/knoblet-macro/internal/ui"
)

func main() {
	cfg, err := config.Load()
	if err != nil {
		log.Fatal("failed to load config: %w", err)
	}

	runner := runner.New(cfg)

	events := make(chan knoblet.Event, 1)
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, os.Interrupt, syscall.SIGTERM)

	ui := ui.New(cfg)

	go func() {
		go bluetooth.New(events)
		for {
			select {
			case e := <-events:
				ui.LogEvent(e)
				runner.HandleEvent(e)

			case <-quit:
				ui.Quit()
				log.Println("quitting")
				return
			}
		}
	}()

	ui.Start()
}
