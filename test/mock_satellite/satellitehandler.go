package main

import (
    "io"
    "bufio"
    "bytes"
    "log"
    "net/http"
    "time"
    "github.com/lightstep/lightstep-tracer-cpp/lightstep-tracer-common"
    "github.com/golang/protobuf/proto"
    "github.com/golang/protobuf/ptypes"
    tspb "github.com/golang/protobuf/ptypes/timestamp"
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

func (handler *SatelliteHandler) ServeHTTP(responseWriter http.ResponseWriter, request *http.Request) {
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

func (handler* SatelliteHandler) serveStreamingHTTP(responseWriter http.ResponseWriter, request *http.Request) {
  response := &collectorpb.ReportResponse{
    ReceiveTimestamp: getCurrentTime(),
  }
  reader := bufio.NewReader(request.Body)
  reportRequest, err := ReadStreamHeader(reader)
  if err != nil {
    log.Fatalf("ReadStreamHeader failed: %s\n", err.Error())
  }
  handler.reportChannel <- reportRequest

  // Nulify InteralMetrics so that we can reuse the report request without reporting metrics multiple times
  reportRequest.InternalMetrics = nil
  reportRequest.Spans = make([]*collectorpb.Span, 1)

  for {
    span, err := ReadSpan(reader)
    if err == io.EOF {
      break
    }
    if err != nil {
      log.Fatalf("ReadSpan failed: %s\n", err.Error())
    }
    reportRequest.Spans[0] = span
    handler.reportChannel <- reportRequest
  }
  sendResponse(responseWriter, response)
}
