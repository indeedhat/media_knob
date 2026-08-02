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

func initUi(cfg *Config) (fyne.App, fyne.Window) {
	if err := lang.AddTranslationsFS(translationsFS, "translations"); err != nil {
		fyne.LogError("failed to load translations", err)
	}

	a := app.NewWithID(AppId)
	w := a.NewWindow(t("app.name"))

	w.SetCloseIntercept(w.Hide)
	initSystemTray(a, w)

	w.SetContent(container.NewAppTabs(
		container.NewTabItem(t("tab.config"), container.NewPadded(
			initFormTab(cfg, w).Draw(),
		)),
		container.NewTabItem(t("tab.debug"), container.NewPadded(
			initDebugTab(cfg, w).Draw(),
		)),
	))

	return a, w
}

func initSystemTray(a fyne.App, w fyne.Window) error {
	desk, ok := a.(desktop.App)
	if !ok {
		return nil
	}

	m := fyne.NewMenu(t("app.name"),
		fyne.NewMenuItem(t("tray.open"), w.Show),
		fyne.NewMenuItem(t("tray.quit"), a.Quit),
	)

	r, err := loadAsset(TrayIconPath)
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

func t(k string, d ...any) string {
	return lang.Localize(k, d...)
}
