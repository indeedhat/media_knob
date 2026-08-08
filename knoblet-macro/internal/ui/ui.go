package ui

import (
	"fmt"
	"log"

	"github.com/indeedhat/media-knob/knoblet-macro/assets"
	knoblet "github.com/indeedhat/media-knob/knoblet-macro/internal"
	"github.com/indeedhat/media-knob/knoblet-macro/internal/config"
	"github.com/indeedhat/media-knob/knoblet-macro/translations"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/driver/desktop"
	"fyne.io/fyne/v2/lang"
	"fyne.io/fyne/v2/widget"
)

type UiModel struct {
	app fyne.App
	win fyne.Window

	dialog   *DialogModel
	commands *CommandsModel
	settings *SettingsModel
	debug    *DebugModel

	cfg *config.Config
}

func New(cfg *config.Config) UiModel {
	log.Print(translations.FS.ReadDir("lang_packs"))

	if err := lang.AddTranslationsFS(translations.FS, "lang_packs"); err != nil {
		fyne.LogError("failed to load translations", err)
	}

	var model UiModel

	model.cfg = cfg
	model.app = app.NewWithID(knoblet.AppId)
	model.win = model.app.NewWindow(t("app.name"))

	model.win.SetCloseIntercept(model.win.Hide)
	initDIalog(&model)

	initSystemTray(model)

	initDebugTab(&model)
	initCommandsTab(&model)
	initSettingsTab(&model)

	model.win.SetContent(container.NewAppTabs(
		container.NewTabItem(t("tab.commands"), container.NewPadded(
			model.commands.Draw(),
		)),
		container.NewTabItem(t("tab.settings"), container.NewPadded(
			model.settings.Draw(),
		)),
		container.NewTabItem(t("tab.debug"), container.NewPadded(
			model.debug.Draw(),
		)),
	))

	return model
}

func (m UiModel) Start() {
	m.win.ShowAndRun()
}

func (m UiModel) LogEvent(e knoblet.Event) {
	if !m.cfg.LogsEnabled {
		return
	}

	m.debug.data = append(m.debug.data, e.String())
	m.debug.Debug.Refresh()
	m.debug.Debug.ScrollToBottom()
}

func (m UiModel) Quit() {
	m.app.Quit()
}

func (m UiModel) modal(title string, icon, content *fyne.CanvasObject) {

}

func initSystemTray(model UiModel) error {
	desk, ok := model.app.(desktop.App)
	if !ok {
		return nil
	}

	m := fyne.NewMenu(t("app.name"),
		fyne.NewMenuItem(t("tray.open"), model.win.Show),
		fyne.NewMenuItem(t("tray.quit"), model.app.Quit),
	)

	r, err := loadAsset(knoblet.TrayIconPath)
	if err != nil {
		return err
	}

	desk.SetSystemTrayIcon(r)
	desk.SetSystemTrayMenu(m)

	return nil
}

func loadAsset(path string) (fyne.Resource, error) {
	data, err := assets.FS.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("failed to load tray icon: %w", err)
	}

	return fyne.NewStaticResource(path, data), nil
}

func t(k string, d ...any) string {
	return lang.Localize(k, d...)
}

func toggle(w fyne.Window, o *widget.Entry) func(b bool) {
	return func(b bool) {
		if b {
			w.Canvas().Focus(o)
			o.Enable()
		} else {
			o.Disable()
		}
	}
}
