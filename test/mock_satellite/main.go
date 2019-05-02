package main

import (
	"fmt"
	"github.com/golang/protobuf/proto"
	"github.com/lightstep/lightstep-tracer-cpp/lightstep-tracer-common"
	"log"
	"net/http"
	"os"
	"strconv"
	"sync/atomic"
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

	server := http.Server{Addr: fmt.Sprintf(":%d", port)}

	reportChannel := make(chan *collectorpb.ReportRequest, maxBufferedReports)
	satelliteHandler := NewSatelliteHandler(reportChannel)
	reportProcessor := NewReportProcessor(reportChannel)
	defer reportProcessor.Close()
	defer close(reportChannel)

	prematureClose := int32(0)
	http.HandleFunc("/api/v2/reports", func(responseWriter http.ResponseWriter, request *http.Request) {
		if atomic.SwapInt32(&prematureClose, 0) == 1 {
			server.Close()
		}
		satelliteHandler.ServeHTTP(responseWriter, request)
	})
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
		atomic.StoreInt32(&satelliteHandler.SendErrorResponse, 1)
	})

	http.HandleFunc("/timeout-on-next-report", func(responseWriter http.ResponseWriter, request *http.Request) {
		atomic.StoreInt32(&satelliteHandler.Timeout, 1)
	})

	http.HandleFunc("/throttle-reports", func(responseWriter http.ResponseWriter, request *http.Request) {
		atomic.StoreInt32(&satelliteHandler.Throttle, 1)
	})

	http.HandleFunc("/premature-close-next-report", func(responseWriter http.ResponseWriter, request *http.Request) {
		atomic.StoreInt32(&prematureClose, 1)
	})

	log.Fatal(server.ListenAndServe())
}
