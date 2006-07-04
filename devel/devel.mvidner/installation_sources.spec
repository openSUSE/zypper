#
# spec file for package installation_sources
#
# Copyright (c) 2006 SUSE LINUX Products GmbH, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

# norootforbuild

Name:           installation_sources
BuildRequires:  libzypp-devel
License:        GPL
Group:          System/Packages
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Autoreqprov:    on
Version:        0.0.1
Summary:        Command Line Tool for libzypp Installation Sources
Release:        0
Source:         installation_sources.cc
Source1:	README
Prefix:         /usr

%description
Command Line Tool for libzypp Installation Sources.
Resurrection of a program that was in yast2-packagemanager.rpm.

Authors:
--------
    Martin Vidner <mvidner@suse.cz>

%prep
%setup -c -T
cp %SOURCE0 %SOURCE1 .

%build
g++ -o installation_sources installation_sources.cc -lzypp

%install
install -d ${RPM_BUILD_ROOT}%{prefix}/bin
install installation_sources ${RPM_BUILD_ROOT}%{prefix}/bin

%post

%postun

%clean

%files
%defattr(-,root,root)
%{prefix}/bin/installation_sources
%doc README

%changelog
* Tue Jul 04 2006 - mvidner@suse.cz
- Released in build.opensuse.org
