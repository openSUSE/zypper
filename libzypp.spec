#
# spec file for package libzypp (Version 0.0.7)
#
# Copyright (c) 2006 SUSE LINUX Products GmbH, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org
#

# norootforbuild

Name:         libzypp
BuildRequires: boost-devel curl-devel dejagnu doxygen gcc-c++ graphviz hal-devel libjpeg-devel libxml2-devel rpm-devel sqlite-devel
License:      GPL
Group:        System/Packages
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Autoreqprov:  on
Summary:      Package, Patch, Pattern, and Product Management
Version:      0.0.7
Release:      1
Source:       zypp-0.0.0.tar.bz2
Prefix:       /usr
Provides:     yast2-packagemanager
Obsoletes:    yast2-packagemanager

%description
Package, Patch, Pattern, and Product Management

Authors:
--------
    Michael Andres <ma@suse.de>
    Jiri Srain <jsrain@suse.cz>
    Stefan Schubert <schubi@suse.de>
    Duncan Mac-Vicar <dmacvicar@suse.de>
    Klaus Kaempf <kkaempf@suse.de>

%package devel
Requires:     libzypp
Summary:      Package, Patch, Pattern, and Product Management - developers files
Group:        System/Packages
Provides:     yast2-packagemanager-devel
Obsoletes:    yast2-packagemanager-devel

%description -n libzypp-devel
Package, Patch, Pattern, and Product Management - developers files

Authors:
--------
    Michael Andres <ma@suse.de>
    Jiri Srain <jsrain@suse.cz>
    Stefan Schubert <schubi@suse.de>
    Duncan Mac-Vicar <dmacvicar@suse.de>
    Klaus Kaempf <kkaempf@suse.de>

%package zmd-backend
Requires:     libzypp
Provides:     zmd-backend
Provides:     zmd-librc-backend
Obsoletes:    zmd-librc-backend
Summary:      ZMD backend for Package, Patch, Pattern, and Product Management - developers files
Group:        System/Management

%description -n libzypp-zmd-backend
This package provides backend binaries for ZMD

Authors:
--------
    Klaus Kaempf <kkaempf@suse.de>

%prep
%setup -q -n zypp-0.0.0

%build
mv configure.ac x
grep -v devel/ x > configure.ac
autoreconf --force --install --symlink --verbose
%{?suse_update_config:%{suse_update_config -f}}
./configure --prefix=%{prefix} --libdir=%{_libdir} --mandir=%{_mandir} --disable-static
make %{?jobs:-j %jobs}
make check

%install
make install DESTDIR=$RPM_BUILD_ROOT
# Create filelist with translatins
%{find_lang} zypp

%post
%run_ldconfig

%postun
%run_ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files -f zypp.lang
%defattr(-,root,root)
%dir %{_libdir}/libzypp*so.*

%files devel
%defattr(-,root,root)
%dir %{_libdir}/libzypp.so
%dir %{_libdir}/libzypp.la
%dir %{_docdir}/zypp
%{_docdir}/zypp/*
%dir %{prefix}/include/zypp
%{prefix}/include/zypp/*
%dir %{prefix}/share/zypp
%{prefix}/share/zypp/*

%files zmd-backend
%defattr(-,root,root)
%dir %{_libdir}/zmd
%{_libdir}/zmd/*

%changelog -n libzypp
* Fri Feb 03 2006 - schubi@suse.de
- removed Obsoletes:    yast2-packagemanager
* Fri Feb 03 2006 - schubi@suse.de
- Snapshoot 3 Feb 2005 (11:30)
* Thu Feb 02 2006 - schubi@suse.de
- Snapshoot 2 Feb 2005 (14:00)
* Thu Feb 02 2006 - schubi@suse.de
- Snapshoot 2 Feb 2005 ( integrating YaST )
* Wed Jan 25 2006 - mls@suse.de
- converted neededforbuild to BuildRequires
* Sat Jan 14 2006 - kkaempf@suse.de
- Initial version
