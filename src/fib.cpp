  /* TODO:
   v 1. cmd line arguments.
   v 2. extract Fib step.
   v 3. code style.
     4. apply tests.
     5. review.  */
#include <sstream>
#include <memory>
#include <cstdlib>
#include <restbed>

/*  Boost stuff.  */
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/gmp.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "logger.h"

#define BOOST_LOG_DYN_LINK 1

using namespace std;
using namespace restbed;
using namespace boost::multiprecision;
namespace po = boost::program_options;

const std::string space = ", ";
string service_name = "fib";
LogLevel Log::reportingLevel;

mpz_int
make_fib_step (mpz_int &prev_1, mpz_int &prev_2)
{
  mpz_int tmp = prev_1;
  prev_1 = tmp + prev_2;
  prev_2 = tmp;
  return prev_1;
}

class fib_cache
{
protected:
  boost::shared_mutex data_mtx;
  string data;
  vector <unsigned> data_idx;

  unsigned limit_cnt;
  unsigned initialized_cnt;
  mpz_int prev_1, prev_2;

public:
  fib_cache () : prev_1 (1),
		 prev_2 (1),
		 initialized_cnt (0),
		 limit_cnt (0) {}

  void
  init (unsigned cnt)
  {
    unsigned est = estimate_fib_str_size (cnt) + 1; /*  +1 is '['.  */
    LOGD << "Estimated cache size for " << cnt << " Fibonacci numbers:" << est << " chars.";
    data.reserve (est);
    data_idx.reserve (cnt);
    limit_cnt = cnt;
    data = "[";
  }

  string& d () { return data; }
  vector<unsigned> di () { return data_idx; }
  unsigned limit () { return limit_cnt; }

  unsigned
  estimate_fib_str_size (unsigned fib_num)
  {
    double lphi = log ((sqrt (5.d) + 1) / 2.d) / log (10); /* LOG (Golden ratio). */

    LOGD << static_cast<double> ((fib_num * (fib_num + 1)) / 2) * lphi;

    return static_cast <unsigned> (static_cast<double> ((fib_num * (fib_num + 1)) / 2) * lphi)
	   + fib_num + fib_num * space.size ();
  }

  void
  add_entry (mpz_int e)
  {
    data += static_cast<ostringstream*> (&(ostringstream () << e << space))->str ();
    data_idx.push_back (data.size () - space.size ());

    ++initialized_cnt;
    LOGD << "Add entry.  New idx: " << data_idx [data_idx.size () - 1];
  }

  void
  read_cur_vals (unsigned &ic, mpz_int &p1, mpz_int &p2)
  {
    boost::shared_lock<boost::shared_mutex> lock (data_mtx);
    ic = initialized_cnt;
    p1 = prev_1;
    p2 = prev_2;
  }

  unsigned
  maybe_calc_fib (unsigned num, mpz_int &p1, mpz_int &p2)
  {
    unsigned ic;

    read_cur_vals (ic, p1, p2);

    /*  As far as we're realloc-free, it should be safe
	to read from data*.  */
    if (num <= ic)
      return num;

    if (ic == limit_cnt)
      return limit_cnt;

    boost::upgrade_lock<boost::shared_mutex> lock (data_mtx);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock (lock);

    unsigned limit = num > limit_cnt ? limit_cnt : num;

    if (initialized_cnt == 0 && limit > 0)
	add_entry (prev_1);

    if (initialized_cnt == 1 && limit > 1)
	add_entry (prev_2);

    for (int i = initialized_cnt;i < limit; ++i)
      add_entry (make_fib_step (prev_1, prev_2));

    return limit;
  }
};

class fib_cache the_cache;

void
get_method_handler (const shared_ptr<Session> session)
{
  LOGD << session->get_request ()->get_path ();

  string num_str = session->get_request ()->get_path ();
  int fib_num;

  num_str = num_str.substr (service_name.size (), num_str.size ());

  LOGD << "Request is: " << num_str;

  try
    {
      fib_num = boost::lexical_cast<int> (num_str);
    }
  catch (const boost::bad_lexical_cast &)
    {
      session->close (BAD_REQUEST, "[]", { { "Content-Length", "2" }, { "Connection", "close" } });
      return;
    }

  if (fib_num == 0)
    {
      session->close (OK, "[]", { { "Content-Length", "2" }, { "Connection", "close" } });
      return;
    }

  if (fib_num <= 0)
    {
      session->close (BAD_REQUEST, "[]", { { "Content-Length", "2" }, { "Connection", "close" } });
      return;
    }

  LOGD << "Parsed number is " << fib_num;

  mpz_int prev_1;
  mpz_int prev_2;
  if (the_cache.maybe_calc_fib (fib_num, prev_1, prev_2) == fib_num)
    {
      LOGD << "Result was fully cached.";

      LOGD << "Idx is: " << the_cache.di ()[fib_num - 1];
      session->close (OK, the_cache.d ().substr (0, the_cache.di ()[fib_num - 1]) + "]",
		      {{ "Content-Length", to_string (the_cache.di ()[fib_num - 1] + 1)},
		       { "Connection", "close" }});
      return;
    }
  else
    {
      LOGD << "Result wasn't fully cached.  Need to calc " << fib_num - the_cache.limit () << " extra numbers.";

      unsigned est = the_cache.estimate_fib_str_size (fib_num) + 1; /*  +2 is '[' and ']'.  */
      LOGD << "Estimated size for " << fib_num << " Fibonacci numbers:" << est << " chars.";
      string res;
      res.reserve (est);
      res = the_cache.d ().substr (0, the_cache.d ().size () - space.size ());

      unsigned start = the_cache.limit ();

      if (the_cache.limit () == 0 && fib_num > 0)
	{
	  res += "1";
	  ++start;
	}

      if (the_cache.limit () <= 1 && fib_num > 1)
	{
	  res += space + "1";
	  ++start;
	}

      for (int i = start;i < fib_num; ++i)
	  res += static_cast<ostringstream*> (&(ostringstream ()
						<< space
						<< make_fib_step (prev_1, prev_2)))->str ();

      res += "]";
      session->close (OK, res,
		      {{ "Content-Length", to_string (res.size ())},
		       { "Connection", "close" }});
      return;
    }
}

int
main (const int ac, const char** av)
{
  int verbosity;
  int num_thr;
  int cache_size;
  int port;

  /* Declare the supported options.  */
  po::options_description desc ("Fibonacci web service usage");
  desc.add_options ()
    ("help", "produce help message")
    ("verbose", po::value<int> (&verbosity)->default_value (1),  "Set verbosity: 0, 1 or 2 (max)")
    ("num-threads", po::value<int> (&num_thr)->default_value (4), "Set number of threads.")
    ("cache-size", po::value<int> (&cache_size)->default_value (1000), "Set max Fibonacci sequence to cache.")
    ("port", po::value<int> (&port)->default_value (8080), "Port to listen to.")
    ("service-name", po::value<string> (&service_name)->default_value ("fib"), "Service name.");

  po::variables_map vm;
  po::store (po::parse_command_line (ac, av, desc), vm);
  po::notify (vm);

  if (vm.count ("help"))
    {
      std::cout << desc << "\n";
      return 1;
    }

  if (service_name[0] != '/')
    service_name = string ("/") + service_name;

  if (service_name[service_name.size () - 1] != '/')
    service_name += '/';

  Log::reportingLevel = logINFO;
  switch (verbosity)
    {
    case 0: Log::reportingLevel = logWARNING; break;
    case 1: Log::reportingLevel = logINFO; break;
    case 2: Log::reportingLevel = logDEBUG; break;
    }

  LOGI << "Verbosity is: " << verbosity;
  LOGI << "Threads num is:" << num_thr;
  LOGI << "Fib sequence length cached is:" << cache_size;
  LOGI << "Service will run @ " << service_name << ", port " << port;

  the_cache.init (cache_size);

  auto resource = make_shared<Resource> ();
  resource->set_path (service_name + "/.*");
  resource->set_method_handler ("GET", get_method_handler);

  auto settings = make_shared<Settings> ();
  settings->set_port (port);
  settings->set_worker_limit (num_thr);

  auto service = make_shared<Service> ();
  service->publish (resource);
  service->start (settings);

  return 0;
}
