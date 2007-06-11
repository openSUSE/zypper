#
# spec file for package @PACKAGE@ (Version @VERSION@)
#
# Copyright (c) 2006 SUSE LINUX Products GmbH, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

# norootforbuild

Name:           @PACKAGE@
BuildRequires:  libzypp-devel
BuildRequires:  gcc-c++ pkg-config boost-devel gettext-devel
BuildRequires:  readline-devel
Requires:	procps
License:        GPL
Group:          System/Packages
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Autoreqprov:    on
PreReq:         permissions
Summary:        Command line package management tool using libzypp
Version:        @VERSION@
Release:        0
Source:         @PACKAGE@-@VERSION@.tar.bz2
Prefix:         /usr
Url:            http://en.opensuse.org/Zypper
# zypper is not a fully featured replacement yet
#Provides:       y2pmsh
#Obsoletes:      y2pmsh
#Provides:       rug
#Obsoletes:      rug

%description
Command line package management tool using libzypp.

Authors:
--------
    Jan Kupec <jkupec@suse.cz>
    Duncan Mac-Vicar <dmacvicar@suse.de>
    Martin Vidner <mvidner@suse.cz>

%prep
%setup -q

%build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=%{prefix} \
      -DSYSCONFDIR=%{_sysconfdir} \
      -DMANDIR=%{_mandir} \
      -DCMAKE_C_FLAGS="%{optflags}" \
      -DCMAKE_CXX_FLAGS="%{optflags}" \
      -DCMAKE_BUILD_TYPE=Release \
      ..

#gettextize -f
make %{?jobs:-j %jobs}


%install
cd build
make install DESTDIR=$RPM_BUILD_ROOT

# Create filelist with translatins
#%{find_lang} zypper
rm -f ${RPM_BUILD_ROOT}%{_sbindir}/zypp-checkpatches-wrapper
%{__install} -d -m755 %buildroot%_var/log
touch %buildroot%_var/log/zypper.log

%post
%run_ldconfig
%if %suse_version > 1010
%run_permissions
%endif

%if %suse_version > 1010
%verifyscript
%verify_permissions -e %{_sbindir}/zypp-checkpatches-wrapper
%endif

%postun
%run_ldconfig

%clean
rpm -rf $RPM_BUILD_ROOT

#%files -f zypper.lang
%files
%defattr(-,root,root)
%{_sysconfdir}/logrotate.d/zypper.lr
%{_bindir}/zypper
%{_bindir}/installation_sources
%{_sbindir}/zypp-checkpatches
%verify(not mode) %attr (755,root,root) %{_sbindir}/zypp-checkpatches-wrapper
%doc %{_mandir}/*/*
%doc %dir %{_datadir}/doc/packages/zypper
%doc %{_datadir}/doc/packages/zypper/TODO
%doc %{_datadir}/doc/packages/zypper/zypper-rug
# declare ownership of the log file but prevent
# it from being erased by rpm -e
%ghost %config(noreplace) %{_var}/log/zypper.log
