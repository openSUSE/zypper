#include <db_cxx.h>

#include "zypp/Patch.h"
#include "zypp/Edition.h"

#define DEBUG(X) std::cout << X << std::endl

class MyDb
{
public:
  // Constructor requires a path to the database,
  // and a database name.
  MyDb(std::string &path, std::string &dbName);
  // Our destructor just calls our private close method.
  ~MyDb() { close(); }
  inline Db &getDb() {return db_;}
private:
  Db db_;
  std::string dbFileName_;
  u_int32_t cFlags_;
  // Make sure the default constructor is private
  // We don't want it used.
  //MyDb() : db_(NULL, 0) {}
  // We put our database close activity here.
  // This is called from our destructor. In
  // a more complicated example, we might want
  // to make this method public, but a private
  // method is more appropriate for this example.
  void close();
}; 

// Class constructor. Requires a path to the location
// where the database is located, and a database name
MyDb::MyDb(std::string &path, std::string &dbName)
    : db_(NULL, 0),               // Instantiate Db object
      dbFileName_(path + dbName), // Database file name
      cFlags_(DB_CREATE)          // If the database doesn't yet exist,
                                  // allow it to be created.
{
  try
  {
    // Redirect debugging information to std::cerr
    db_.set_error_stream(&std::cerr);
    // Open the database
    db_.open(NULL, dbFileName_.c_str(), NULL, DB_BTREE, cFlags_, 0);
  }
  // DbException is not a subclass of std::exception, so we
  // need to catch them both.
  catch(DbException &e)
  {
    std::cerr << "Error opening database: " << dbFileName_ << "\n";
    std::cerr << e.what() << std::endl;
  }
  catch(std::exception &e)
  {
    std::cerr << "Error opening database: " << dbFileName_ << "\n";
    std::cerr << e.what() << std::endl;
  }
}

// Private member used to close a database. Called from the class
// destructor.
void
MyDb::close()
{
  // Close the db
  try
  {
    db_.close(0);
    std::cout << "Database " << dbFileName_
              << " is closed." << std::endl;
  }
  catch(DbException &e)
  {
    std::cerr << "Error closing database: " << dbFileName_ << "\n";
    std::cerr << e.what() << std::endl;
  }
  catch(std::exception &e)
  {
    std::cerr << "Error closing database: " << dbFileName_ << "\n";
    std::cerr << e.what() << std::endl;
  }
}


class PatchStorage
{
	public:
	
	PatchStorage()
	{
		m_db = new Db(NULL, 0);
		// Set up error handling for this database
    //m_db->set_errcall(&PatchStorage::errorHandler);
    m_db->set_errpfx("PatchStorage"); 
		// flags http://www.sleepycat.com/docs/gsg/CXX/DBOpenFlags.html
		
	}

	void
	openDatabase()
	{
		u_int32_t oFlags = DB_CREATE; // Open flags;
		try
		{
			// Open the database
			// Transaction pointer, Database file name, Optional logical database name, Database access method, Open flags, File mode (using defaults)
			m_db->open(NULL, "my_db.db", NULL, DB_BTREE, oFlags, 0);
			
			// DbException is not subclassed from std::exception, so
			// need to catch both of these.
		}
		catch(DbException &e)
		{
			// Error handling code goes here    
			//db->err(e.get_errno(), "Database open failed %s", 
			//dbFileName.c_str());
			throw e;
		}
		catch(std::exception &e)
		{
			// Error handling code goes here
			// No DB error number available, so use errx
			m_db->errx("Error opening database: %s", e.what());
			throw e;
		}
	}

	void
	closeDatabase()
	{
		// close the database
		try
		{
			// Close the database
			m_db->close(0);
			// DbException is not subclassed from std::exception, so
			// need to catch both of these.
		}
		catch(DbException &e)
		{
			// Error handling code goes here    
		}
		catch(std::exception &e)
		{
				// Error handling code goes here
		}
	}

	~PatchStorage()
	{
		
	}

	void
	errorHandler(const char *error_prefix, char *msg)
	{
		/*
		 * Put your code to handle the error prefix and error
		 * message here. Note that one or both of these parameters
		 * may be NULL depending on how the error message is issued
		 * and how the DB handle is configured.
		 */
	}

	private:
	Db *m_db;
};

int main()
{
	PatchStorage ps;
	ps.openDatabase();
	return 0;
}

/*

*/
