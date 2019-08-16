import sys
import os
import unittest

for pyversion in os.listdir('bridge/python/binary'):
    sys.path.append('bridge/python/binary/' + pyversion)
import lightstep_native

class TestTracer(unittest.TestCase):
    def test_error_on_bad_config(self):
        with self.assertRaises(Exception):
            lightstep_native.Tracer()

        with self.assertRaises(Exception):
            lightstep_native.Tracer('abc')

        with self.assertRaises(Exception):
            lightstep_native.Tracer(xyz='abc')

    def test_report_span(self):
        tracer = lightstep_native.Tracer(access_token='abc',
                                         use_stream_recorder=True,
                                         collector_plaintext=True,
                                         satellite_endpoints=[{'host':'locahost', 'port':123}])


if __name__ == '__main__':
    unittest.main()
