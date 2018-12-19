/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "products.h"
#include "Zypper.h"
#include "utils/flags/flagtypes.h"
#include "global-settings.h"

using namespace zypp;

ProductsCmdBase::ProductsCmdBase(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("products (pd) [OPTIONS] [REPOSITORY] ..."),
    // translators: command summary: products, pd
    _("List all available products."),
    // translators: command description
    _("List all products available in specified repositories."),
    DisableAll
  )
{
  _initRepoFlags.setCompatibilityMode( CompatModeBits::EnableRugOpt | CompatModeBits::EnableNewOpt );
}


zypp::ZyppFlags::CommandGroup ProductsCmdBase::cmdOptions() const
{
  auto that = const_cast<ProductsCmdBase *>(this);
  return {{
    { "xmlfwd", '\0', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType( &that->_xmlFwdTags, ARG_TAG ),
      // translators: --xmlfwd <TAG>
      _("XML output only: Literally forward the XML tags found in a product file.")
    }
  }};
}

void ProductsCmdBase::doReset()
{
  _xmlFwdTags.clear();
}

int ProductsCmdBase::execute( Zypper &zypper, const std::vector<std::string> & )
{
  if ( _xmlFwdTags.size() && zypper.out().type() != Out::TYPE_XML )
  {
    zypper.out().warning( str::Format(_("Option %1% has no effect without the %2% global option.")) % "--xmlfwd" % "--xmlout" );
  }

  if ( zypper.out().type() == Out::TYPE_XML )
    list_products_xml( zypper, _instFilterFlags._mode, _xmlFwdTags );
  else
    list_product_table( zypper, _instFilterFlags._mode );
  return ZYPPER_EXIT_OK;
}
