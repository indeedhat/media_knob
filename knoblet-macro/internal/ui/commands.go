package ui

import (
	"github.com/indeedhat/media-knob/knoblet-macro/internal/config"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/theme"
	"fyne.io/fyne/v2/widget"
)

type CommandsModel struct {
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

	cfg    *config.Config
	dialog *DialogModel
}

func (m CommandsModel) Draw() fyne.CanvasObject {
	return &widget.Form{
		SubmitText: t("label.save"),
		Items: []*widget.FormItem{
			// rotation
			m.item("label.clockwise", "", m.ClockwiseEnabled),
			m.item("", "help.clockwise", m.Clockwise),
			m.item("label.anti-clockwise", "", m.AntiClockwiseEnabled),
			m.item("", "help.anti-clockwise", m.AntiClockwise),

			// side button
			m.item("label.side-button", "", m.SideButtonEnabled),
			m.item("", "help.side-button", m.SideButton),
			m.item("label.side-button-double", "", m.SideButtonDoubleEnabled),
			m.item("", "help.side-button-double", m.SideButtonDouble),

			// base button
			m.item("label.base-button", "", m.BaseButtonEnabled),
			m.item("", "help.base-button", m.BaseButton),
			m.item("label.base-button-double", "", m.BaseButtonDoubleEnabled),
			m.item("", "help.base-button-double", m.BaseButtonDouble),
		},
		OnSubmit: m.onSubmit,
	}
}

func (m CommandsModel) onSubmit() {
	m.cfg.AntiClockwise.Cmd = m.AntiClockwise.Text
	m.cfg.AntiClockwise.Enabled = m.AntiClockwiseEnabled.Checked
	m.cfg.Clockwise.Cmd = m.Clockwise.Text
	m.cfg.Clockwise.Enabled = m.ClockwiseEnabled.Checked

	m.cfg.SideButton.Cmd = m.SideButton.Text
	m.cfg.SideButton.Enabled = m.SideButtonEnabled.Checked
	m.cfg.SideButtonDouble.Cmd = m.SideButtonDouble.Text
	m.cfg.SideButtonDouble.Enabled = m.SideButtonDoubleEnabled.Checked

	m.cfg.BaseButton.Cmd = m.BaseButton.Text
	m.cfg.BaseButton.Enabled = m.BaseButtonEnabled.Checked
	m.cfg.BaseButtonDouble.Cmd = m.BaseButtonDouble.Text
	m.cfg.BaseButtonDouble.Enabled = m.BaseButtonDoubleEnabled.Checked

	if err := config.Save(m.cfg); err != nil {
		m.dialog.Error(t("error.config-not-saved"))
	} else {
		m.dialog.Success(t("error.config-saved"))
	}
}

func initCommandsTab(ui *UiModel) CommandsModel {
	m := CommandsModel{
		cfg:    ui.cfg,
		dialog: ui.dialog,
	}
	ui.commands = &m

	m.Clockwise = widget.NewEntry()
	m.Clockwise.Disable()
	m.Clockwise.SetText(ui.cfg.Clockwise.Cmd)
	m.ClockwiseEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.Clockwise))
	m.ClockwiseEnabled.SetChecked(ui.cfg.Clockwise.Enabled)

	m.AntiClockwise = widget.NewEntry()
	m.AntiClockwise.Disable()
	m.AntiClockwise.SetText(ui.cfg.AntiClockwise.Cmd)
	m.AntiClockwiseEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.AntiClockwise))
	m.AntiClockwiseEnabled.SetChecked(ui.cfg.AntiClockwise.Enabled)

	m.SideButton = widget.NewEntry()
	m.SideButton.Disable()
	m.SideButton.SetText(ui.cfg.SideButton.Cmd)
	m.SideButtonEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.SideButton))
	m.SideButtonEnabled.SetChecked(ui.cfg.SideButton.Enabled)

	m.SideButtonDouble = widget.NewEntry()
	m.SideButtonDouble.Disable()
	m.SideButtonDouble.SetText(ui.cfg.SideButtonDouble.Cmd)
	m.SideButtonDoubleEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.SideButtonDouble))
	m.SideButtonDoubleEnabled.SetChecked(ui.cfg.SideButtonDouble.Enabled)

	m.BaseButton = widget.NewEntry()
	m.BaseButton.Disable()
	m.BaseButton.SetText(ui.cfg.BaseButton.Cmd)
	m.BaseButtonEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.BaseButton))
	m.BaseButtonEnabled.SetChecked(ui.cfg.BaseButton.Enabled)

	m.BaseButtonDouble = widget.NewEntry()
	m.BaseButtonDouble.Disable()
	m.BaseButtonDouble.SetText(ui.cfg.BaseButtonDouble.Cmd)
	m.BaseButtonDoubleEnabled = widget.NewCheck(t("label.enable"), toggle(ui.win, m.BaseButtonDouble))
	m.BaseButtonDoubleEnabled.SetChecked(ui.cfg.BaseButtonDouble.Enabled)

	return m
}

func (m CommandsModel) item(label, helpText string, input fyne.CanvasObject) *widget.FormItem {
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
