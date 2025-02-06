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
		fmt.Printf("\r%s", prepareLiveStats(dbPath))
	}
}

func prepareLiveStats(dbPath string) string {
	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		log.Printf("Error opening database: %s\n", err)
		return ""
	}
	defer db.Close()

	var liveStat string = "Live Stats:\n"

	queries := []struct {
		query string
		dest  interface{}
		label string
	}{
		{"SELECT COUNT(DISTINCT source_ip) FROM records", new(int), "Number of unique source IPs: %d\n"},
		{"SELECT COUNT(DISTINCT dest_ip) FROM records", new(int), "Number of unique destination IPs: %d\n"},
		{"SELECT COUNT(DISTINCT source_port) FROM records", new(int), "Number of unique source ports: %d\n"},
		{"SELECT COUNT(DISTINCT dest_port) FROM records", new(int), "Number of unique destination ports: %d\n"},
		{"SELECT source_ip, COUNT(*) as packet_count FROM records GROUP BY source_ip ORDER BY packet_count DESC LIMIT 1", new(struct {
			sourceIP    string
			packetCount int
		}), "Source IP with most packets sent: %s (%d packets)\n"},
		{"SELECT dest_ip, COUNT(*) as packet_count FROM records GROUP BY dest_ip ORDER BY packet_count DESC LIMIT 1", new(struct {
			destIP      string
			packetCount int
		}), "Destination IP with most packets received: %s (%d packets)\n"},
	}

	for _, q := range queries {
		err := db.QueryRow(q.query).Scan(q.dest)
		if err != nil {
			log.Printf("Error executing query: %s\n", err)
		}

		switch v := q.dest.(type) {
		case *int:
			liveStat += fmt.Sprintf(q.label, *v)
		case *struct {
			sourceIP    string
			packetCount int
		}:
			liveStat += fmt.Sprintf(q.label, v.sourceIP, v.packetCount)
		case *struct {
			destIP      string
			packetCount int
		}:
			liveStat += fmt.Sprintf(q.label, v.destIP, v.packetCount)
		}
	}

	return liveStat
}
