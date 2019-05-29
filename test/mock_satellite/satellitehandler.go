package main

import (
	"bufio"
	"github.com/golang/protobuf/proto"
	"github.com/golang/protobuf/ptypes"
	tspb "github.com/golang/protobuf/ptypes/timestamp"
	"github.com/lightstep/lightstep-tracer-cpp/lightstep-tracer-common"
	"io"
	"log"
	"net/http"
	"sync/atomic"
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
	reportChannel     chan *collectorpb.ReportRequest
	SendErrorResponse int32
	Timeout           int32
	Throttle          int32
}

func NewSatelliteHandler(reportChannel chan *collectorpb.ReportRequest) *SatelliteHandler {
	return &SatelliteHandler{
		reportChannel:     reportChannel,
		SendErrorResponse: 0,
		Timeout:           0,
		Throttle:          0,
	}
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

func (handler *SatelliteHandler) sendResponse(responseWriter http.ResponseWriter, response *collectorpb.ReportResponse) {
	if atomic.SwapInt32(&handler.SendErrorResponse, 0) == 1 {
		http.Error(responseWriter, http.StatusText(http.StatusNotFound), http.StatusNotFound)
		return
	}
	if atomic.SwapInt32(&handler.Timeout, 0) == 1 {
		time.Sleep(time.Hour)
	}
	response.TransmitTimestamp = getCurrentTime()
	serializedResponse, err := proto.Marshal(response)
	if err != nil {
		log.Fatalf("Failed to marshal ReportResponse: %s\n", err.Error())
	}
	responseWriter.Write(serializedResponse)
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
	if request.ContentLength < 0 {
		log.Fatalf("Request has unexpected content length %s\n", request.ContentLength)
	}
	report := &collectorpb.ReportRequest{}
	buffer := make([]byte, request.ContentLength)
	_, err := io.ReadFull(request.Body, buffer)
	if err != nil {
		log.Fatalf("Failed to read request body: %s\n", err.Error())
	}
	err = proto.Unmarshal(buffer, report)
	if err != nil {
		log.Fatalf("Failed to unmarshal ReportRequest: %s\n", err.Error())
	}
	handler.sendResponse(responseWriter, response)
	handler.reportChannel <- report
}

func (handler *SatelliteHandler) serveStreamingHTTP(responseWriter http.ResponseWriter, request *http.Request) {
	log.Println("serving streaming http")
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
		if atomic.LoadInt32(&handler.Throttle) == 1 {
			time.Sleep(5 * time.Millisecond)
		}
		_, err := reader.ReadByte()
		if err == io.EOF || err == io.ErrUnexpectedEOF {
			break
		}
		if err != nil {
			log.Fatalf("ReadByte failed: %s\n", err.Error())
			break
		}
		err = reader.UnreadByte()
		if err != nil {
			log.Fatalf("UnreadByte failed: %s\n", err.Error())
			break
		}
		span, err := ReadSpan(reader)
		if err != nil {
			log.Fatalf("ReadSpan failed: %s\n", err.Error())
		}
		log.Println("Reporting a span")
		report := &collectorpb.ReportRequest{
			Reporter: reportHeader.Reporter,
			Auth:     reportHeader.Auth,
			Spans:    make([]*collectorpb.Span, 1),
		}
		report.Spans[0] = span
		handler.reportChannel <- report
	}
	handler.sendResponse(responseWriter, response)
}
