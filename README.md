Fibonacci series web service (CXX based)
===================

This is a simple web service which intended
to calculate Fibonacci series.

Requirements
---

* g++-4.9+
* GNU make 4.0+
* Boost v1.55+ (w/ threading, syntactic analysis and
		   arg parsind features.)
* Restbed framework v4.0+ [1]
* libgmp v6.0+

For testing:
* Python 2.7+
* ab tool

Compile
--
  make

Run service
---
  ./fib --help to see usage of the service.

Example:
  ./fib --verbose=2 --cache-size 2000
This will start the service (at localhost, port 8080)
and pre-initialize internal cache with first
2000 Fibonacci numbers. This also will enable debug
output to the console.

Testing
---

To run funtional testing:
  make check

To benchmark your server:
  time make bench
See Makefile header for list of availble knobs to
play with performance.

[1] - https://github.com/Corvusoft/restbed
