package main

import (
    "fmt"
    "log"
    "net/http"
    "os"
    "strconv"
)

func handler(w http.ResponseWriter, r *http.Request) {
    fmt.Fprintf(w, "Hi there, I love %s!", r.URL.Path[1:])
}

func main() {
	if len(os.Args) != 2 {
		log.Fatal("Must provide port")
	}
	port, err := strconv.Atoi(os.Args[1])
	if err != nil {
		log.Fatalf("Failed to parse port: %s\n", err.Error())
	}
  http.HandleFunc("/", handler)
  log.Fatal(http.ListenAndServe(fmt.Sprintf("localhost:%d", port), nil))
}
