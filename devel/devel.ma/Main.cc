#include "Tools.h"

#include <zypp/PoolQuery.h>
#include <zypp/target/rpm/librpmDb.h>

///////////////////////////////////////////////////////////////////

//static const Pathname sysRoot( getenv("SYSROOT") ? getenv("SYSROOT") : "/Local/ROOT" );
static const Pathname sysRoot( "/tmp/ToolScanRepos" );

///////////////////////////////////////////////////////////////////

bool solve()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    //getZYpp()->resolver()->setOnlyRequires( true );
    rres = getZYpp()->resolver()->resolvePool();
  }
  if ( ! rres )
  {
    ERR << "resolve " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "resolve " << rres << endl;
  return true;
}

bool upgrade()
{
  bool rres = false;
  {
    //zypp::base::LogControl::TmpLineWriter shutUp;
    Measure x( "Upgrade" );
    rres = getZYpp()->resolver()->doUpgrade();
  }
  if ( ! rres )
  {
    Measure x( "Upgrade Error" );
    ERR << "upgrade " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "upgrade " << rres << endl;
  return true;
}

namespace zypp
{
  namespace target
  {
    void writeUpgradeTestcase();
  }
}

int main( int argc, char * argv[] )
try {
  --argc;
  ++argv;
  zypp::base::LogControl::instance().logToStdErr();
  INT << "===[START]==========================================" << endl;
  ::unsetenv( "ZYPP_CONF" );
  ZConfig::instance();
  TestSetup::LoadSystemAt( sysRoot, Arch_i586 );
  ///////////////////////////////////////////////////////////////////
  ResPool   pool( ResPool::instance() );
  sat::Pool satpool( sat::Pool::instance() );
  ///////////////////////////////////////////////////////////////////
  dumpRange( USR, satpool.reposBegin(), satpool.reposEnd() ) << endl;
  USR << "pool: " << pool << endl;

  sat::AttrMatcher matches( "^aaa_base$", Match::REGEX );

  getZYpp()->resolver()->addRequire( Capability("aaa_base") );

  solve();
  for_( it, make_filter_begin<resfilter::ByTransact>(pool), make_filter_end<resfilter::ByTransact>(pool) )
  {
      USR << *it << " " << (*it)->repoInfo().name() << endl;
      if ( matches( (*it)->name() ) )
      {
        SEC << *it << " " << (*it)->repoInfo().name() << endl;
      }
  }

  std::vector<std::string> words;
  str::split( "3ddiag ConsoleKit-x11 FZFangSong FZKaiTiB IPAGothic ImageMagick MozillaFirefox MozillaFirefox-branding-Moblin MozillaFirefox-translations NetworkManager OpenOffice_org OpenOffice_org-LanguageTool OpenOffice_org-LanguageTool-de OpenOffice_org-LanguageTool-en OpenOffice_org-LanguageTool-es OpenOffice_org-LanguageTool-fr OpenOffice_org-LanguageTool-it OpenOffice_org-LanguageTool-nl OpenOffice_org-LanguageTool-pl OpenOffice_org-LanguageTool-sv OpenOffice_org-base OpenOffice_org-base-extensions OpenOffice_org-branding-SLED OpenOffice_org-calc OpenOffice_org-calc-extensions OpenOffice_org-components OpenOffice_org-draw OpenOffice_org-draw-extensions OpenOffice_org-filters OpenOffice_org-filters-optional OpenOffice_org-gnome OpenOffice_org-hyphen OpenOffice_org-icon-themes OpenOffice_org-impress OpenOffice_org-impress-extensions OpenOffice_org-l10n-de OpenOffice_org-l10n-el OpenOffice_org-l10n-en-GB OpenOffice_org-l10n-es OpenOffice_org-l10n-extras OpenOffice_org-l10n-fi OpenOffice_org-l10n-fr OpenOffice_org-l10n-it OpenOffice_org-l10n-ja OpenOffice_org-l10n-ko OpenOffice_org-l10n-nl OpenOffice_org-l10n-pl OpenOffice_org-l10n-pt OpenOffice_org-l10n-pt-BR OpenOffice_org-l10n-ru OpenOffice_org-l10n-sv OpenOffice_org-l10n-zh-CN OpenOffice_org-l10n-zh-TW OpenOffice_org-libs-core OpenOffice_org-libs-extern OpenOffice_org-libs-gui OpenOffice_org-math OpenOffice_org-openclipart OpenOffice_org-templates-en OpenOffice_org-templates-labels-a4 OpenOffice_org-templates-labels-letter OpenOffice_org-templates-presentation-layouts OpenOffice_org-thesaurus-de OpenOffice_org-thesaurus-en OpenOffice_org-thesaurus-es OpenOffice_org-thesaurus-fr OpenOffice_org-thesaurus-pl OpenOffice_org-thesaurus-pt OpenOffice_org-thesaurus-ru OpenOffice_org-thesaurus-sk OpenOffice_org-thesaurus-sv OpenOffice_org-ure OpenOffice_org-writer OpenOffice_org-writer-extensions PackageKit PolicyKit PolicyKit-gnome SDL SDL_image SDL_mixer SDL_net SDL_ttf SuSEfirewall2 WorldOfGooDemo a2ps aaa_base acct acl acpid acroread agfa-fonts alsa alsa-tools amavisd-new anerley anjal ash at atk attr audit-libs autoyast2-installation avahi backup-manager baekmuk-ttf banshee-1 banshee-1-backend-engine-gstreamer banshee-1-backend-platform-gnome banshee-1-backend-platform-unix banshee-1-extensions-default banshee-1-novell-aac bash bc bind-libs bind-utils binutils bisho bitstream-vera blt bluez bluez-alsa bluez-cups bluez-firmware bluez-gnome bluez-gstreamer bootchart bootsplash-branding-SLED branding-SLED brasero bzip2 cabextract cairo cdrkit-cdrtools-compat checkmedia cheese cifs-mount clutter-mozembed compat convmv coreutils cpio cpp cpufrequtils cracklib cracklib-dict-full cron cryptconfig csync cubano cups cups-autoconfig cups-backends cups-client cups-drivers cups-libs curl cyrus-sasl cyrus-sasl-crammd5 cyrus-sasl-digestmd5 cyrus-sasl-gssapi cyrus-sasl-plain dalston dates db-utils dbus-1 dbus-1-glib dbus-1-x11 dejavu deltarpm desktop-file-utils desktop-translations device-mapper dhcpcd dialog diffutils dirmngr dmraid dos2unix dosfstools e2fsprogs ed eject elfutils empathy ethtool evince evolution evolution-pilot expat extreme-tuxracer f-spot fbset file file-roller filesystem fillup filters findutils finger flash-player fontconfig fonts-config foomatic-filters freeglut freetype freetype2 fribidi frozen-bubble gammu gawk gcalctool gconf2-branding-SLED gdb gdbm gdm gdm-branding-upstream gedit geoclue gettext-runtime gftp gfxboot-branding-SLED ghostscript-fonts-other ghostscript-fonts-std ghostscript-library ghostscript-x11 giflib gimp glib glib2 glib2-branding-Moblin glibc glibc-locale gnome-audio gnome-control-center gnome-desktop gnome-games gnome-icon-theme gnome-keyring gnome-keyring-pam gnome-media gnome-menus gnome-mime-data gnome-mount gnome-packagekit gnome-panel gnome-pilot gnome-session gnome-spell2 gnome-system-monitor gnome-terminal gnome-themes gnome-utils gnome-vfs2 gnome-web-photo gpart gpg2 gpgme gphoto gpm gpsd grep groff grub gstreamer-0_10 gstreamer-0_10-plugins-base gstreamer-0_10-plugins-farsight gstreamer-0_10-plugins-good gstreamer-0_10-schroedinger gstreamer-0_10-utils gsynaptics gtk gtk2 gtk2-branding-SLED gtk2-engines gtk2-theme-SLED gtk2-themes gtkhtml2 gtkspell gutenprint gvfs-backends gypsy gzip hal hdparm hornsey hwinfo ifplugd imlib info initviocons inkscape insserv iproute2 iptables iputils ipw-firmware ispell ispell-american ispell_english_dictionary iw iwl3945-ucode iwl4965-ucode iwl5000-ucode jana java-1_6_0-sun java-1_6_0-sun-plugin json-glib kernel-default kernel-default-base kernel-default-extra kernel-firmware klogd krb5 ksymoops laptop-mode-tools lcms less libQtWebKit4 libacl libattr libblocxx6 libbluetooth3 libclutter-box2d-0_10-0 libclutter-glx-1_0-0 libclutter-gst-1_0-0 libclutter-gtk-1_0-0 libclutter-qt-1_0-0 libdb-4_5 libevent-1_4-2 libfakekey0 libfprint0 libgcc43 libgcrypt11 libgimpprint libgpg-error0 libgsf-gnome libjpeg libmikmod libmng libmoon0 libnetpbm10 libnscd libopensync-plugin-file libpcap0 libpng12-0 libproxy-tools libproxy0 libproxy0-gnome libproxy0-mozjs libproxy0-networkmanager libreadline5 librpcsecgss libsmbios-bin libstdc++33 libstdc++43 libtiff3 libusbpp-0_1-4 libxcrypt libxml2 libxslt libzio libzypp lilo limal limal-perl linux-atm-lib login logrotate lsof lukemftp m4 mailx make man man-pages manufacturer-PPDs master-boot-code mdadm meerkat metacity metamail microcode_ctl mingetty mkinitrd moblin-branding-Samsung moblin-gtk-engine moblin-icon-theme moblin-menus moblin-registration moblin-release moblin-sound-theme moblin-ux-settings moblin-web-browser module-init-tools mojito monsoon moonlight-plugin moonshine moonshine-plugin mousetweaks mozilla-filesystem mozilla-xulrunner190 mtools mutter mutter-moblin myspell-american myspell-british myspell-dutch myspell-french myspell-german myspell-greek myspell-italian myspell-polish myspell-portuguese myspell-russian myspell-spanish myspell-swedish nautilus nautilus-cd-burner nautilus-eiciel nautilus-open-terminal nautilus-sendto nautilus-share nbtk ncurses-utils net-tools netcat netcfg network-manager-netbook neverball nfsidmap notification-daemon novell-ipsec-tools novell-ldapext novell-nortelplugins nscd nspluginwrapper nss_ldap ntfs-3g ntfsprogs ntp numlockx openclipart-svg openldap2-client openmotif-libs openobex opensc openslp openssh openssh-askpass openssl padevchooser pam pam-config pam-modules pam_csync pam_krb5 pam_ldap pam_p11 pam_pkcs11 pam_radius pam_smb pam_ssh paman paprefs parted pavucontrol pavumeter pciutils pcmciautils pcre pcsc-lite perl perl-Bootloader perl-Config-Crontab perl-Crypt-SmbHash perl-Digest-MD4 perl-Digest-SHA1 perl-Parse-RecDescent perl-TermReadKey perl-X500-DN perl-XML-XPath perl-base perl-gettext perl-satsolver permissions pinentry-gtk2 planner plymouth-lite pmtools polkit-default-privs popt postfix ppp pptp preload procinfo procmail procps psmisc pulseaudio-esound-compat pulseaudio-module-bluetooth pulseaudio-module-gconf pulseaudio-module-lirc pulseaudio-module-x11 pulseaudio-module-zeroconf pulseaudio-utils pwdutils python python-gnome python-gtk python-numeric python-qt python-tk python-xml rdesktop recode reiserfs release-notes-sled rest rpm rsh rsync rsyslog rtc-tzset samba samba-client sample-media-images samsung-icon-theme samsung-manual sash sax2 sax2-gui sax2-ident sax2-libsax sax2-libsax-perl sax2-tools scim scim-bridge scim-bridge-clutter scim-bridge-gtk scim-bridge-qt scim-chewing scim-m17n scim-pinyin scim-tables scim-tables-zh screen seahorse sed setserial sg3_utils shared-mime-info sled-manuals_en sled-moblinquick_en sled-moblinquick_en-pdf splashy splashy-branding-SLED sqlite3 sreadahead star startup-notification strace sudo supportutils suse-build-key suse-sam suseRegister susehelp susehelp_en suspend syncevolution sysconfig sysfsutils syslinux system-config-printer sysvinit tango-icon-theme tar tcl-devel tcpd tcsh telnet terminfo tightvnc timezone tk-devel translation-update tsclient turnpike twitter-glib udev ulimit unrar unzip usbutils utempter util-linux uxlaunch vbetool vim vim-data vte w3m wdiff wget wimax-tools wireless-tools wol words wpa_supplicant wvdial wvstreams x11-input-synaptics x11-tools x86info xchat xdmbgrd xfsprogs xkeyboard-config xorg-x11 xorg-x11-Xvnc xorg-x11-driver-input xorg-x11-driver-video xorg-x11-fonts xorg-x11-libX11-ccache xorg-x11-libs xorg-x11-server xorg-x11-server-extra xorg-x11-xauth xpdf-tools xterm xtermset xulrunner yast2 yast2-bootloader yast2-branding-SLED yast2-control-center-gnome yast2-core yast2-country yast2-firewall yast2-firstboot yast2-gtk yast2-hardware-detection yast2-installation yast2-kerberos-client yast2-ldap yast2-ldap-client yast2-metapackage-handler yast2-mouse yast2-ncurses yast2-ncurses-pkg yast2-network yast2-ntp-client yast2-online-update yast2-online-update-frontend yast2-packager yast2-pam yast2-perl-bindings yast2-pkg-bindings yast2-qt yast2-qt-pkg yast2-rdp yast2-registration yast2-registration-branding-SLE yast2-repair yast2-runlevel yast2-samba-client yast2-schema yast2-security yast2-slp yast2-sound yast2-storage yast2-storage-lib yast2-support yast2-sysconfig yast2-theme-SLE yast2-trans-en_GB yast2-trans-en_US yast2-trans-es yast2-trans-fi yast2-trans-fr yast2-trans-ja yast2-trans-ko yast2-trans-pt_BR yast2-trans-stats yast2-trans-sv yast2-trans-zh_CN yast2-trans-zh_TW yast2-transfer yast2-tune yast2-update yast2-users yast2-wagon yast2-x11 yast2-xml yelp youtube-player zenity zip zlib",
              std::back_inserter(words) );
  for_( it, words.begin(), words.end() )
  {
    getZYpp()->resolver()->addRequire( Capability(*it) );
  }

  getZYpp()->resolver()->addRequire( Capability("aaa_base > 11-6.2") );

  solve();
  for_( it, make_filter_begin<resfilter::ByTransact>(pool), make_filter_end<resfilter::ByTransact>(pool) )
  {
      USR << *it << " " << (*it)->repoInfo().name() << endl;
      if ( matches( (*it)->name() ) )
      {
        SEC << *it << " " << (*it)->repoInfo().name() << endl;
      }
  }


  if ( 0 )
  {
    getZYpp()->resolver()->addRequire( Capability("emacs") );
    solve();
    vdumpPoolStats( USR << "Transacting:"<< endl,
                    make_filter_begin<resfilter::ByTransact>(pool),
                    make_filter_end<resfilter::ByTransact>(pool) ) << endl;
  }

  ///////////////////////////////////////////////////////////////////
  INT << "===[END]============================================" << endl << endl;
  zypp::base::LogControl::instance().logNothing();
  return 0;
}
catch ( const Exception & exp )
{
  INT << exp << endl << exp.historyAsString();
}
catch (...)
{}

