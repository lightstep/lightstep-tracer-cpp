package main

import (
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
  response.TransmitTimestamp = getCurrentTime()
  serializedResponse, err := proto.Marshal(response)
  if err != nil {
    log.Fatalf("Failed to marshal ReportResponse: %s\n", err.Error())
  }
  responseWriter.Write(serializedResponse)
  handler.reportChannel <- report
}
