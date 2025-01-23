package main

// #cgo LDFLAGS: -L. -lpacketProcessor
// #include "packetProcessor.h"
import (
	"C"
)

import (
	"database/sql"
	"fmt"
	"log"
	"time"

	_ "github.com/mattn/go-sqlite3"
)

func main() {
	go C.initMain()

	dbPath := "packet_log.db"

	ticker := time.NewTicker(1 * time.Second)
	defer ticker.Stop()

	for range ticker.C {
		printLiveStats(dbPath)
	}
}

func printLiveStats(dbPath string) {
	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	var uniqueSourceIPs int
	err = db.QueryRow("SELECT COUNT(DISTINCT source_ip) FROM records").Scan(&uniqueSourceIPs)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("Number of unique source IPs: %d\n", uniqueSourceIPs)

	var uniqueDestIPs int
	err = db.QueryRow("SELECT COUNT(DISTINCT dest_ip) FROM records").Scan(&uniqueDestIPs)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("Number of unique destination IPs: %d\n", uniqueDestIPs)

	var uniqueSourcePorts int
	err = db.QueryRow("SELECT COUNT(DISTINCT source_port) FROM records").Scan(&uniqueSourcePorts)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("Number of unique source ports: %d\n", uniqueSourcePorts)

	var uniqueDestPorts int
	err = db.QueryRow("SELECT COUNT(DISTINCT dest_port) FROM records").Scan(&uniqueDestPorts)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("Number of unique destination ports: %d\n", uniqueDestPorts)

	var sourceIPMostPackets string
	var packetsSent int
	err = db.QueryRow("SELECT source_ip, COUNT(*) as packet_count FROM records GROUP BY source_ip ORDER BY packet_count DESC LIMIT 1").Scan(&sourceIPMostPackets, &packetsSent)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("Source IP with most packets sent: %s (%d packets)\n", sourceIPMostPackets, packetsSent)

	var destIPMostPackets string
	var packetsReceived int
	err = db.QueryRow("SELECT dest_ip, COUNT(*) as packet_count FROM records GROUP BY dest_ip ORDER BY packet_count DESC LIMIT 1").Scan(&destIPMostPackets, &packetsReceived)
	if err != nil {
		log.Fatal(err)
	}
	fmt.Printf("Destination IP with most packets received: %s (%d packets)\n", destIPMostPackets, packetsReceived)
}
