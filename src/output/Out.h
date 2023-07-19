/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_OUT_H_
#define ZYPPER_OUT_H_

#include <zypp-tui/output/Out.h>

using ztui::Out;
using ztui::TermLine;
using ztui::PromptOptions;
using ztui::ProgressEnd;

namespace text {
  using ztui::text::join;
  using ztui::text::endsOnWS;
  using ztui::text::optBlankAfter;
  using ztui::text::tagError;
  using ztui::text::tagNote;
  using ztui::text::tagWarning;
  using ztui::text::qContinue;
}

namespace out {
  using ztui::out::CompressedListLayout;
  using ztui::out::DefaultGapedListLayout;
  using ztui::out::DefaultListLayout;
  using ztui::out::IndentedGapedListLayout;
  using ztui::out::IndentedListLayout;
  using ztui::out::ListLayout;
  using ztui::out::XmlListLayout;
  using ztui::out::DefaultTableLayout;
  using ztui::out::TableLayout;
  using ztui::out::asTableHeader;
  using ztui::out::asTableRow;
  using ztui::out::asListElement;
  using ztui::out::asXmlListElement;
  using ztui::out::writeContainer;
  using ztui::out::xmlWriteContainer;
  using ztui::out::TableFormater;
  using ztui::out::ListFormater;
}

#endif // ZYPPER_OUT_H_
