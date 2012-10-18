# spec file for package zypper
#
# Copyright (c) 2006-2011 SUSE LINUX Products GmbH, Nuernberg, Germany.
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

# norootforbuild

Name:           @PACKAGE@
BuildRequires:  libzypp-devel >= 12.3.0
BuildRequires:  boost-devel >= 1.33.1
BuildRequires:  gettext-devel >= 0.15
BuildRequires:  readline-devel >= 5.1
BuildRequires:  augeas-devel >= 0.5.0
BuildRequires:  gcc-c++ >= 4.5
BuildRequires:  cmake >= 2.4.6
Requires:       procps
%if 0%{?suse_version}
%requires_ge    libzypp
Recommends:     logrotate cron zypper-log
Requires(post): permissions
%endif
License:        GPL-2.0+
Group:          System/Packages
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Summary:        Command line software manager using libzypp
Version:        @VERSION@
Release:        0
Source:         %{name}-%{version}.tar.bz2
Source1:        %{name}-rpmlintrc
URL:            http://en.opensuse.org/Zypper
Provides:       y2pmsh
Obsoletes:      y2pmsh

%description
Zypper is a command line tool for managing software. It can be used to add
package repositories, search for packages, install, remove, or update packages,
install patches, hardware drivers, verify dependencies, and more.

Zypper can be used interactively or non-interactively by user, from scripts,
or front-ends.

Authors:
--------
    Jan Kupec <jkupec@suse.cz>
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
Requires:	xz
BuildArch:      noarch
Summary:        CLI for accessing the zypper logfile
License:        GPL-2.0+
Group:          System/Packages
%description -n zypper-log
CLI for accessing the zypper logfile

Authors:
--------
    Dominik Heidler <dheidler@suse.de>

%package aptitude
Summary:        aptitude compatibility with zypper
License:        GPL-2.0+
Requires:       perl
%if 0%{?suse_version}
Supplements:    zypper
%endif
BuildArch:      noarch
Group:          System/Packages
%description aptitude
provides aptitude compatibility using zypper

Authors:
--------
    Bernhard M. Wiedemann <bernhard+aptitude4zypp lsmod de>

%prep
%setup -q

%build
mkdir build
cd build

# The code base is the same, but SLES11-SP1 (suse_version == 1110)
# may use it's own set of .po files from po/sle-zypper-po.tar.bz2.
unset TRANSLATION_SET
%if 0%{?suse_version} == 1110
if [ -f ../po/sle-zypper-po.tar.bz ]; then
  export TRANSLATION_SET=sle-zypper
fi
%endif

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

# Create filelist with translations
cd ..
%{find_lang} zypper
%{__install} -d -m755 $RPM_BUILD_ROOT%{_var}/log
touch $RPM_BUILD_ROOT%{_var}/log/zypper.log

%if 0%{?suse_version}
%post
%run_permissions
%endif

%clean
rm -rf "$RPM_BUILD_ROOT"

%files -f zypper.lang
%defattr(-,root,root)
%config(noreplace) %{_sysconfdir}/zypp/zypper.conf
%config(noreplace) %{_sysconfdir}/logrotate.d/zypper.lr
%config(noreplace) %{_sysconfdir}/logrotate.d/zypp-refresh.lr
%{_sysconfdir}/bash_completion.d/zypper.sh
%{_bindir}/zypper
%{_bindir}/installation_sources
%{_sbindir}/zypp-refresh
%dir %{_datadir}/zypper
%{_datadir}/zypper/zypper.aug
%dir %{_datadir}/zypper/xml
%{_datadir}/zypper/xml/xmlout.rnc
%doc %{_mandir}/man8/zypper.8*
%doc %dir %{_datadir}/doc/packages/zypper
%doc %{_datadir}/doc/packages/zypper/TODO
%doc %{_datadir}/doc/packages/zypper/zypper-rug
%doc %{_datadir}/doc/packages/zypper/COPYING
%doc %{_datadir}/doc/packages/zypper/HACKING
# declare ownership of the log file but prevent
# it from being erased by rpm -e
%ghost %config(noreplace) %{_var}/log/zypper.log

%files log
%defattr(-,root,root)
%{_sbindir}/zypper-log
%doc %{_mandir}/man8/zypper-log.8*

%files aptitude
%defattr(-,root,root)
%{_bindir}/aptitude

%changelog
