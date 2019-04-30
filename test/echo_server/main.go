package main

import (
	"bytes"
	"fmt"
	"log"
	"net"
	"net/http"
	"os"
	"strconv"
	"sync"
)

type server struct {
	mutex  sync.Mutex
	buffer bytes.Buffer
}

func (server *server) handleTcpConnection(connection net.Conn) {
	server.mutex.Lock()
	defer server.mutex.Unlock()
	_, err := server.buffer.ReadFrom(connection)
	if err != nil {
		log.Fatalf("Read failed: %s\n", err.Error())
	}
}

func (server *server) serveTcp(port int) {
	listener, err := net.Listen("tcp", fmt.Sprintf("127.0.0.1:%d", port))
	if err != nil {
		log.Fatalf("Listen failed: %s\n", err.Error())
	}
	defer listener.Close()
	for {
		connection, err := listener.Accept()
		if err != nil {
			log.Fatalf("Accept failed: %s\n", err.Error())
		}
		go server.handleTcpConnection(connection)
	}
}

func (server *server) serveHttp(port int) {
	http.Handle("/data", server)
	log.Fatal(http.ListenAndServe(fmt.Sprintf("127.0.0.1:%d", port), nil))
}

func (server *server) ServeHTTP(responseWriter http.ResponseWriter, request *http.Request) {
	server.mutex.Lock()
	defer server.mutex.Unlock()
	_, err := responseWriter.Write(server.buffer.Bytes())
	if err != nil {
		log.Fatalf("Write failed: %s\n", err.Error())
	}
}

func main() {
	if len(os.Args) != 3 {
		log.Fatal("Must provide port")
	}
	httpPort, err := strconv.Atoi(os.Args[1])
	if err != nil {
		log.Fatalf("Failed to parse httpPort: %s\n", err.Error())
	}
	tcpPort, err := strconv.Atoi(os.Args[2])
	if err != nil {
		log.Fatalf("Failed to parse tcpPort: %s\n", err.Error())
	}
	server := &server{}
	go server.serveHttp(httpPort)
	server.serveTcp(tcpPort)
}
