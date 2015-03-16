/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Digest.h
 *
 * \todo replace by Blocxx
 *
*/

#ifndef ZYPP_MEDIA_DIGEST_H
#define ZYPP_MEDIA_DIGEST_H

#include <string>
#include <iosfwd>
#include <vector>

#include "zypp/Callback.h"
#include "zypp/Pathname.h"

namespace zypp {


  struct DigestReport : public callback::ReportBase
  {
    virtual bool askUserToAcceptNoDigest( const zypp::Pathname &file );
    virtual bool askUserToAccepUnknownDigest( const Pathname &file, const std::string &name );
    virtual bool askUserToAcceptWrongDigest( const Pathname &file, const std::string &requested, const std::string &found );
  };



    /** \brief Compute Message Digests (MD5, SHA1 etc)
     *
     * The computation works by initializing the algorithm using create(). This
     * will construct an internal state. successive calls to update() deliver the
     * data for which the digest is to be computed. After all data has been
     * deliverd, a call to digest() finalizes the computation and returns the
     * result
     * */
    class Digest
    {
      private:
    	class P;
    	P* _dp;

    	// disabled
    	Digest(const Digest& d);
    	// disabled
    	const Digest& operator=(const Digest& d);

      public:
	/** \name Well known digest algorithm names. */
	//@{
	/** md5 */
	static const std::string & md5();
	/** sha1 */
	static const std::string & sha1();
	/** sha224 */
	static const std::string & sha224();
	/** sha256 */
	static const std::string & sha256();
	/** sha384 */
	static const std::string & sha384();
	/** sha512 */
	static const std::string & sha512();
	//@}

      public:
    	Digest();
    	~Digest();

    	/** \brief initialize creation of a new message digest
    	 *
    	 * Since openssl is used as backend you may use anything that openssl
    	 * supports (see man 1 dgst). Common examples are md5 or sha1. sha1
    	 * should be preferred when creating digests to verify the authenticity
    	 * of something.
    	 *
    	 * successive calls to this funcion will destroy the internal state and
    	 * reinit from scratch
    	 *
    	 * @param name name of the message digest algorithm.
    	 * @return whether an error occured
    	 * */
    	bool create(const std::string& name);

    	/** \brief get the name of the current digest algorithm */
    	const std::string& name();

    	/** \brief feed data into digest computation algorithm
    	 * @param bytes
    	 * @param len
    	 * @return whether an error occured
    	 * */
    	bool update(const char* bytes, size_t len);

    	/** \brief get hex string representation of the digest
    	 *
    	 * this function will finalize the digest computation. calls to update
    	 * after this function will start from scratch
    	 *
    	 * @return hex string representation of the digest
    	 * */
    	std::string digest();

    	/** \brief get vector of unsigned char representation of the digest
    	 *
    	 * this function will finalize the digest computation. calls to update
    	 * after this function will start from scratch
    	 *
    	 * @return vector representation of the digest
    	 * */
    	std::vector<unsigned char> digestVector();

    	/** \brief reset internal digest state
    	 *
	 * this function is equivalent to calling create() with an unchanged name,
	 * but it may be implemented in a more efficient way.
	 */
	bool reset();

    	/** \brief compute digest of a stream. convenience function
    	 *
    	 * calls create, update and digest in one function. The data for the
    	 * computation is read from the stream
    	 *
    	 * @param name name of the digest algorithm, \see create
    	 * @param is an input stream to get the data from
    	 * @param bufsize size of the buffer used for update(). Be careful, this is on the stack.
    	 * @return the digest or empty on error
    	 * */
    	static std::string digest(const std::string& name, std::istream& is, size_t bufsize = 4096);

	/** \overload Reading input data from \c string. */
    	static std::string digest( const std::string & name, const std::string & input, size_t bufsize = 4096 );
    };

} // namespace zypp

#endif
