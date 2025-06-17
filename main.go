package main

// #cgo LDFLAGS: -L. -lpacketProcessor
// #include "packetProcessor.h"
import "C"

import (
	"database/sql"
	"fmt"
	"log"
	"net/http"

	"github.com/gin-gonic/gin"
	_ "github.com/mattn/go-sqlite3"
)

type Record struct {
	ID         int    `json:"id"`
	SourceIP   string `json:"source_ip"`
	DestIP     string `json:"dest_ip"`
	SourceMAC  string `json:"source_mac"`
	DestMAC    string `json:"dest_mac"`
	SourcePort string `json:"source_port"`
	DestPort   string `json:"dest_port"`
	Protocol   string `json:"protocol"`
	Payload    string `json:"payload"`
}

func main() {
	go C.initMain() // start packet logging in background

	dbPath := "packet_log.db"
	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		log.Fatal("Failed to open DB:", err)
	}
	defer db.Close()

	r := gin.Default()

	// Serve static files
	r.Static("/static", "./static")
	r.GET("/", func(c *gin.Context) {
		c.File("./static/index.html")
	})

	// API endpoint to get records
	r.GET("/data", func(c *gin.Context) {
		limit := 50
		offset := 0

		if l := c.Query("limit"); l != "" {
			fmt.Sscanf(l, "%d", &limit)
		}
		if o := c.Query("offset"); o != "" {
			fmt.Sscanf(o, "%d", &offset)
		}

		query := `SELECT id, source_ip, dest_ip, source_mac, dest_mac,
	source_port, dest_port, protocol, payload FROM records
	ORDER BY id DESC LIMIT ? OFFSET ?`

		rows, err := db.Query(query, limit, offset)

		if err != nil {
			log.Println("DB error:", err)
			c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
			return
		}
		defer rows.Close()

		results := make([]Record, 0) // ensures [] even if no rows
		for rows.Next() {
			var r Record
			if err := rows.Scan(&r.ID, &r.SourceIP, &r.DestIP, &r.SourceMAC, &r.DestMAC,
				&r.SourcePort, &r.DestPort, &r.Protocol, &r.Payload); err != nil {
				log.Println("Row scan error:", err)
				continue
			}
			results = append(results, r)
		}
		fmt.Println(len(results))
		// always returns [] or populated array, never null
		c.JSON(http.StatusOK, results)
	})

	log.Println("Server running at http://localhost:8080")
	r.Run(":8080")
}
