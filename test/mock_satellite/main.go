package main

import (
	"fmt"
	"io"
	"log"
	"net"
	"os"
	"strconv"
)

func handleConnection(connection net.Conn) {
	defer connection.Close()
	buffer := make([]byte, 512)
	var err error
	for {
		_, err = connection.Read(buffer)
		if err != nil {
			break
		}
	}
	if err != io.EOF {
		log.Fatalf("Read failed: %s\n", err.Error())
	}
}

func main() {
	if len(os.Args) != 2 {
		log.Fatal("Must provide port")
	}
	port, err := strconv.Atoi(os.Args[1])
	if err != nil {
		log.Fatalf("Failed to parse port: %s\n", err.Error())
	}

	listener, err := net.Listen("tcp", fmt.Sprintf("localhost:%d", port))
	if err != nil {
		log.Fatalf("Listen failed: %s\n ", err.Error())
	}
	defer listener.Close()
	log.Printf("Starting at %d\n", port)
	for {
		connection, err := listener.Accept()
		if err != nil {
			log.Fatalf("Accept failed: %s\n", err.Error())
		}
		go handleConnection(connection)
	}
}
