#ifndef __LOGGER_H_INCLUDED__
#define __LOGGER_H_INCLUDED__

#include <string>
#include <sstream>

#define LOGD Log().Get(logDEBUG)
#define LOGI Log().Get(logINFO)


/* Simple logging infra.  */
enum LogLevel {logERROR,
	       logWARNING,
	       logINFO,
	       logDEBUG};
class Log
{
 public:
  Log () {}
  virtual ~Log ();
  std::ostringstream& Get (LogLevel level = logINFO);
 public:
  static LogLevel reportingLevel;
 protected:
  std::ostringstream os;
 private:
  Log (const Log&);
  Log& operator = (const Log&);
 private:
  LogLevel messageLevel;
};

std::ostringstream&
Log::Get (LogLevel level)
{
  messageLevel = level;
  return os;
}

Log::~Log ()
{
  if (messageLevel <= reportingLevel)
    {
      os << std::endl;
      fprintf (stderr, "%s", os.str ().c_str ());
      fflush (stderr);
    }
}

#endif // __LOGGER_H_INCLUDED__
