package main

import (
	"encoding/binary"
	"fmt"

	"tinygo.org/x/bluetooth"
)

const (
	DeviceName = "Knoblet"

	ServiceUUID        = "F28EC1C1-7BE9-4776-B063-578239F29BFF"
	CharacteristicUUID = "29389447-CCD7-4E89-B8C6-3437651B5488"

	Button2  = "Side Button"
	Button11 = "Base Button"
)

var adapter = bluetooth.DefaultAdapter

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
		case 2:
			button = Button2
		case 11:
			button = Button11
		}

		if e.ButtonDown {
			return fmt.Sprintf("Key down (%s)", button)
		}

		return fmt.Sprintf("Key up (%s)", button)
	}

	return "Unknown event"
}

func logEvent(e chan<- Event, format string, args ...any) {
	e <- Event{
		Log: fmt.Sprintf(format, args...),
	}
}

func InitBluetooth(e chan<- Event) {
	if err := adapter.Enable(); err != nil {
		logEvent(e, "failed to enable bluetooth adapter: %s", err)
		return
	}

	var result bluetooth.ScanResult
	seen := make(map[string]struct{})

	err := adapter.Scan(func(a *bluetooth.Adapter, sr bluetooth.ScanResult) {
		if _, found := seen[sr.Address.String()]; !found {
			logEvent(e, "%s (%s)", sr.LocalName(), sr.Address)
		}

		seen[sr.Address.String()] = struct{}{}

		if sr.LocalName() != DeviceName {
			return
		}

		logEvent(e, "found device")

		result = sr
		_ = adapter.StopScan()
	})

	if err != nil {
		logEvent(e, "scanning failed: %s", err)
	}

	var dev bluetooth.Device
	logEvent(e, "attempting to connect")
	dev, err = adapter.Connect(result.Address, bluetooth.ConnectionParams{})
	if err != nil {
		logEvent(e, "failed to connect to device: %w", err)
	}

	logEvent(e, "connected")

	deviceLoop(e, dev)
}

func deviceLoop(e chan<- Event, dev bluetooth.Device) {
	svcUUID, err := bluetooth.ParseUUID(ServiceUUID)
	if err != nil {
		logEvent(e, "failed to parse service uuid: %s", err)
		return
	}

	svcs, err := dev.DiscoverServices([]bluetooth.UUID{svcUUID})
	if err != nil {
		logEvent(e, "failed to discover services: %s", err)
		return
	}

	charUUID, err := bluetooth.ParseUUID(CharacteristicUUID)
	if err != nil {
		logEvent(e, "failed to parse service uuid: %s", err)
		return
	}

	chars, err := svcs[0].DiscoverCharacteristics([]bluetooth.UUID{charUUID})
	if err != nil {
		logEvent(e, "failed to list characteristics: %s", err)
		return
	}

	if len(chars) != 1 {
		logEvent(e, "did not find any characteristics")
		return
	}

	chars[0].EnableNotifications(func(buf []byte) {
		e <- Event{
			Button:     int8(buf[0]),
			ButtonDown: buf[1] == 1,
			Angle:      int16(binary.BigEndian.Uint16(buf[2:])),
		}
	})
}
