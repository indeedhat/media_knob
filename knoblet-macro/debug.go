package main

import (
	"fmt"

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
	return container.NewVBox(
		m.DebugEnable,
		container.NewVScroll(m.Debug),
	)
}

func initDebugTab(w fyne.Window) DebugModel {
	var m DebugModel

	m.Debug = widget.NewList(
		func() int {
			return len(m.data)
		},
		func() fyne.CanvasObject {
			return widget.NewLabel("empty")
		},
		func(i widget.ListItemID, o fyne.CanvasObject) {
			o.(*widget.Label).SetText(m.data[i])
		},
	)

	m.DebugEnable = widget.NewCheck("Debug Logs", func(b bool) {
		if !b {
			m.data = nil
			return
		}

		m.data = make([]string, 0, 100)
		for i := range 100 {
			m.data = append(m.data, fmt.Sprintf("Log line %d\n", i))
		}
	})

	return m
}
