#
# spec file for package libzypp
#
# Copyright (c) 2005-2013 SUSE LINUX Products GmbH, Nuernberg, Germany.
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

%if 0%{?suse_version} > 1500 || 0%{?sle_version} >= 150400 || (0%{?is_opensuse} && 0%{?sle_version} >= 150100)
%bcond_without zchunk
%else
%bcond_with zchunk
%endif
# libsolvs external references require us to link against it:
%if 0%{?sle_version} >= 150000 || 0%{?suse_version} >= 1500
%bcond_without zstd
%else
%bcond_with zstd
%endif

%bcond_without mediabackend_tests

# older libsigc versions have a bug that causes a segfault
# when clearing connections during signal emission
# see https://bugzilla.gnome.org/show_bug.cgi?id=784550
%if 0%{?sle_version} < 150200
%bcond_without sigc_block_workaround
%else
%bcond_with sigc_block_workaround
%endif

# Distros using just zypper may want to enable this as default earlier
%bcond_with enable_preview_single_rpmtrans_as_default_for_zypper

Name:           libzypp
Version:        @VERSION@
Release:        0
License:        GPL-2.0+
Url:            https://github.com/openSUSE/libzypp
Summary:        Library for package, patch, pattern and product management
Group:          System/Packages
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Source:         %{name}-%{version}.tar.bz2
Source1:        %{name}-rpmlintrc
Provides:       yast2-packagemanager
Obsoletes:      yast2-packagemanager

# Features we provide (update doc/autoinclude/FeatureTest.doc):
Provides:       libzypp(plugin) = 0.1
Provides:       libzypp(plugin:appdata) = 0
Provides:       libzypp(plugin:commit) = 1
Provides:       libzypp(plugin:services) = 1
Provides:       libzypp(plugin:system) = 1
Provides:       libzypp(plugin:urlresolver) = 0
Provides:       libzypp(plugin:repoverification) = 0
Provides:       libzypp(repovarexpand) = 1.1

%if 0%{?suse_version}
Recommends:     logrotate
# lsof is used for 'zypper ps':
Recommends:     lsof
%endif
BuildRequires:  cmake >= 3.1
BuildRequires:  pkgconfig(openssl)
BuildRequires:  pkgconfig(libudev)
%if 0%{?suse_version} >= 1330
BuildRequires:  libboost_headers-devel
BuildRequires:  libboost_program_options-devel
BuildRequires:  libboost_test-devel
BuildRequires:  libboost_thread-devel
%else
BuildRequires:  boost-devel
%endif
BuildRequires:  dejagnu
BuildRequires:  doxygen
BuildRequires:  texlive-latex
BuildRequires:  texlive-xcolor
BuildRequires:  texlive-newunicodechar
BuildRequires:  texlive-dvips
BuildRequires:  ghostscript
BuildRequires:  gcc-c++ >= 7
BuildRequires:  gettext-devel
BuildRequires:  graphviz
BuildRequires:  libxml2-devel
BuildRequires:  yaml-cpp-devel
BuildRequires:  libproxy-devel

%if 0%{?fedora_version} || 0%{?rhel_version} || 0%{?centos_version}
BuildRequires:  pkgconfig
%else
BuildRequires:  pkg-config
%endif

BuildRequires:  libsolv-devel >= 0.7.17
%if 0%{?suse_version} >= 1100
BuildRequires:  libsolv-tools
%requires_eq    libsolv-tools
Requires:	( findutils if libsolv-tools <= 0.7.23 )
%else
Requires:       libsolv-tools
%endif

BuildRequires:  glib2-devel
BuildRequires:  libsigc++2-devel
BuildRequires:  protobuf-devel

# required for testsuite
%if %{with mediabackend_tests}
BuildRequires:  nginx
%endif

Requires:       rpm
BuildRequires:  rpm

%if 0%{?suse_version}
BuildRequires:  rpm-devel > 4.4
%endif

%if 0%{?fedora_version} || 0%{?rhel_version} >= 600 || 0%{?centos_version} >= 600
BuildRequires:  popt-devel
BuildRequires:  rpm-devel > 4.4
%endif

%if 0%{?mandriva_version}
BuildRequires:  librpm-devel > 4.4
%endif

%if 0%{?suse_version}
BuildRequires:  libgpgme-devel
#testsuite
%if %{with mediabackend_tests}
BuildRequires:  FastCGI-devel
%endif
%else
BuildRequires:  gpgme-devel
#testsuite
%if %{with mediabackend_tests}
BuildRequires:	fcgi-devel
%endif
%endif

%define min_curl_version 7.19.4
%if 0%{?suse_version}
%if 0%{?suse_version} >= 1100
# Code11+
BuildRequires:  libcurl-devel >= %{min_curl_version}
Requires:       libcurl4   >= %{min_curl_version}
%else
# Code10
BuildRequires:  curl-devel
%endif
%else
# Other distros (Fedora)
BuildRequires:  libcurl-devel >= %{min_curl_version}
Requires:       libcurl   >= %{min_curl_version}
%endif

# required for documentation
%if 0%{?suse_version} >= 1330
BuildRequires:  rubygem(asciidoctor)
%else
BuildRequires:  asciidoc
BuildRequires:  libxslt-tools
%endif

%if %{with zchunk}
BuildRequires:  libzck-devel
%endif
%if %{with zstd}
BuildRequires:  libzstd-devel
%endif

%description
libzypp is the package management library that powers applications
like YaST, zypper and the openSUSE/SLE implementation of PackageKit.

libzypp provides functionality for a package manager:

  * An API for package repository management, supporting most common
    repository metadata formats and signed repositories.
  * An API for solving packages, products, patterns and patches
    (installation, removal, update and distribution upgrade
    operations) dependencies, with additional features like locking.
  * An API for commiting the transaction to the system over a rpm
    target. Supporting deltarpm calculation, media changing and
    installation order calculation.
  * An API for browsing available and installed software, with some
    facilities for programs with an user interface.

%package devel
Summary:        Header files for libzypp, a library for package management
Group:          Development/Libraries/C and C++
Provides:       yast2-packagemanager-devel
Obsoletes:      yast2-packagemanager-devel
%if 0%{?suse_version} >= 1330
Requires:       libboost_headers-devel
Requires:       libboost_program_options-devel
Requires:       libboost_test-devel
Requires:       libboost_thread-devel
%else
Requires:       boost-devel
%endif
Requires:       bzip2
Requires:       glibc-devel
Requires:       libstdc++-devel
Requires:       libxml2-devel
Requires:       libzypp = %{version}
Requires:       pkgconfig(openssl)
Requires:       popt-devel
Requires:       rpm-devel > 4.4
Requires:       zlib-devel
Requires:       libudev-devel
%if 0%{?suse_version}
%if 0%{?suse_version} >= 1100
# Code11+
Requires:       libcurl-devel >= %{min_curl_version}
%else
# Code10
Requires:       curl-devel
%endif
%else
# Other distros (Fedora)
Requires:       libcurl-devel >= %{min_curl_version}
%endif
%if 0%{?suse_version} >= 1100
%requires_ge    libsolv-devel
%else
Requires:       libsolv-devel
%endif

%description devel
Development files for libzypp, a library for package, patch, pattern
and product management.

%package devel-doc
Summary:        Developer documentation for libzypp
Group:          Documentation/HTML

%description devel-doc
Developer documentation for libzypp.

%prep
%setup -q

%build
mkdir build
cd build
%if 0%{?suse_version} > 1500
# Bug 1189788 - UsrMerge: filesystem package breaks system when upgraded in a single rpm transaction
# While the bug is not fixed, we don't allow ZYPP_SINGLE_RPMTRANS=1 on a not UsrMerged system
export CFLAGS="%{optflags} -DNO_SINGLETRANS_USERMERGE"
export CXXFLAGS="%{optflags} -DNO_SINGLETRANS_USERMERGE"
%else
export CFLAGS="%{optflags}"
export CXXFLAGS="%{optflags}"
%endif
unset EXTRA_CMAKE_OPTIONS

cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} \
      -DENABLE_BUILD_DOCS=TRUE \
      -DENABLE_BUILD_TRANS=TRUE \
      -DENABLE_BUILD_TESTS=TRUE \
      -DDOC_INSTALL_DIR=%{_docdir} \
      -DLIB=%{_lib} \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_SKIP_RPATH=1 \
      -DCMAKE_INSTALL_LIBEXECDIR=%{_libexecdir} \
      %{?with_zchunk:-DENABLE_ZCHUNK_COMPRESSION=1} \
      %{?with_zstd:-DENABLE_ZSTD_COMPRESSION=1} \
      %{?with_sigc_block_workaround:-DENABLE_SIGC_BLOCK_WORKAROUND=1} \
      %{!?with_mediabackend_tests:-DDISABLE_MEDIABACKEND_TESTS=1} \
      %{?with enable_preview_single_rpmtrans_as_default_for_zypper:-DENABLE_PREVIEW_SINGLE_RPMTRANS_AS_DEFAULT_FOR_ZYPPER=1} \
      ${EXTRA_CMAKE_OPTIONS} \
      ..
make %{?_smp_mflags} VERBOSE=1

%install
cd build
%make_install
%if 0%{?fedora_version} || 0%{?rhel_version} >= 600 || 0%{?centos_version} >= 600
ln -s %{_sysconfdir}/yum.repos.d %{buildroot}/%{_sysconfdir}/zypp/repos.d
%else
mkdir -p %{buildroot}/%{_sysconfdir}/zypp/repos.d
%endif
mkdir -p %{buildroot}/%{_sysconfdir}/zypp/services.d
mkdir -p %{buildroot}/%{_sysconfdir}/zypp/systemCheck.d
mkdir -p %{buildroot}/%{_sysconfdir}/zypp/vars.d
mkdir -p %{buildroot}/%{_sysconfdir}/zypp/vendors.d
mkdir -p %{buildroot}/%{_sysconfdir}/zypp/multiversion.d
mkdir -p %{buildroot}/%{_sysconfdir}/zypp/needreboot.d
mkdir -p %{buildroot}/%{_sysconfdir}/zypp/credentials.d
mkdir -p %{buildroot}/%{_prefix}/lib/zypp
mkdir -p %{buildroot}/%{_prefix}/lib/zypp/plugins
mkdir -p %{buildroot}/%{_prefix}/lib/zypp/plugins/appdata
mkdir -p %{buildroot}/%{_prefix}/lib/zypp/plugins/commit
mkdir -p %{buildroot}/%{_prefix}/lib/zypp/plugins/services
mkdir -p %{buildroot}/%{_prefix}/lib/zypp/plugins/system
mkdir -p %{buildroot}/%{_prefix}/lib/zypp/plugins/urlresolver
mkdir -p %{buildroot}/%{_var}/lib/zypp
mkdir -p %{buildroot}/%{_var}/log/zypp
mkdir -p %{buildroot}/%{_var}/cache/zypp

# Default to 'solver.dupAllowVendorChange = false' on TW and post SLE12
%if 0%{?suse_version} >= 1330 || "%{distribution}" == "openSUSE Tumbleweed"
sed -i "s|# solver.dupAllowVendorChange = true|solver.dupAllowVendorChange = false|g" %{buildroot}%{_sysconfdir}/zypp/zypp.conf
%endif

cd ..

# Create filelist with translations
%{find_lang} zypp

%if %{defined _distconfdir}
# Move logratate files form /etc/logrotate.d to /usr/etc/logrotate.d
mkdir -p %{buildroot}/%{_distconfdir}/logrotate.d
mv %{buildroot}/%{_sysconfdir}/logrotate.d/zypp-history.lr %{buildroot}%{_distconfdir}/logrotate.d
%endif

%check
pushd build/tests
LD_LIBRARY_PATH="$(pwd)/../zypp:$LD_LIBRARY_PATH" ctest --output-on-failure .
popd

%if %{defined _distconfdir}
%pre
# Prepare for migration to /usr/etc; save any old .rpmsave
for i in logrotate.d/zypp-history.lr; do
   test -f %{_sysconfdir}/${i}.rpmsave && mv -v %{_sysconfdir}/${i}.rpmsave %{_sysconfdir}/${i}.rpmsave.old ||:
done
%endif

%if %{defined _distconfdir}
%posttrans
# Migration to /usr/etc, restore just created .rpmsave
for i in logrotate.d/zypp-history.lr; do
   test -f %{_sysconfdir}/${i}.rpmsave && mv -v %{_sysconfdir}/${i}.rpmsave %{_sysconfdir}/${i} ||:
done
%endif

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files -f zypp.lang
%defattr(-,root,root)
%if 0%{?suse_version} >= 1500
%license COPYING
%endif
%dir               %{_sysconfdir}/zypp
%if 0%{?fedora_version} || 0%{?rhel_version} >= 600 || 0%{?centos_version} >= 600
%{_sysconfdir}/zypp/repos.d
%else
%dir               %{_sysconfdir}/zypp/repos.d
%endif
%dir               %{_sysconfdir}/zypp/services.d
%dir               %{_sysconfdir}/zypp/systemCheck.d
%dir               %{_sysconfdir}/zypp/vars.d
%dir               %{_sysconfdir}/zypp/vendors.d
%dir               %{_sysconfdir}/zypp/multiversion.d
%config(noreplace) %{_sysconfdir}/zypp/needreboot
%dir               %{_sysconfdir}/zypp/needreboot.d
%dir               %{_sysconfdir}/zypp/credentials.d
%config(noreplace) %{_sysconfdir}/zypp/zypp.conf
%config(noreplace) %{_sysconfdir}/zypp/systemCheck
%if %{defined _distconfdir}
%{_distconfdir}/logrotate.d/zypp-history.lr
%else
%config(noreplace) %{_sysconfdir}/logrotate.d/zypp-history.lr
%endif
%dir               %{_var}/lib/zypp
%if "%{_libexecdir}" != "%{_prefix}/lib"
%dir               %{_libexecdir}/zypp
%endif
%dir %attr(750,root,root) %{_var}/log/zypp
%dir               %{_var}/cache/zypp
%{_prefix}/lib/zypp
%{_datadir}/zypp
%{_bindir}/*
%{_libexecdir}/zypp/zypp-rpm
%{_libdir}/libzypp*so.*
%doc %{_mandir}/man1/*.1.*
%doc %{_mandir}/man5/*.5.*

%files devel
%defattr(-,root,root)
%{_libdir}/libzypp.so
%{_datadir}/cmake/Modules/*
%{_includedir}/zypp
%{_includedir}/zypp-core
%{_includedir}/zypp-media
%{_includedir}/zypp-curl
%{_libdir}/pkgconfig/libzypp.pc
%{_libdir}/pkgconfig/libzypp.pc
%{_libdir}/pkgconfig/libzypp.pc

%files devel-doc
%defattr(-,root,root)
%{_docdir}/%{name}

%changelog
