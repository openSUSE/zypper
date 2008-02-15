#ifndef OUT_H_
#define OUT_H_

#include <string>

#include "zypp/base/NonCopyable.h"
#include "zypp/base/Exception.h"

/**
 * 
 * - Logger (DBG, MIL, ...) must be in place
 * - out.XX wherever output is needed
 * 
 */
class Out : private zypp::base::NonCopyable
{
public:
  typedef enum
  {
    QUIET  = 0,
    NORMAL = 1,
    HIGH   = 2,
    DEBUG  = 3
  } Verbosity;

  typedef enum
  {
    TYPE_NORMAL = 1,
    TYPE_XML    = 2,
    TYPE_ALL    = 0xff
  } Type;

public:
  Out(Verbosity verbosity = NORMAL) : _verbosity(verbosity) {}
  virtual ~Out();

public:
  virtual void info(const std::string & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL) = 0;
  virtual void warning(const std::string & msg, Verbosity verbosity = NORMAL, Type mask = TYPE_ALL) = 0;
  virtual void error(const std::string & problem_desc, const std::string & hint = "") = 0;
  virtual void error(const zypp::Exception & e,
                     const std::string & problem_desc,
                     const std::string & hint = "") = 0;
  // virtual void table()
  // virtual void text()

  // progress
  virtual void progressStart() = 0;
  virtual void progress() = 0;
  virtual void progressEnd() = 0;

  // progress with download rate
  virtual void dwnldProgressStart() = 0;
  virtual void dwnldProgress() = 0;
  virtual void dwnldProgressEnd() = 0;

public:
  /** Get current verbosity. */
  Verbosity verbosity() { return _verbosity; }

  /** Set current verbosity. */
  void setVerbosity(Verbosity verbosity) { _verbosity = verbosity; }

  /** Return the type of the instance. */
  Type type() { return _type; }

protected:
  /**
   * Determine whether the output is intended for the particular type.
   */
  virtual bool mine(Type type) = 0;

private:
  Verbosity _verbosity;
  Type      _type;
};

#endif /*OUT_H_*/
