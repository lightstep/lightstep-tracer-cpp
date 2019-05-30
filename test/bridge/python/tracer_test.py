import sys
import unittest

sys.path.append('bridge/python')
import lightstep

class TestTracer(unittest.TestCase):
    def test_report_span(self):
        tracer = lightstep.Tracer(access_token='abc',
                                  use_stream_recorder=True,
                                  collector_plaintext=True,
                                  satellite_endpoints=[{'host':'locahost', 'port':123}])


if __name__ == '__main__':
    unittest.main()
