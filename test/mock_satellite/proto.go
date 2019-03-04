package main

import (
	"bufio"
	"encoding/binary"
	"errors"
	"fmt"
	"github.com/golang/protobuf/proto"
	"github.com/lightstep/lightstep-tracer-cpp/lightstep-tracer-common"
	"io"
)

func computeFieldTypeNumber(field uint64, wireType uint64) uint64 {
	return (field << 3) | wireType
}

func readEmbeddedMessage(reader *bufio.Reader, field uint64, message proto.Message) error {
	fieldType, err := binary.ReadUvarint(reader)
	if err != nil {
		return err
	}
	if expectedFieldType := computeFieldTypeNumber(field, proto.WireBytes); fieldType != expectedFieldType {
		return errors.New(fmt.Sprintf("Unexpected fieldType %d number for Reporter: expected %d", fieldType,
			expectedFieldType))
	}
	length, err := binary.ReadUvarint(reader)
	if err != nil {
		return err
	}
	buffer := make([]byte, length)
	_, err = io.ReadFull(reader, buffer)
	if err != nil {
		return err
	}
	return proto.Unmarshal(buffer, message)
}

func readReporter(reader *bufio.Reader) (*collectorpb.Reporter, error) {
	reporter := &collectorpb.Reporter{}
	return reporter, readEmbeddedMessage(reader, 1, reporter)
}

func readAuth(reader *bufio.Reader) (*collectorpb.Auth, error) {
	auth := &collectorpb.Auth{}
	return auth, readEmbeddedMessage(reader, 2, auth)
}

func readInternalMetrics(reader *bufio.Reader) (*collectorpb.InternalMetrics, error) {
	internalMetrics := &collectorpb.InternalMetrics{}
	return internalMetrics, readEmbeddedMessage(reader, 6, internalMetrics)
}

func ReadStreamHeader(reader *bufio.Reader) (*collectorpb.ReportRequest, error) {
	reporter, err := readReporter(reader)
	if err != nil {
		return nil, err
	}
	auth, err := readAuth(reader)
	if err != nil {
		return nil, err
	}
	internalMetrics, err := readInternalMetrics(reader)
	if err != nil {
		return nil, err
	}
	result := &collectorpb.ReportRequest{
		Reporter:        reporter,
		Auth:            auth,
		InternalMetrics: internalMetrics,
	}
	return result, nil
}

func ReadSpan(reader *bufio.Reader) (*collectorpb.Span, error) {
	span := &collectorpb.Span{}
	return span, readEmbeddedMessage(reader, 3, span)
}
