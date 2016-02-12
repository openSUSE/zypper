#
# spec file for package zypper
#
# Copyright (c) 2006-2013 SUSE LINUX Products GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#


Name:           @PACKAGE@
BuildRequires:  augeas-devel >= 0.5.0
BuildRequires:  boost-devel >= 1.33.1
BuildRequires:  cmake >= 2.4.6
BuildRequires:  gcc-c++ >= 4.7
BuildRequires:  gettext-devel >= 0.15
BuildRequires:  libzypp-devel >= 15.20.1
BuildRequires:  readline-devel >= 5.1
Requires:       procps
%if 0%{?suse_version}
%requires_ge    libzypp
Recommends:     logrotate cron zypper-log
%endif
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Summary:        Command line software manager using libzypp
License:        GPL-2.0+
Group:          System/Packages
Version:        @VERSION@
Release:        0
Source:         %{name}-%{version}.tar.bz2
Source1:        %{name}-rpmlintrc
Url:            http://en.opensuse.org/Zypper
Provides:       y2pmsh
Obsoletes:      y2pmsh

Provides:       zypper(oldpackage)
Provides:	zypper(updatestack-only)

%description
Zypper is a command line tool for managing software. It can be used to add
package repositories, search for packages, install, remove, or update packages,
install patches, hardware drivers, verify dependencies, and more.

Zypper can be used interactively or non-interactively by user, from scripts,
or front-ends.

Authors:
--------
    Jan Kupec <jkupec@suse.cz>
    Michael Andres <ma@suse.de>
    Duncan Mac-Vicar <dmacvicar@suse.de>
    Martin Vidner <mvidner@suse.cz>
    Josef Reidinger <jreidinger@suse.cz>

%package log
%if 0%{?suse_version} && 0%{?suse_version} < 1140
Requires:       python >= 2.6
Requires:       python-argparse
%else
Requires:       python >= 2.7
%endif
Requires:       xz
BuildArch:      noarch
Summary:        CLI for accessing the zypper logfile
Group:          System/Packages

%description -n zypper-log
CLI for accessing the zypper logfile

Authors:
--------
    Dominik Heidler <dheidler@suse.de>

%package aptitude
Summary:        aptitude compatibility with zypper
Group:          System/Packages
Requires:       perl
Requires:       zypper
%if 0%{?suse_version}
Supplements:    zypper
%endif
BuildArch:      noarch

%description aptitude
provides compatibility to Debian's aptitude command using zypper

Authors:
--------
    Bernhard M. Wiedemann <bernhard+aptitude4zypp lsmod de>

%prep
%setup -q

%build
mkdir -p build
cd build

# We are moving towards a uniform translation set for SLE and openSUSE.
# While separate SLE translations are still present, overlay them.
unset TRANSLATION_SET
if [ -f ../po/sle-zypper-po.tar.bz2 ]; then
  export TRANSLATION_SET=sle-zypper
fi

cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} \
      -DSYSCONFDIR=%{_sysconfdir} \
      -DMANDIR=%{_mandir} \
      -DCMAKE_VERBOSE_MAKEFILE=TRUE \
      -DCMAKE_C_FLAGS_RELEASE:STRING="$RPM_OPT_FLAGS" \
      -DCMAKE_CXX_FLAGS_RELEASE:STRING="$RPM_OPT_FLAGS" \
      -DCMAKE_BUILD_TYPE=Release \
      -DUSE_TRANSLATION_SET=${TRANSLATION_SET:-zypper} \
      ..

#gettextize -f
make %{?_smp_mflags}
make -C po %{?_smp_mflags} translations

%install
cd build
make install DESTDIR=$RPM_BUILD_ROOT
make -C po install DESTDIR=$RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib/zypper
mkdir -p $RPM_BUILD_ROOT%{_prefix}/lib/zypper/commands

# yzpper symlink
ln -s zypper $RPM_BUILD_ROOT%{_bindir}/yzpper

# Create filelist with translations
cd ..
%{find_lang} zypper
%{__install} -d -m755 $RPM_BUILD_ROOT%{_var}/log
touch $RPM_BUILD_ROOT%{_var}/log/zypper.log

%clean
rm -rf "$RPM_BUILD_ROOT"

%files -f zypper.lang
%defattr(-,root,root)
%config(noreplace) %{_sysconfdir}/zypp/zypper.conf
%config(noreplace) %{_sysconfdir}/logrotate.d/zypper.lr
%config(noreplace) %{_sysconfdir}/logrotate.d/zypp-refresh.lr
%{_sysconfdir}/bash_completion.d/zypper.sh
%{_bindir}/zypper
%{_bindir}/yzpper
%{_bindir}/installation_sources
%{_sbindir}/zypp-refresh
%dir %{_datadir}/zypper
%{_datadir}/zypper/zypper.aug
%dir %{_datadir}/zypper/xml
%{_datadir}/zypper/xml/xmlout.rnc
%{_prefix}/lib/zypper
%doc %{_mandir}/man8/zypper.8*
%doc %{_mandir}/man8/zypp-refresh.8*
%doc %dir %{_datadir}/doc/packages/zypper
%doc %{_datadir}/doc/packages/zypper/COPYING
%doc %{_datadir}/doc/packages/zypper/HACKING
# declare ownership of the log file but prevent
# it from being erased by rpm -e
%ghost %config(noreplace) %attr (640,root,root) %{_var}/log/zypper.log

%files log
%defattr(-,root,root)
%{_sbindir}/zypper-log
%doc %{_mandir}/man8/zypper-log.8*

%files aptitude
%defattr(-,root,root)
%{_bindir}/aptitude
%{_bindir}/apt-get
%dir %{_sysconfdir}/zypp/apt-packagemap.d/
%config(noreplace) %{_sysconfdir}/zypp/apt-packagemap.d/*

%changelog
