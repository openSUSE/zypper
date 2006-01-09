/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/tagfile/Grammar.h
 *
*/
#ifndef ZYPP_PARSER_TAGFILE_GRAMMAR_H
#define ZYPP_PARSER_TAGFILE_GRAMMAR_H

#include <iosfwd>

#include "zypp/parser/tagfile/Tags.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace tagfile
    { /////////////////////////////////////////////////////////////////

      typedef scanner<iterator_t>  scanner_t;
      typedef rule<scanner_t>      rule_t;

      ////////////////////////////////////////////////////////////////////////////
      //
      //  Closures
      //
      ////////////////////////////////////////////////////////////////////////////

      struct TagClosure : public spirit::closure<TagClosure, Tag>
      {
        member1 val;
      };
      typedef TagClosure::context_t TagVal;

      struct STagClosure : public spirit::closure<STagClosure, STag>
      {
        member1 val;
      };
      typedef STagClosure::context_t STagVal;

      struct MTagClosure : public spirit::closure<MTagClosure, MTag, std::string>
      {
        member1 val;
        member2 expect;
      };
      typedef MTagClosure::context_t MTagVal;

      ////////////////////////////////////////////////////////////////////////////
      //
      //  SingleTag Grammar
      //
      ////////////////////////////////////////////////////////////////////////////

      struct SingleTag : public grammar<SingleTag, STagVal>
      {
        template <typename ScannerT>
          struct definition
          {
            definition( const SingleTag & self )
            {
              first =
              (
                line
                    = stag                [bind(&STag::stag)(self.val)=arg1]
                      >> *blank_p
                      >> (!data)          [bind(&STag::data)(self.val)=construct_<Range>(arg1,arg2)]
                                          [bind(&STag::value)(self.val)=construct_<std::string>(arg1,arg2)]
                      >> *blank_p
                      >> (eol_p|end_p)
                ,
                stag
                    = ( ch_p('=')
                        >> (+alpha_p)     [bind(&Tag::ident)(stag.val)=construct_<std::string>(arg1,arg2)]
                        >> !( '.'
                              >> (+alpha_p) [bind(&Tag::ext)(stag.val)=construct_<std::string>(arg1,arg2)]
                            )
                        >> ':'
                      )                   [bind(&Tag::range)(stag.val)=construct_<Range>(arg1,arg2)]
                ,
                data
                    = +~space_p
                      >> *( +blank_p
                            >> +~space_p
                          )
              );
            }

            subrule<0>        line;
            subrule<1,TagVal> stag;
            subrule<2>        data;

            rule<ScannerT> first;
            const rule<ScannerT> & start() const
            { return first; }
          };
      };

      ////////////////////////////////////////////////////////////////////////////
      //
      //  MultiTag Grammar
      //
      ////////////////////////////////////////////////////////////////////////////

      struct MultiTag : public grammar<MultiTag, MTagVal>
      {
        template <typename ScannerT>
          struct definition
          {
            definition( const MultiTag & self )
            {
              first =
              (
                line
                    = stag                  [bind(&MTag::stag)(self.val)=arg1]
                      >> *(anychar_p - eol_p)
                      >> eol_p
                      >> data               [bind(&MTag::data)(self.val)=construct_<Range>(arg1,arg2)]
                      >> ( etag             [bind(&MTag::etag)(self.val)=arg1]
                         | error_report_p( "Missing end tag" )
                         )
                      >> *(anychar_p - eol_p)
                      >> (eol_p|end_p)
                ,
                stag
                    = ( ch_p('+')
                        >> ( (+alpha_p)     [bind(&Tag::ident)(stag.val)=construct_<std::string>(arg1,arg2)]
                             >> !( '.'
                                    >> (+alpha_p) [bind(&Tag::ext)(stag.val)=construct_<std::string>(arg1,arg2)]
                                 )
                           )                [self.expect=construct_<std::string>(arg1,arg2)]
                        >> ':'
                      )                     [bind(&Tag::range)(stag.val)=construct_<Range>(arg1,arg2)]
                ,
                data
                    = while_p( ~eps_p(dataend) )
                      [
                        ( !( *blank_p >> +~space_p
                              >> *( +blank_p >> +~space_p )
                           )
                        )                   [push_back(bind(&MTag::value)(self.val),construct_<std::string>(arg1,arg2))]
                        >> *blank_p
                        >> eol_p
                      ]
                ,
                dataend
                    = ( ch_p('-') | ch_p('+') )
                      >> f_str_p(self.expect)
                      >> ':'
                ,
                etag
                    = ( ch_p('-')
                        >> f_str_p(self.expect)
                        >> ':'
                      )                     [bind(&Tag::range)(etag.val)=construct_<Range>(arg1,arg2)]

              );
            }

            subrule<0>        line;
            subrule<1,TagVal> stag;
            subrule<2>        data;
            subrule<3>        dataend;
            subrule<4,TagVal> etag;

            rule<ScannerT> first;
            const rule<ScannerT> & start() const
            { return first; }
          };
      };

      /////////////////////////////////////////////////////////////////
    } // namespace tagfile
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_TAGFILE_GRAMMAR_H
