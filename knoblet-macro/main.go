package main

import (
	"embed"
	"fmt"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/driver/desktop"
	"fyne.io/fyne/v2/lang"
)

//go:embed translations/*.json
var translationsFS embed.FS

//go:embed assets/*
var assetsFS embed.FS

func main() {
	a := app.NewWithID("dev.ihat.knoblet")
	w := a.NewWindow("Knoblet Macro")

	if err := lang.AddTranslationsFS(translationsFS, "translations"); err != nil {
		fyne.LogError("failed to load translations", err)
	}

	w.SetCloseIntercept(w.Hide)
	initSystemTray(a, w)

	w.SetContent(container.NewAppTabs(
		container.NewTabItem("Config", container.NewPadded(
			initFormTab(w).Draw(),
		)),
		container.NewTabItem("Debug", container.NewPadded(
			initDebugTab(w).Draw(),
		)),
	))

	w.ShowAndRun()
}

func initSystemTray(a fyne.App, w fyne.Window) error {
	desk, ok := a.(desktop.App)
	if !ok {
		return nil
	}

	m := fyne.NewMenu("Knoblet",
		fyne.NewMenuItem("Open", w.Show),
		fyne.NewMenuItem("Quit", a.Quit),
	)

	r, err := loadAsset("assets/tray.svg")
	if err != nil {
		return err
	}

	desk.SetSystemTrayIcon(r)
	desk.SetSystemTrayMenu(m)

	return nil
}

func loadAsset(path string) (fyne.Resource, error) {
	data, err := assetsFS.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("failed to load tray icon: %w", err)
	}

	return fyne.NewStaticResource(path, data), nil
}
