package ui

import (
	"strconv"

	"github.com/indeedhat/media-knob/knoblet-macro/internal/config"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/theme"
	"fyne.io/fyne/v2/widget"
)

type SettingsModel struct {
	DeadZone               *widget.Entry
	KeyDownDeadZone        *widget.Entry
	DoubleTapInterval      *widget.Entry
	DisableKeyupOnRotation *widget.Check

	cfg    *config.Config
	dialog *DialogModel
}

func (m SettingsModel) Draw() fyne.CanvasObject {
	return &widget.Form{
		SubmitText: t("label.save"),
		Items: []*widget.FormItem{
			m.item("label.dead-zone", "help.dead-zone", m.DeadZone),
			m.item("label.key-down-dead-zone", "help.key-down-dead-zone", m.KeyDownDeadZone),
			m.item("label.double-tap-interval", "help.double-tap-interval", m.DoubleTapInterval),
			m.item("label.disable-keyup-on-rotation", "help.disable-keyup-on-rotation", m.DisableKeyupOnRotation),
		},
		OnSubmit: m.onSubmit,
	}
}

func (m SettingsModel) onSubmit() {
	m.cfg.Meta.DeadZone, _ = strconv.Atoi(m.DeadZone.Text)
	m.cfg.Meta.KeyDownDeadZone, _ = strconv.Atoi(m.KeyDownDeadZone.Text)
	m.cfg.Meta.DoubleTapInterval, _ = strconv.Atoi(m.DoubleTapInterval.Text)
	m.cfg.Meta.DisableKeyUpAfterRotation = m.DisableKeyupOnRotation.Checked

	if err := config.Save(m.cfg); err != nil {
		m.dialog.Error(t("error.config-not-saved"))
	} else {
		m.dialog.Success(t("error.config-saved"))
	}
}

func initSettingsTab(ui *UiModel) SettingsModel {
	m := SettingsModel{
		cfg:    ui.cfg,
		dialog: ui.dialog,
	}
	ui.settings = &m

	m.DeadZone = &newIntEntry().Entry
	m.DeadZone.Text = strconv.Itoa(m.cfg.Meta.DeadZone)
	m.DeadZone.Enable()

	m.KeyDownDeadZone = &newIntEntry().Entry
	m.KeyDownDeadZone.Text = strconv.Itoa(m.cfg.Meta.KeyDownDeadZone)
	m.KeyDownDeadZone.Enable()

	m.DoubleTapInterval = &newIntEntry().Entry
	m.DoubleTapInterval.Text = strconv.Itoa(m.cfg.Meta.DoubleTapInterval)
	m.DoubleTapInterval.Enable()

	m.DisableKeyupOnRotation = widget.NewCheck("", nil)
	m.DisableKeyupOnRotation.Checked = m.cfg.Meta.DisableKeyUpAfterRotation

	return m
}

func (m SettingsModel) item(label, helpText string, input fyne.CanvasObject) *widget.FormItem {
	if label != "" {
		label = t(label)
	}

	if helpText == "" {
		return &widget.FormItem{
			Text:   label,
			Widget: input,
		}
	}

	helpButton := widget.NewButton("", func() {
		m.dialog.Help(t(helpText))
	})
	helpButton.SetIcon(theme.HelpIcon())
	helpButton.Importance = widget.LowImportance

	return &widget.FormItem{
		Text: label,
		Widget: container.NewBorder(
			nil,
			nil,
			nil,
			helpButton,
			input,
		),
	}
}
