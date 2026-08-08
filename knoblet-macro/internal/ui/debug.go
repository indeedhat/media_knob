package ui

import (
	"github.com/indeedhat/media-knob/knoblet-macro/internal/config"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/widget"
)

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
		config.Save(ui.cfg)

		if !b {
			ui.debug.data = nil
			return
		}

		ui.debug.data = make([]string, 0)
	})

	ui.debug.DebugEnable.SetChecked(ui.cfg.LogsEnabled)
}
