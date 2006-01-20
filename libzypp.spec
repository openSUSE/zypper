#
# spec file for package libzypp (Version 1.9.1)
#
# Copyright (c) 2006 SUSE LINUX Products GmbH, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org
#

# norootforbuild
# neededforbuild  gpp libgpp boost boost-devel doxygen graphviz gettext-devel tcl expect dejagnu pkgconfig expat zlib zlib-devel libxml2 libxml2-devel curl curl-devel dbus-1 dbus-1-glib dbus-1-devel hal hal-devel fontconfig freetype2 libjpeg libpng  glib2 glib2-devel rpm-devel popt popt-devel openssl openssl-devel libicu

BuildRequires: binutils gcc gcc-c++ glibc-devel libstdc++ libstdc++-devel boost-devel doxygen graphviz gettext-devel dejagnu pkgconfig expat zlib-devel libxml2-devel curl-devel dbus-1-devel hal-devel glib2 glib2-devel rpm-devel

Name:         libzypp
License:      GNUv2
Group:        System/Libraries
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Autoreqprov:  on
Obsoletes:    yast2-packagemanager
Summary:      Package, Patch, Pattern, Product Management
Version:      0.0.0
Release:      0
Source:       zypp-0.0.0.tar.bz2
prefix:       /usr

%description
We are ZYPP, you will be assimilated.

%package devel
Requires:     libzypp = %{version}
Summary:      Package, Patch, Pattern, Product Management - development files
Group:        Development/Libraries/C and C++

%description -n libzypp-devel
We are ZYPP, you will be assimilated.

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

%post
%run_ldconfig

%postun
%run_ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
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
