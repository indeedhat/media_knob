package main

import (
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

	quit := make(chan os.Signal, 1)
	signal.Notify(quit, os.Interrupt, syscall.SIGTERM)

	a, w := initUi(cfg)
	defer a.Quit()

	w.ShowAndRun()

	<-quit
}
