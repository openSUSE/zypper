#
# spec file for package zypper.spec
#
# Copyright (c) 2025 SUSE LLC
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#


Name:           zypper
Version:        @VERSION@
Release:        0
Source:         %{name}-%{version}.tar.bz2
Source1:        %{name}-rpmlintrc
URL:            https://github.com/openSUSE/zypper
BuildRequires:  augeas-devel >= 1.10.0
%if 0%{?suse_version} > 1325
BuildRequires:  libboost_headers-devel
%else
BuildRequires:  boost-devel >= 1.33.1
%endif
BuildRequires:  cmake >= 3.10
BuildRequires:  gcc-c++ >= 7
BuildRequires:  gettext-devel >= 0.15
BuildRequires:  libzypp-devel >= 17.37.12
BuildRequires:  readline-devel >= 5.1
BuildRequires:  libxml2-devel

# required for documentation
BuildRequires:  rubygem(asciidoctor)

# TUI library which is built and shipped with libzypp-devel
BuildRequires:  libzypp-tui-devel >= 1

Summary:        Command line software manager using libzypp
License:        GPL-2.0-or-later
Group:          System/Packages

Provides:       y2pmsh
Obsoletes:      y2pmsh
Provides:       zypper(auto-agree-with-product-licenses)
Provides:       zypper(oldpackage)
Provides:       zypper(updatestack-only)
Provides:       zypper(purge-kernels)
Provides:       zypper(include-all-archs)
%if 0%{?suse_version}
Requires:       libaugeas0 >= 1.10.0
%requires_ge    libzypp
Recommends:     logrotate
Recommends:     zypper-log
%else
Requires:       augeas >= 1.10.0
%endif


%description
Zypper is a command line tool for managing software. It can be used to add
package repositories, search for packages, install, remove, or update packages,
install patches, hardware drivers, verify dependencies, and more.

Zypper can be used interactively or non-interactively by user, from scripts,
or front-ends.

%package log
Summary:        CLI for accessing the zypper logfile
Group:          System/Packages
Requires:       %{_bindir}/awk
Requires:       %{_bindir}/grep
Requires:       /bin/bash
BuildArch:      noarch

%description log
CLI for accessing the zypper logfile

%package mark
Summary:        CLI for accessing the autoinstall flag
Group:          System/Packages
Requires:       %{_bindir}/perl
Supplements:    zypper
BuildArch:      noarch

%description mark
CLI for accessing the zypper autoinstall flag


%package aptitude
Summary:        aptitude compatibility with zypper
Group:          System/Packages
Requires:       perl
Requires:       zypper
BuildArch:      noarch

%description aptitude
provides compatibility to Debian's aptitude command using zypper

%package needs-restarting
Summary:        needs-restarting compatibility with zypper
Group:          System/Packages
Requires:       zypper
BuildArch:      noarch
%if 0%{?suse_version}
Supplements:    zypper
%endif

%description needs-restarting
provides compatibility to YUM needs-restarting command using zypper

%prep
%autosetup -p1

%build
mkdir -p build
cd build

CMAKE_FLAGS=

%if 0%{?suse_version} == 1500 && 0%{?sle_version} && 0%{?sle_version} <= 150100
  # Fixed in 1.14.34: Do not allow the abbreviation of cli arguments (bsc#1164543)
  # On SLE15/Leap 15.0 and 15.1 we will stay bug-compatible and accept the
  # abbreviations in order not to break tools. In 15.2 they must be fixed.
  CMAKE_FLAGS="$CMAKE_FLAGS -DLEGACY_ENABLE_LONGOPT_ABBREV=1"
%endif

export CFLAGS="%{optflags}"
export CXXFLAGS="%{optflags}"
export LDFLAGS="-Wl,--as-needed -fpie %{optflags}"

cmake $CMAKE_FLAGS \
      -DCMAKE_INSTALL_PREFIX=%{_prefix} \
      -DSYSCONFDIR=%{_sysconfdir} \
      -DMANDIR=%{_mandir} \
      -DCMAKE_VERBOSE_MAKEFILE=TRUE \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DENABLE_BUILD_TESTS=ON \
      ..

#gettextize -f
%make_build

%check
cd build/tests
ctest --output-on-failure .

%install
cd build
%make_install

mkdir -p %{buildroot}%{_prefix}/lib/zypper
mkdir -p %{buildroot}%{_prefix}/lib/zypper/commands

# yzpper symlink
ln -s zypper %{buildroot}%{_bindir}/yzpper

# Create filelist with translations
cd ..
%find_lang zypper
install -d -m755 %{buildroot}%{_var}/log
touch %{buildroot}%{_var}/log/zypper.log

%if %{defined _distconfdir}
# Move logratate files form /etc/logrotate.d to /usr/etc/logrotate.d
mkdir -p %{buildroot}/%{_distconfdir}/logrotate.d
mv %{buildroot}/%{_sysconfdir}/logrotate.d/zypper.lr %{buildroot}%{_distconfdir}/logrotate.d
mv %{buildroot}/%{_sysconfdir}/logrotate.d/zypp-refresh.lr %{buildroot}%{_distconfdir}/logrotate.d
%endif

%if %{defined _distconfdir}
%pre
# Prepare for migration to /usr/etc; save any old .rpmsave
for i in logrotate.d/zypper.lr logrotate.d/zypp-refresh.lr; do
   test -f %{_sysconfdir}/${i}.rpmsave && mv -v %{_sysconfdir}/${i}.rpmsave %{_sysconfdir}/${i}.rpmsave.old ||:
done
%endif

%if %{defined _distconfdir}
%posttrans
# Migration to /usr/etc, restore just created .rpmsave
for i in logrotate.d/zypper.lr logrotate.d/zypp-refresh.lr; do
   test -f %{_sysconfdir}/${i}.rpmsave && mv -v %{_sysconfdir}/${i}.rpmsave %{_sysconfdir}/${i} ||:
done
%endif

%files -f zypper.lang
%if 0%{?suse_version} >= 1500
%license COPYING
%endif
%config(noreplace) %{_sysconfdir}/zypp/zypper.conf
%if %{defined _distconfdir}
%{_distconfdir}/logrotate.d/zypper.lr
%{_distconfdir}/logrotate.d/zypp-refresh.lr
%else
%config(noreplace) %{_sysconfdir}/logrotate.d/zypper.lr
%config(noreplace) %{_sysconfdir}/logrotate.d/zypp-refresh.lr
%endif
%{_datadir}/bash-completion/completions/zypper
%{_bindir}/zypper
%{_bindir}/yzpper
%{_bindir}/installation_sources
%{_sbindir}/zypp-refresh
%dir %{_datadir}/zypper
%{_datadir}/zypper/zypper.aug
%dir %{_datadir}/zypper/xml
%{_datadir}/zypper/xml/xmlout.rnc
%{_prefix}/lib/zypper
%{_mandir}/man8/zypper.8%{?ext_man}
%{_mandir}/man8/zypp-refresh.8%{?ext_man}
%doc %dir %{_docdir}/zypper
%doc %{_docdir}/zypper/HACKING
# declare ownership of the log file but prevent
# it from being erased by rpm -e
%ghost %config(noreplace) %attr (640,root,root) %{_var}/log/zypper.log

%files log
%{_sbindir}/zypper-log
%{_mandir}/man8/zypper-log.8%{?ext_man}

%files mark
%{_sbindir}/zypper-mark

%files aptitude
%{_bindir}/aptitude
%{_bindir}/apt-get
%{_bindir}/apt
%dir %{_sysconfdir}/zypp/apt-packagemap.d/
%config(noreplace) %{_sysconfdir}/zypp/apt-packagemap.d/*

%files needs-restarting
%{_bindir}/needs-restarting
%{_mandir}/man1/needs-restarting.1%{?ext_man}

%changelog
