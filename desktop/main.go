package main

import (
	"encoding/binary"
	"log"

	"tinygo.org/x/bluetooth"
)

const (
	DeviceName = "Knoblet"

	ServiceUUID        = "F28EC1C1-7BE9-4776-B063-578239F29BFF"
	CharacteristicUUID = "29389447-CCD7-4E89-B8C6-3437651B5488"
)

var adapter = bluetooth.DefaultAdapter
var events = make(chan Event, 1)

type Event struct {
	Angle      int16
	Button     int8
	ButtonDown bool
}

func main() {
	if err := adapter.Enable(); err != nil {
		log.Fatalf("failed to enable bluetooth adapter: %s", err)
	}

	var result bluetooth.ScanResult
	seen := make(map[string]struct{})

	err := adapter.Scan(func(a *bluetooth.Adapter, sr bluetooth.ScanResult) {
		if _, found := seen[sr.Address.String()]; !found {
			log.Printf("%s (%s)", sr.LocalName(), sr.Address)
		}

		seen[sr.Address.String()] = struct{}{}

		if sr.LocalName() != DeviceName {
			return
		}

		log.Println("found device")

		result = sr
		_ = adapter.StopScan()
	})

	if err != nil {
		log.Printf("scanning failed: %s", err)
	}

	var dev bluetooth.Device
	log.Println("attempting to connect")
	dev, err = adapter.Connect(result.Address, bluetooth.ConnectionParams{})
	if err != nil {
		log.Fatalf("failed to connect to device: %w", err)
	}

	log.Println("connected")

	defer dev.Disconnect()
	deviceLoop(dev)

	for evt := range events {
		log.Print(evt)
	}
}

func deviceLoop(dev bluetooth.Device) {
	svcUUID, err := bluetooth.ParseUUID(ServiceUUID)
	if err != nil {
		log.Fatalf("failed to parse service uuid: %s", err)
	}

	svcs, err := dev.DiscoverServices([]bluetooth.UUID{svcUUID})
	if err != nil {
		log.Fatalf("failed to discover services: %s", err)
	}

	charUUID, err := bluetooth.ParseUUID(CharacteristicUUID)
	if err != nil {
		log.Fatalf("failed to parse service uuid: %s", err)
	}

	chars, err := svcs[0].DiscoverCharacteristics([]bluetooth.UUID{charUUID})
	if err != nil {
		log.Fatalf("failed to list characteristics: %s", err)
	}

	if len(chars) != 1 {
		log.Fatalf("did not find any characteristics")
	}

	chars[0].EnableNotifications(func(buf []byte) {
		events <- Event{
			Button:     int8(buf[0]),
			ButtonDown: buf[1] == 1,
			Angle:      int16(binary.BigEndian.Uint16(buf[2:])),
		}
	})
}
