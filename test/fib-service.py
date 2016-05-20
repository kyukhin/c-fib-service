import unittest
import urllib2
import json

class fib_service (unittest.TestCase):
    def get (self, param):
        url = "http://localhost:1983/%s" % (param)
	return json.loads (urllib2.urlopen (url).read ())

    def test_0 (self):
	self.assertEqual (self.get ("fib/0"), [])

    def test_1 (self):
	self.assertEqual (self.get ("fib/1"), [1])

    def test_2 (self):
	self.assertEqual (self.get ("fib/2"), [1, 1])

    def test_3 (self):
	self.assertEqual (self.get ("fib/3"), [1, 1, 2])

    def test_5 (self):
	self.assertEqual (self.get ("fib/5"), [1, 1, 2, 3, 5])

    def test_symbol (self):
        with self.assertRaises (urllib2.HTTPError):
            self.get("fib/a")

    def test_max_neg (self):
        with self.assertRaises (urllib2.HTTPError):
            self.get("fib/-1")

if __name__ == '__main__':
	unittest.main()
