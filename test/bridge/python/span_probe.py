import sys
import lightstep_streaming

if len(sys.argv) != 3:
    print("Usage: span_probe.py <host> <port>")
    sys.exit(-1)


host = sys.argv[1]
port = sys.argv[2]

tracer = lightstep_streaming.Tracer(
        component_name = "span_probe",
        access_token = "abc",
        collector_plaintext = True,
        use_stream_recorder = True,
        satellite_endpoints = [{
            'host': host,
            'port': int(port)
        }]
)

span = tracer.start_span('A')
span.finish()

tracer.close()
