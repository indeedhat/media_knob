package ui

import (
	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/theme"
	"fyne.io/fyne/v2/widget"
)

func initDIalog(ui *UiModel) {
	ui.dialog = &DialogModel{ui.win}
}

type DialogModel struct {
	win fyne.Window
}

func (d DialogModel) Success(text string) {

}

func (d DialogModel) Error(text string) {

}

func (d DialogModel) Help(text string) {
	var modal *widget.PopUp

	closeButton := widget.NewButton("", func() { modal.Hide() })
	closeButton.SetIcon(theme.WindowCloseIcon())
	closeButton.Importance = widget.LowImportance

	content := container.NewBorder(
		nil,
		container.NewPadded(widget.NewLabel(text)),
		container.NewPadded(widget.NewIcon(theme.HelpIcon())),
		container.NewPadded(closeButton),
		container.NewPadded(
			widget.NewLabel(t("help")),
		),
	)

	modal = widget.NewModalPopUp(content, d.win.Canvas())
	modal.Show()
}
