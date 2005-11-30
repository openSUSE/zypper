/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMGroupImpl.h
 *
*/
#ifndef ZYPP_SOURCE_YUM_YUMGROUPIMPL_H
#define ZYPP_SOURCE_YUM_YUMGROUPIMPL_H

#include "zypp/detail/SelectionImplIf.h"
#include "zypp/parser/yum/YUMParserData.h"
#include "zypp/Edition.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace yum
    { //////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : YUMGroupImpl
      //
      /** Class representing a message
      */
      class YUMGroupImpl : public detail::SelectionImplIf
      {
      public:
        class GroupReq
        {
        public:
          GroupReq(const std::string & type, const std::string & name)
          : _type(type)
          , _name(name)
          {}
          std::string type() const { return _type; }
          std::string name() const { return _name; }
        private:
          std::string _type;
          std::string _name;
        };
        class PkgReq
        {
        public:
          PkgReq(const std::string & type, const std::string & name,
                 const std::string & ver, const std::string & rel,
                 const std::string & epoch)
          : _type(type)
          , _name(name)
          , _edition(ver, rel, epoch)
          {}
          std::string type() const { return _type; }
          std::string name() const { return _name; }
          Edition edition() const { return _edition; }
        private:
          std::string _type;
          std::string _name;
          Edition _edition;
        };
        /** Default ctor */
        YUMGroupImpl( const zypp::parser::yum::YUMGroupData & parsed );
	/** Is to be visible for user? */
	virtual bool userVisible() const;
        /** Other requested groups */
	virtual std::list<GroupReq> groupsReq() const;
	/** Requested packages */
	virtual std::list<PkgReq> pkgsReq() const;
	/** */
	virtual Label summary() const;
	/** */
	virtual Text description() const;
	/** */
	virtual Text insnotify() const;
	/** */
	virtual Text delnotify() const;
	/** */
	virtual bool providesSources() const;
	/** */
	virtual Label instSrcLabel() const;
	/** */
	virtual Vendor instSrcVendor() const;
        /** */
        virtual FSize size() const;


      protected:
// _summary
// _description;
        bool _user_visible;
	std::list<GroupReq> _groups_req;
	std::list<PkgReq> _pkgs_req;
      };
      ///////////////////////////////////////////////////////////////////
    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_YUM_YUMGROUPIMPL_H
