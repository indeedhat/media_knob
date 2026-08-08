package internal

import "fmt"

type Event struct {
	Angle      int16
	Button     int8
	ButtonDown bool
	Log        string
}

func (e Event) String() string {
	if e.Log != "" {
		return e.Log
	}

	if e.Angle != 0 {
		return fmt.Sprintf("Angle delta: %d", e.Angle)
	}

	if e.Button != 0 {
		button := "unknown"
		switch e.Button {
		case SideButtonId:
			button = SideButton
		case BaseButtonId:
			button = BaseButton
		}

		if e.ButtonDown {
			return fmt.Sprintf("Key down (%s)", button)
		}

		return fmt.Sprintf("Key up (%s)", button)
	}

	return "Unknown event"
}
