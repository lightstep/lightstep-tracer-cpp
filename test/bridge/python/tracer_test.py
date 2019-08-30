import sys
import os
import unittest

for pyversion in os.listdir('bridge/python/binary'):
    sys.path.append('bridge/python/binary/' + pyversion)
import lightstep_streaming

class TestTracer(unittest.TestCase):
    def test_error_on_bad_config(self):
        with self.assertRaises(Exception):
            lightstep_streaming.Tracer()

        with self.assertRaises(Exception):
            lightstep_streaming.Tracer('abc')

        with self.assertRaises(Exception):
            lightstep_streaming.Tracer(xyz='abc')

    def test_report_span(self):
        tracer = lightstep_streaming.Tracer(access_token='abc',
                                         use_stream_recorder=True,
                                         collector_plaintext=True,
                                         satellite_endpoints=[{'host':'locahost', 'port':123}])
        self.assertEqual(tracer.num_spans_sent, 0)
        self.assertEqual(tracer.num_spans_dropped, 0)


if __name__ == '__main__':
    unittest.main()
