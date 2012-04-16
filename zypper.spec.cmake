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
BuildRequires:  libzypp-devel >= 8.12.0
BuildRequires:  boost-devel >= 1.33.1
BuildRequires:  gettext-devel >= 0.15
BuildRequires:  readline-devel >= 5.1
BuildRequires:  augeas-devel >= 0.5.0
BuildRequires:  gcc-c++ >= 4.1
BuildRequires:  cmake >= 2.4.6
Requires:       procps
%if 0%{?suse_version}
%requires_ge    libzypp
Recommends:     logrotate cron
Requires(post): permissions
%endif
License:        GPLv2+
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

%prep
%setup -q

%build
mkdir build
cd build

# Use different translation set for SUSE Linux Enterprise 10 SP1
#
# The code base is the same, but SLES11-SP1 (suse_version == 1110)
# may use it's own set of .po files from po/sle-zypper-po.tar.bz2.
%if 0%{?suse_version} == 1110
%define use_translation_set sle-zypper
%endif

cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} \
      -DSYSCONFDIR=%{_sysconfdir} \
      -DMANDIR=%{_mandir} \
      -DCMAKE_VERBOSE_MAKEFILE=TRUE \
      -DCMAKE_C_FLAGS_RELEASE:STRING="$RPM_OPT_FLAGS" \
      -DCMAKE_CXX_FLAGS_RELEASE:STRING="$RPM_OPT_FLAGS" \
      -DCMAKE_BUILD_TYPE=Release \
      %{?use_translation_set:-DUSE_TRANSLATION_SET=%use_translation_set} \
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

%verifyscript
%verify_permissions -e %{_sbindir}/zypp-refresh-wrapper
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
%verify(not mode) %attr (755,root,root) %{_sbindir}/zypp-refresh-wrapper
%dir %{_datadir}/zypper
%{_datadir}/zypper/zypper.aug
%dir %{_datadir}/zypper/xml
%{_datadir}/zypper/xml/xmlout.rnc
%dir %{_includedir}/zypper
%{_includedir}/zypper/prompt.h
%doc %{_mandir}/*/*
%doc %dir %{_datadir}/doc/packages/zypper
%doc %{_datadir}/doc/packages/zypper/TODO
%doc %{_datadir}/doc/packages/zypper/zypper-rug
%doc %{_datadir}/doc/packages/zypper/COPYING
%doc %{_datadir}/doc/packages/zypper/HACKING
# declare ownership of the log file but prevent
# it from being erased by rpm -e
%ghost %config(noreplace) %{_var}/log/zypper.log
