package main

import (
	"fmt"
	"log"
  "os"
	"strconv"

	"github.com/miekg/dns"
)

var records = map[string]string{
	"test.service.": "192.168.0.2",
}

func parseQuery(m *dns.Msg) {
	for _, q := range m.Question {
		switch q.Qtype {
		case dns.TypeA:
			log.Printf("Query for %s\n", q.Name)
			ip := records[q.Name]
			if ip != "" {
				rr, err := dns.NewRR(fmt.Sprintf("%s A %s", q.Name, ip))
				if err == nil {
					m.Answer = append(m.Answer, rr)
				}
			}
		}
	}
}

func handleDnsRequest(w dns.ResponseWriter, r *dns.Msg) {
	m := new(dns.Msg)
	m.SetReply(r)
	m.Compress = false

	switch r.Opcode {
	case dns.OpcodeQuery:
		parseQuery(m)
	}

	w.WriteMsg(m)
}

func main() {
  if len(os.Args) != 2 {
    log.Fatal("Must provide port")
  }
  port, err := strconv.Atoi(os.Args[1])
  if err != nil {
    log.Fatalf("Failed to parse port: %s\n", err.Error())
  }

	// attach request handler func
	dns.HandleFunc("service.", handleDnsRequest)

	// start server
	server := &dns.Server{Addr: ":" + strconv.Itoa(port), Net: "udp"}
	log.Printf("Starting at %d\n", port)
	err = server.ListenAndServe()
	defer server.Shutdown()
	if err != nil {
		log.Fatalf("Failed to start server: %s\n ", err.Error())
	}
}
