/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* Strictly for internal use!
*/

#ifndef ZYPP_TUI_OUTPUT_PROMPTOPTIONS_H_INCLUDED
#define ZYPP_TUI_OUTPUT_PROMPTOPTIONS_H_INCLUDED

#include <vector>
#include <string>
#include <set>

#include <zypp-tui/utils/colors.h>

namespace ztui {

  /**
 * Example:
 * <code>
 * PromptOptions popts;
 * popts.setOptions(_("y/n/p"), 0 / * default reply * /);
 * popts.setOptionHelp(0, _("Help for option 'y'"));
 * popts.setOptionHelp(1, _("Help for option 'n'"));
 * ...
 * zypper.out().prompt(PROMPT_YN_INST_REMOVE_CONTINUE, prompt_text, popts);
 * unsigned reply =
 *   get_prompt_reply(zypper, PROMPT_YN_INST_REMOVE_CONTINUE, popts);
 * </code>
 */
  class PromptOptions
  {
  public:
    typedef std::vector<std::string> StrVector;

    /**
   * Default c-tor.
   */
    PromptOptions() {};

    /** Ctor taking the option values as vector */
    PromptOptions( StrVector options_r, unsigned defaultOpt_r );

    /**
   * Constructor.
   *
   * \param optionstr_r  translated option string containing one or more
   *                     options separated by slash '/' character
   *                     e.g. "yes/no/?" or "1/s/r/c"
   * \param defaultOpt_r index of the default answer within the \a option_str
   */
    PromptOptions( const std::string & optionstr_r, unsigned defaultOpt_r );

    /** D-tor */
    ~PromptOptions();

    const StrVector & options() const { return _options; }
    void setOptions( StrVector options_r, unsigned defaultOpt_r );
    void setOptions( const std::string & optionstr_r, unsigned defaultOpt_r );

    unsigned defaultOpt() const { return _default; }
    /** Option string (may have embedded color codes) */
    ColorString optionString() const;
    bool empty() const { return _options.empty(); }
    bool isYesNoPrompt() const;

    const std::string & optionHelp(unsigned opt) const
    { static std::string empty; return opt < _opt_help.size() ? _opt_help[opt] : empty; }
    //const std::string getOptionHelp(const std::string & opt_str);
    void setOptionHelp(unsigned opt, const std::string & help_str);
    bool helpEmpty() const { return _opt_help.empty(); }

    bool isEnabled(unsigned opt) const
    { return !isDisabled(opt); }
    bool isDisabled(unsigned opt) const
    { return _disabled.count(opt); }
    void disable(unsigned opt)
    { _disabled.insert(opt); }
    void enable(unsigned opt)
    { _disabled.erase(opt); }
    void enableAll()
    { _disabled.clear(); }

    unsigned shownCount() const
    { return _shown_count; }
    void setShownCount(unsigned count)
    { _shown_count = count; }

    /** Return the indices of option string matches (lowercase/prefix or #NUM). */
    std::vector<int> getReplyMatches( const std::string & reply_r ) const;
    /** The returned reply matches as '(,)' list. */
    std::string replyMatchesStr( const std::vector<int> & matches_r ) const;

  private:
    /** option strings */
    StrVector _options;
    /** index of the default option */
    unsigned _default = 0;
    /** help strings corresponding to options */
    StrVector _opt_help;
    /** set of options to ignore */
    std::set<unsigned> _disabled;
    /**
   * Number of options to show (the rest will still be available and visible
   * through '?' help). If negative, all options will be shown. Zero is allowed.
   */
    int _shown_count = -1;
  };



}

#endif // ZYPP_TUI_OUTPUT_PROMPTOPTIONS_H_INCLUDED
