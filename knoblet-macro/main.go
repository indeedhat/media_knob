package main

import (
	"log"
	"os"
	"os/signal"
	"syscall"
)

const (
	AppId = "dev.ihat.knoblet"

	TrayIconPath = "assets/tray.svg"
)

func main() {
	cfg, err := LoadConfig()
	if err != nil {
		cfg = &Config{}
	}

	events := make(chan Event, 1)
	quit := make(chan os.Signal, 1)
	signal.Notify(quit, os.Interrupt, syscall.SIGTERM)

	ui := initUi(cfg)

	go func() {
		go InitBluetooth(events)
		for {
			select {
			case e := <-events:
				if ui.cfg.LogsEnabled {
					ui.debug.data = append(ui.debug.data, e.String())
					ui.debug.Debug.Refresh()
					ui.debug.Debug.ScrollToBottom()
				}

				// TODO: handle triggering macros

			case <-quit:
				ui.app.Quit()
				log.Println("quitting")
				return
			}
		}
	}()

	ui.win.ShowAndRun()
}
