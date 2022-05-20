package main

import (
	"log"
	"os"

	"github.com/connyay/wsusscn2.sqlite3/cabfile"
)

func main() {
	path := "wsusscn2.cab"
	f, err := os.Open(path)
	if err != nil {
		log.Fatalf("failed opening %q %v", path, err)
	}
	cb := func(name string, data []byte) error {
		log.Printf("extract %q len=%d", name, len(data))
		return nil
	}
	err = cabfile.Extract(f, cb)
	if err != nil {
		log.Fatalf("failed extraction %v", err)
	}
}
