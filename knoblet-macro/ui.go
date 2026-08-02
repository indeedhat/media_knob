package main

import (
	"embed"
	"fmt"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/driver/desktop"
	"fyne.io/fyne/v2/lang"
	"fyne.io/fyne/v2/widget"
)

//go:embed translations/*.json
var translationsFS embed.FS

//go:embed assets/*
var assetsFS embed.FS

type UiModel struct {
	app fyne.App
	win fyne.Window

	form  *FormModel
	debug *DebugModel

	cfg *Config
}

func initUi(cfg *Config) UiModel {
	if err := lang.AddTranslationsFS(translationsFS, "translations"); err != nil {
		fyne.LogError("failed to load translations", err)
	}

	var model UiModel

	model.cfg = cfg
	model.app = app.NewWithID(AppId)
	model.win = model.app.NewWindow(t("app.name"))

	model.win.SetCloseIntercept(model.win.Hide)
	initSystemTray(model)

	initDebugTab(&model)
	initFormTab(&model)

	model.win.SetContent(container.NewAppTabs(
		container.NewTabItem(t("tab.config"), container.NewPadded(
			model.form.Draw(),
		)),
		container.NewTabItem(t("tab.debug"), container.NewPadded(
			model.debug.Draw(),
		)),
	))

	return model
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

type DebugModel struct {
	DebugEnable *widget.Check
	Debug       *widget.List

	data []string
}

func (m DebugModel) Draw() fyne.CanvasObject {
	return container.NewBorder(
		m.DebugEnable,
		nil,
		nil,
		nil,
		container.NewPadded(
			widget.NewCard(t("label.logs"), "", m.Debug),
		),
	)
}

func initDebugTab(ui *UiModel) {
	ui.debug = &DebugModel{}

	ui.debug.Debug = widget.NewList(
		func() int {
			return len(ui.debug.data)
		},
		func() fyne.CanvasObject {
			return widget.NewLabel("empty")
		},
		func(i widget.ListItemID, o fyne.CanvasObject) {
			o.(*widget.Label).SetText(ui.debug.data[i])
		},
	)

	ui.debug.DebugEnable = widget.NewCheck(t("label.logs.enable"), func(b bool) {
		ui.cfg.LogsEnabled = b
		SaveConfig(ui.cfg)

		if !b {
			ui.debug.data = nil
			return
		}

		ui.debug.data = make([]string, 0)
	})

	ui.debug.DebugEnable.SetChecked(ui.cfg.LogsEnabled)
}

type FormModel struct {
	Clockwise        *widget.Entry
	AntiClockwise    *widget.Entry
	SideButton       *widget.Entry
	BaseButton       *widget.Entry
	SideButtonDouble *widget.Entry
	BaseButtonDouble *widget.Entry

	ClockwiseEnabled        *widget.Check
	AntiClockwiseEnabled    *widget.Check
	SideButtonEnabled       *widget.Check
	BaseButtonEnabled       *widget.Check
	SideButtonDoubleEnabled *widget.Check
	BaseButtonDoubleEnabled *widget.Check

	cfg *Config
}

func (m FormModel) Draw() fyne.CanvasObject {
	return &widget.Form{
		Items: []*widget.FormItem{
			{Text: t("label.clockwise"), Widget: m.ClockwiseEnabled},
			{Text: "", Widget: m.Clockwise},
			{Text: t("label.anti-clockwise"), Widget: m.AntiClockwiseEnabled},
			{Text: "", Widget: m.AntiClockwise},
			{Text: t("label.side-button"), Widget: m.SideButtonEnabled},
			{Text: "", Widget: m.SideButton},
			{Text: t("label.side-button-double"), Widget: m.SideButtonDoubleEnabled},
			{Text: "", Widget: m.SideButtonDouble},
			{Text: t("label.base-button"), Widget: m.BaseButtonEnabled},
			{Text: "", Widget: m.BaseButton},
			{Text: t("label.base-button-double"), Widget: m.BaseButtonDoubleEnabled},
			{Text: "", Widget: m.BaseButtonDouble},
		},
		OnSubmit: func() {
			m.cfg.Form.ClockwiseCmd = m.Clockwise.Text
			m.cfg.Form.AntiClockwiseCmd = m.AntiClockwise.Text
			m.cfg.Form.SideButtonCmd = m.SideButton.Text
			m.cfg.Form.SideButtonDoubleCmd = m.SideButtonDouble.Text
			m.cfg.Form.BaseButtonCmd = m.BaseButton.Text
			m.cfg.Form.BaseButtonDoubleCmd = m.BaseButtonDouble.Text

			m.cfg.Form.ClockwiseEnabled = m.ClockwiseEnabled.Checked
			m.cfg.Form.AntiClockwiseEnabled = m.AntiClockwiseEnabled.Checked
			m.cfg.Form.SideButtonEnabled = m.SideButtonEnabled.Checked
			m.cfg.Form.SideButtonDoubleEnabled = m.SideButtonDoubleEnabled.Checked
			m.cfg.Form.BaseButtonEnabled = m.BaseButtonEnabled.Checked
			m.cfg.Form.BaseButtonDoubleEnabled = m.BaseButtonDoubleEnabled.Checked

			// TODO: handle error and show toast
			SaveConfig(m.cfg)
		},
	}
}

func initFormTab(ui *UiModel) FormModel {
	m := FormModel{
		cfg: ui.cfg,
	}
	ui.form = &m

	m.Clockwise = widget.NewEntry()
	m.Clockwise.Disable()
	m.Clockwise.SetText(ui.cfg.Form.ClockwiseCmd)
	m.ClockwiseEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.Clockwise))
	m.ClockwiseEnabled.SetChecked(ui.cfg.Form.ClockwiseEnabled)

	m.AntiClockwise = widget.NewEntry()
	m.AntiClockwise.Disable()
	m.AntiClockwise.SetText(ui.cfg.Form.AntiClockwiseCmd)
	m.AntiClockwiseEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.AntiClockwise))
	m.AntiClockwiseEnabled.SetChecked(ui.cfg.Form.AntiClockwiseEnabled)

	m.SideButton = widget.NewEntry()
	m.SideButton.Disable()
	m.SideButton.SetText(ui.cfg.Form.SideButtonCmd)
	m.SideButtonEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.SideButton))
	m.SideButtonEnabled.SetChecked(ui.cfg.Form.SideButtonEnabled)

	m.SideButtonDouble = widget.NewEntry()
	m.SideButtonDouble.Disable()
	m.SideButtonDouble.SetText(ui.cfg.Form.SideButtonDoubleCmd)
	m.SideButtonDoubleEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.SideButtonDouble))
	m.SideButtonDoubleEnabled.SetChecked(ui.cfg.Form.SideButtonDoubleEnabled)

	m.BaseButton = widget.NewEntry()
	m.BaseButton.Disable()
	m.BaseButton.SetText(ui.cfg.Form.BaseButtonCmd)
	m.BaseButtonEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.BaseButton))
	m.BaseButtonEnabled.SetChecked(ui.cfg.Form.BaseButtonEnabled)

	m.BaseButtonDouble = widget.NewEntry()
	m.BaseButtonDouble.Disable()
	m.BaseButtonDouble.SetText(ui.cfg.Form.BaseButtonDoubleCmd)
	m.BaseButtonDoubleEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.BaseButtonDouble))
	m.BaseButtonDoubleEnabled.SetChecked(ui.cfg.Form.BaseButtonDoubleEnabled)

	return m
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
