/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                         (C) SuSE Linux Products GmbH |
\----------------------------------------------------------------------/

  File:       TmpPath.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Provide temporary files/directories, automaticaly
           deleted when no longer needed.

/-*/
#ifndef TmpPath_h
#define TmpPath_h

#include <iosfwd>

#include <y2util/Rep.h>
#include <y2util/Pathname.h>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TmpPath
/**
 * @short Automaticaly deletes files or directories when no longer needed.
 *
 * TmpPath is constructed from a Pathname. Multiple TmpPath instances created
 * by copy and assign, share the same reference counted internal repesentation.

 * When the last reference drops any file or directory located at the path
 * passed to the ctor is deleted (recursivly in case of directories).
 *
 * Principally serves as base class, but standalone usable.
 **/
class TmpPath
{
  public:
    /**
     * Default Ctor. An empty Pathname.
     **/
    TmpPath();

    /**
     * Ctor. Takes a Pathname.
     **/
    explicit
    TmpPath( const Pathname & tmpPath_r );

    /**
     * Dtor.
     **/
    virtual
    ~TmpPath();

    /**
     * Test whether the Pathname is valid (i.e. not empty. NOT whether
     * it realy denotes an existing file or directory).
     **/
    operator const void *const() const;

    /**
     * @return The Pathname.
     **/
    Pathname
    path() const;

    /**
     * Type conversion to Pathname.
     **/
    operator Pathname() const
    { return path(); }

  public:
    /**
     * @return The default directory where temporary
     * files should be are created (/var/tmp).
     **/
    static const Pathname &
    defaultLocation();

  protected:
    class Impl;
    VarPtr<Impl> _impl;
};
///////////////////////////////////////////////////////////////////

/**
 * Stream output as pathname.
 **/
inline std::ostream &
operator<<( std::ostream & str, const TmpPath & obj )
{ return str << static_cast<Pathname>(obj); }

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TmpFile
/**
 * @short Provide a new empty temporary file and delete it when no
 * longer needed.
 *
 * The temporary file is per default created in '/var/tmp' and named
 ' TmpFile.XXXXXX', with XXXXXX replaced by a string which makes the
 * name unique. Different location and file prefix may be passed to
 * the ctor. TmpFile is created with mode 0600.
 *
 * The directory where the temporary file is to be created must exist.
 * TmpFile provides the Pathname of the temporary file, or an empty
 * path in case of any error.
 **/
class TmpFile : public TmpPath
{
  public:
    /**
     * Ctor. Takes a Pathname.
     **/
    explicit
    TmpFile( const Pathname & inParentDir_r = defaultLocation(),
             const std::string & prefix_r = defaultPrefix() );

  public:
    /**
     * @return The default prefix for temporary files (TmpFile.)
     **/
    static const std::string &
    defaultPrefix();

};
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TmpDir
/**
 * @short Provide a new empty temporary directory and recursively
 * delete it when no longer needed.
 *
 * The temporary directory is per default created in '/var/tmp' and
 ' named TmpDir.XXXXXX', with XXXXXX replaced by a string which makes
 * the  name unique. Different location and file prefix may be passed
 * to the ctor. TmpDir is created with mode 0700.
 *
 * The directory where the temporary directory is to be created must exist.
 * TmpDir provides the Pathname of the temporary directory , or an empty
 * path in case of any error.
 **/
class TmpDir : public TmpPath
{
  public:
    /**
     * Ctor. Takes a Pathname.
     **/
    explicit
    TmpDir( const Pathname & inParentDir_r = defaultLocation(),
            const std::string & prefix_r = defaultPrefix() );

  public:
    /**
     * @return The default prefix for temporary directories (TmpDir.)
     **/
    static const std::string &
    defaultPrefix();
};
///////////////////////////////////////////////////////////////////

#endif // TmpPath_h
