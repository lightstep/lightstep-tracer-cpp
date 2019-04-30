package main

import (
	"fmt"
	"log"
	"os"
	"strconv"
	"sync/atomic"

	"github.com/miekg/dns"
)

var counter = uint64(0)

func addIpv4Address(m *dns.Msg, name string, ip string) {
	rr, err := dns.NewRR(fmt.Sprintf("%s A %s", name, ip))
	if err != nil {
		log.Fatalf("addIpv4Address failed: %s\n ", err.Error())
	}
	m.Answer = append(m.Answer, rr)
}

func handleIpv4Query(m *dns.Msg, name string) {
	switch name {
	case "test.service.":
		addIpv4Address(m, name, "192.168.0.2")
	case "flip.service.":
		count := atomic.LoadUint64(&counter)
		atomic.AddUint64(&counter, 1)
		if count%2 == 0 {
			addIpv4Address(m, name, "192.168.0.2")
		} else {
			addIpv4Address(m, name, "192.168.0.3")
		}
	case "flaky.service.":
		count := atomic.LoadUint64(&counter)
		atomic.AddUint64(&counter, 1)
		if count%2 == 0 {
			return
		} else {
			addIpv4Address(m, name, "192.168.0.1")
		}
	case "satellites.service.":
		addIpv4Address(m, name, "192.168.0.1")
		addIpv4Address(m, name, "192.168.0.2")
	}
}

func parseQuery(m *dns.Msg) {
	for _, q := range m.Question {
		switch q.Qtype {
		case dns.TypeA:
			log.Printf("Query for %s\n", q.Name)
			handleIpv4Query(m, q.Name)
		case dns.TypeAAAA:
			log.Printf("Query for %s\n", q.Name)
			if q.Name != "ipv6.service." {
				return
			}
			rr, err := dns.NewRR(fmt.Sprintf("%s AAAA %s", q.Name, "2001:0db8:85a3:0000:0000:8a2e:0370:7334"))
			if err == nil {
				m.Answer = append(m.Answer, rr)
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
