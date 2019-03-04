package main

import (
	"bufio"
	"bytes"
	"github.com/golang/protobuf/proto"
	"github.com/golang/protobuf/ptypes"
	tspb "github.com/golang/protobuf/ptypes/timestamp"
	"github.com/lightstep/lightstep-tracer-cpp/lightstep-tracer-common"
	"io"
	"log"
	"net/http"
	"time"
)

func getCurrentTime() *tspb.Timestamp {
	timestamp, err := ptypes.TimestampProto(time.Now())
	if err != nil {
		log.Fatalf("TimestampProto failed: %s\n", err.Error())
	}
	return timestamp
}

type SatelliteHandler struct {
	reportChannel chan *collectorpb.ReportRequest
}

func NewSatelliteHandler(reportChannel chan *collectorpb.ReportRequest) *SatelliteHandler {
	return &SatelliteHandler{
		reportChannel: reportChannel,
	}
}

func sendResponse(responseWriter http.ResponseWriter, response *collectorpb.ReportResponse) {
	response.TransmitTimestamp = getCurrentTime()
	serializedResponse, err := proto.Marshal(response)
	if err != nil {
		log.Fatalf("Failed to marshal ReportResponse: %s\n", err.Error())
	}
	responseWriter.Write(serializedResponse)
}

func isStreamedRequest(request *http.Request) bool {
	if request.URL.String() == "/report-mock-streaming" {
		return true
	}
	if len(request.TransferEncoding) == 0 {
		return false
	}
	return request.TransferEncoding[0] == "chunked"
}

func (handler *SatelliteHandler) ServeHTTP(responseWriter http.ResponseWriter, request *http.Request) {
	if isStreamedRequest(request) {
		handler.serveStreamingHTTP(responseWriter, request)
		return
	}
	handler.serveFixedHTTP(responseWriter, request)
}

func (handler *SatelliteHandler) serveFixedHTTP(responseWriter http.ResponseWriter, request *http.Request) {
	response := &collectorpb.ReportResponse{
		ReceiveTimestamp: getCurrentTime(),
	}
	report := &collectorpb.ReportRequest{}
	buffer := new(bytes.Buffer)
	buffer.ReadFrom(request.Body)
	err := proto.Unmarshal(buffer.Bytes(), report)
	if err != nil {
		log.Fatalf("Failed to unmarshal ReportRequest: %s\n", err.Error())
	}
	sendResponse(responseWriter, response)
	handler.reportChannel <- report
}

func (handler *SatelliteHandler) serveStreamingHTTP(responseWriter http.ResponseWriter, request *http.Request) {
	response := &collectorpb.ReportResponse{
		ReceiveTimestamp: getCurrentTime(),
	}
	reader := bufio.NewReader(request.Body)
	reportHeader, err := ReadStreamHeader(reader)
	if err != nil {
		log.Fatalf("ReadStreamHeader failed: %s\n", err.Error())
	}
	handler.reportChannel <- reportHeader
	for {
		span, err := ReadSpan(reader)
		if err == io.EOF {
			break
		}
		if err != nil {
			log.Fatalf("ReadSpan failed: %s\n", err.Error())
		}
		report := &collectorpb.ReportRequest{
			Reporter: reportHeader.Reporter,
			Auth:     reportHeader.Auth,
			Spans:    make([]*collectorpb.Span, 1),
		}
		report.Spans[0] = span
		handler.reportChannel <- report
	}
	sendResponse(responseWriter, response)
}
