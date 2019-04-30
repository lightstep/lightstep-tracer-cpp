package main

import (
	"fmt"
	"github.com/golang/protobuf/proto"
	"github.com/lightstep/lightstep-tracer-cpp/lightstep-tracer-common"
	"log"
	"net/http"
	"os"
	"strconv"
)

const (
	maxBufferedReports = 5
)

func main() {
	if len(os.Args) != 2 {
		log.Fatal("Must provide port")
	}
	port, err := strconv.Atoi(os.Args[1])
	if err != nil {
		log.Fatalf("Failed to parse port: %s\n", err.Error())
	}
	reportChannel := make(chan *collectorpb.ReportRequest, maxBufferedReports)
	satelliteHandler := NewSatelliteHandler(reportChannel)
	reportProcessor := NewReportProcessor(reportChannel)
	defer reportProcessor.Close()
	defer close(reportChannel)

	http.Handle("/api/v2/reports", satelliteHandler)
	http.Handle("/report-mock-streaming", satelliteHandler)

	http.HandleFunc("/spans", func(responseWriter http.ResponseWriter, request *http.Request) {
		data, err := proto.Marshal(reportProcessor.Spans())
		if err != nil {
			log.Fatalf("Failed to marshal Spans: %s\n", err.Error())
		}
		responseWriter.Write(data)
	})

	http.HandleFunc("/reports", func(responseWriter http.ResponseWriter, request *http.Request) {
		data, err := proto.Marshal(reportProcessor.Reports())
		if err != nil {
			log.Fatalf("Failed to marshal Reports: %s\n", err.Error())
		}
		responseWriter.Write(data)
	})

	http.HandleFunc("/error-on-next-report", func(responseWriter http.ResponseWriter, request *http.Request) {
		satelliteHandler.SendErrorResponse = true
	})

	log.Fatal(http.ListenAndServe(fmt.Sprintf("127.0.0.1:%d", port), nil))
}
