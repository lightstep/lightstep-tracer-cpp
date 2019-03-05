package main

import (
	"github.com/lightstep/lightstep-tracer-cpp/lightstep-tracer-common"
	"github.com/lightstep/lightstep-tracer-cpp/test/mock_satellite"
	"sync"
)

type ReportProcessor struct {
	reportChannel chan *collectorpb.ReportRequest
	done          chan bool
	mutex         sync.Mutex
	reports       []*collectorpb.ReportRequest
}

func NewReportProcessor(reportChannel chan *collectorpb.ReportRequest) *ReportProcessor {
	processor := &ReportProcessor{
		reportChannel: reportChannel,
		done:          make(chan bool),
	}
	go processor.processReports()
	return processor
}

func (processor *ReportProcessor) processReports() {
	for report := range processor.reportChannel {
		processor.addReport(report)
	}
	processor.done <- true
}

func (processor *ReportProcessor) addReport(report *collectorpb.ReportRequest) {
	processor.mutex.Lock()
	defer processor.mutex.Unlock()
	processor.reports = append(processor.reports, report)
}

func (processor *ReportProcessor) Reports() *mocksatellitepb.Reports {
	result := &mocksatellitepb.Reports{}
	processor.mutex.Lock()
	defer processor.mutex.Unlock()
	result.Reports = make([]*collectorpb.ReportRequest, len(processor.reports))
	copy(result.Reports, processor.reports)
	return result
}

func (processor *ReportProcessor) Spans() *mocksatellitepb.Spans {
	result := &mocksatellitepb.Spans{}
	processor.mutex.Lock()
	defer processor.mutex.Unlock()
	numSpans := 0
	for _, report := range processor.reports {
		numSpans += len(report.Spans)
	}
	result.Spans = make([]*collectorpb.Span, numSpans)
	spanCount := 0
	for _, report := range processor.reports {
		copy(result.Spans[spanCount:], report.Spans)
		spanCount += len(report.Spans)
	}
	return result
}

func (processor *ReportProcessor) Close() {
	<-processor.done
}
