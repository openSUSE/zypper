#
# spec file for package @PACKAGE@ (Version @VERSION@)
#
# Copyright (c) 2007 SUSE LINUX Products GmbH, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

# norootforbuild

Name:           @PACKAGE@
BuildRequires:  libzypp-devel >= 6.36.0
BuildRequires:  boost-devel >= 1.33.1
%if 0%{?suse_version} >= 1100
BuildRequires:  gettext-devel >= 0.15
%else
BuildRequires:  gettext-devel
%endif
BuildRequires:  readline-devel >= 5.1 augeas-devel >= 0.5.0
BuildRequires:  gcc-c++ >= 4.1 cmake >= 2.4.6
Requires:	procps
%if 0%{?suse_version}
%if 0%{?suse_version} >= 1100
%requires_ge	libzypp
%endif
Recommends:     logrotate cron
PreReq:         permissions
%endif
License:        GPL v2 or later
Group:          System/Packages
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Autoreqprov:    on
Summary:        Command line software manager using libzypp
Version:        @VERSION@
Release:        0
Source:         %{name}-%{version}.tar.bz2
Source1:        %{name}-rpmlintrc
Prefix:         /usr
URL:            http://en.opensuse.org/Zypper
Provides:       y2pmsh
Obsoletes:      y2pmsh

Provides:       zypper(oldpackage)

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

cmake -DCMAKE_INSTALL_PREFIX=%{prefix} \
      -DSYSCONFDIR=%{_sysconfdir} \
      -DMANDIR=%{_mandir} \
      -DCMAKE_VERBOSE_MAKEFILE=TRUE \
      -DCMAKE_C_FLAGS_RELEASE:STRING="%{optflags}" \
      -DCMAKE_CXX_FLAGS_RELEASE:STRING="%{optflags}" \
      -DCMAKE_BUILD_TYPE=Release \
      %{?use_translation_set:-DUSE_TRANSLATION_SET=%use_translation_set} \
      ..

#gettextize -f
make %{?jobs:-j %jobs}
make -C po %{?jobs:-j %jobs} translations

%install
cd build
make install DESTDIR=$RPM_BUILD_ROOT
make -C po install DESTDIR=$RPM_BUILD_ROOT

# Create filelist with translations
cd ..
%{find_lang} zypper
%{__install} -d -m755 %buildroot%_var/log
touch %buildroot%_var/log/zypper.log

%if 0%{?suse_version}
%post
%run_permissions

%verifyscript
%verify_permissions -e %{_sbindir}/zypp-refresh-wrapper
%endif

%clean


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
%dir %{prefix}/share/zypper
%{prefix}/share/zypper/zypper.aug
%dir %{prefix}/share/zypper/xml
%{prefix}/share/zypper/xml/xmlout.rnc
%dir %{prefix}/include/zypper
%{prefix}/include/zypper/prompt.h
%doc %{_mandir}/*/*
%doc %dir %{_datadir}/doc/packages/zypper
%doc %{_datadir}/doc/packages/zypper/TODO
%doc %{_datadir}/doc/packages/zypper/zypper-rug
%doc %{_datadir}/doc/packages/zypper/COPYING
%doc %{_datadir}/doc/packages/zypper/HACKING
# declare ownership of the log file but prevent
# it from being erased by rpm -e
%ghost %config(noreplace) %{_var}/log/zypper.log
%if 0%{?suse_version} < 1100
# building on Code10: add locale directories _NOT_ owned by Code10 filesystem
%dir %{prefix}/share/locale/si
%dir %{prefix}/share/locale/si/LC_MESSAGES
%endif

