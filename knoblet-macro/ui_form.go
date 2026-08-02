package main

import (
	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/widget"
)

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

func initFormTab(cfg *Config, w fyne.Window) FormModel {
	m := FormModel{
		cfg: cfg,
	}

	m.Clockwise = widget.NewEntry()
	m.Clockwise.Disable()
	m.Clockwise.SetText(cfg.Form.ClockwiseCmd)
	m.ClockwiseEnabled = widget.NewCheck(t("label.enable"), toggle(w, m.Clockwise))
	m.ClockwiseEnabled.SetChecked(cfg.Form.ClockwiseEnabled)

	m.AntiClockwise = widget.NewEntry()
	m.AntiClockwise.Disable()
	m.AntiClockwise.SetText(cfg.Form.AntiClockwiseCmd)
	m.AntiClockwiseEnabled = widget.NewCheck(t("label.enable"), toggle(w, m.AntiClockwise))
	m.AntiClockwiseEnabled.SetChecked(cfg.Form.AntiClockwiseEnabled)

	m.SideButton = widget.NewEntry()
	m.SideButton.Disable()
	m.SideButton.SetText(cfg.Form.SideButtonCmd)
	m.SideButtonEnabled = widget.NewCheck(t("label.enable"), toggle(w, m.SideButton))
	m.SideButtonEnabled.SetChecked(cfg.Form.SideButtonEnabled)

	m.SideButtonDouble = widget.NewEntry()
	m.SideButtonDouble.Disable()
	m.SideButtonDouble.SetText(cfg.Form.SideButtonDoubleCmd)
	m.SideButtonDoubleEnabled = widget.NewCheck(t("label.enable"), toggle(w, m.SideButtonDouble))
	m.SideButtonDoubleEnabled.SetChecked(cfg.Form.SideButtonDoubleEnabled)

	m.BaseButton = widget.NewEntry()
	m.BaseButton.Disable()
	m.BaseButton.SetText(cfg.Form.BaseButtonCmd)
	m.BaseButtonEnabled = widget.NewCheck(t("label.enable"), toggle(w, m.BaseButton))
	m.BaseButtonEnabled.SetChecked(cfg.Form.BaseButtonEnabled)

	m.BaseButtonDouble = widget.NewEntry()
	m.BaseButtonDouble.Disable()
	m.BaseButtonDouble.SetText(cfg.Form.BaseButtonDoubleCmd)
	m.BaseButtonDoubleEnabled = widget.NewCheck(t("label.enable"), toggle(w, m.BaseButtonDouble))
	m.BaseButtonDoubleEnabled.SetChecked(cfg.Form.BaseButtonDoubleEnabled)

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
