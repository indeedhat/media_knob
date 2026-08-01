package main

import (
	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
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
}

func (m FormModel) Draw() fyne.CanvasObject {
	return container.NewVBox(
		m.ClockwiseEnabled,
		m.Clockwise,
		m.AntiClockwiseEnabled,
		m.AntiClockwise,
		m.SideButtonEnabled,
		m.SideButton,
		m.SideButtonDoubleEnabled,
		m.SideButtonDouble,
		m.BaseButtonEnabled,
		m.BaseButton,
		m.BaseButtonDoubleEnabled,
		m.BaseButtonDouble,
	)
}

func initFormTab(w fyne.Window) FormModel {
	var m FormModel

	m.Clockwise = widget.NewEntry()
	m.Clockwise.Disable()
	m.ClockwiseEnabled = widget.NewCheck("Clockwise", toggle(w, m.Clockwise))

	m.AntiClockwise = widget.NewEntry()
	m.AntiClockwise.Disable()
	m.AntiClockwiseEnabled = widget.NewCheck("Anti-Clockwise", toggle(w, m.AntiClockwise))

	m.SideButton = widget.NewEntry()
	m.SideButton.Disable()
	m.SideButtonEnabled = widget.NewCheck("Side Button", toggle(w, m.SideButton))

	m.SideButtonDouble = widget.NewEntry()
	m.SideButtonDouble.Disable()
	m.SideButtonDoubleEnabled = widget.NewCheck("Side Button Double Tap", toggle(w, m.SideButtonDouble))

	m.BaseButton = widget.NewEntry()
	m.BaseButton.Disable()
	m.BaseButtonEnabled = widget.NewCheck("Base Button", toggle(w, m.BaseButton))

	m.BaseButtonDouble = widget.NewEntry()
	m.BaseButtonDouble.Disable()
	m.BaseButtonDoubleEnabled = widget.NewCheck("Base Button Double Tap", toggle(w, m.BaseButtonDouble))

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
