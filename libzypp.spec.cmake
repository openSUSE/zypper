#
# spec file for package libzypp
#
# Copyright (c) 2007 SUSE LINUX Products GmbH, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

# norootforbuild

Name:           @PACKAGE@
License:        GPL v2 or later
Group:          System/Packages
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Autoreqprov:    on
Summary:        Package, Patch, Pattern, and Product Management
Version:        @VERSION@
Release:        0
Source:         %{name}-%{version}.tar.bz2
Source1:        %{name}-rpmlintrc
Prefix:         /usr
Provides:       yast2-packagemanager
Obsoletes:      yast2-packagemanager
Recommends:     logrotate
BuildRequires:  cmake
BuildRequires:  libsatsolver-devel >= 0.14.9
BuildRequires:  openssl-devel
BuildRequires:  boost-devel curl-devel dejagnu doxygen gcc-c++ gettext-devel graphviz hal-devel libxml2-devel

# required for testsuite, webrick
BuildRequires:  ruby
BuildRequires: aria2 >= 1.1.2

%if 0%{?suse_version}
BuildRequires:  libexpat-devel
%else
BuildRequires:  expat-devel
%endif

%if 0%{?suse_version}
BuildRequires:  hicolor-icon-theme update-desktop-files rpm-devel
Requires: uuid-runtime
%endif

%if 0%{?fedora_version}
BuildRequires: glib2-devel popt-devel dbus-glib-devel rpm-devel
%endif

%if 0%{?mandriva_version}
BuildRequires:  glib2-devel
BuildRequires:  librpm-devel
# uuidgen
Requires: e2fsprogs
%endif

%if 0%{?suse_version}
Requires:       gpg2
%else
Requires:       gnupg
%endif
%requires_eq    satsolver-tools

Requires:       aria2 >= 1.1.2

%description
Package, Patch, Pattern, and Product Management

Authors:
--------
    Michael Andres <ma@suse.de>
    Jiri Srain <jsrain@suse.cz>
    Stefan Schubert <schubi@suse.de>
    Duncan Mac-Vicar <dmacvicar@suse.de>
    Klaus Kaempf <kkaempf@suse.de>
    Marius Tomaschewski <mt@suse.de>
    Stanislav Visnovsky <visnov@suse.cz>
    Ladislav Slezak <lslezak@suse.cz>

%package devel
Requires:       libzypp == %{version}
Requires:       libxml2-devel curl-devel openssl-devel rpm-devel glibc-devel zlib-devel
Requires:       bzip2 popt-devel dbus-1-devel glib2-devel hal-devel boost-devel libstdc++-devel
Requires:       cmake libsatsolver-devel >= 0.13.0
Summary:        Package, Patch, Pattern, and Product Management - developers files
Group:          System/Packages
Provides:       yast2-packagemanager-devel
Obsoletes:      yast2-packagemanager-devel

%description -n libzypp-devel
Package, Patch, Pattern, and Product Management - developers files

Authors:
--------
    Michael Andres <ma@suse.de>
    Jiri Srain <jsrain@suse.cz>
    Stefan Schubert <schubi@suse.de>
    Duncan Mac-Vicar <dmacvicar@suse.de>
    Klaus Kaempf <kkaempf@suse.de>
    Marius Tomaschewski <mt@suse.de>
    Stanislav Visnovsky <visnov@suse.cz>
    Ladislav Slezak <lslezak@suse.cz>

%prep
%setup -q

%build
mkdir build
cd build
export CFLAGS="$RPM_OPT_FLAGS"
export CXXFLAGS="$CFLAGS"
cmake -DCMAKE_INSTALL_PREFIX=%{prefix} \
      -DDOC_INSTALL_DIR=%{_docdir} \
      -DLIB=%{_lib} \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_SKIP_RPATH=1 \
      ..
make %{?jobs:-j %jobs} VERBOSE=1
make -C doc/autodoc %{?jobs:-j %jobs}
make -C po %{?jobs:-j %jobs} translations

%if 0%{?run_testsuite}
  make -C tests %{?jobs:-j %jobs}
  pushd tests
  LD_LIBRARY_PATH=$PWD/../zypp:$LD_LIBRARY_PATH ctest .
  popd
%endif

#make check

%install
rm -rf "$RPM_BUILD_ROOT"
cd build
make install DESTDIR=$RPM_BUILD_ROOT
make -C doc/autodoc install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/etc/zypp/repos.d
mkdir -p $RPM_BUILD_ROOT/etc/zypp/services.d
mkdir -p $RPM_BUILD_ROOT/%{_usr}/lib/zypp
mkdir -p $RPM_BUILD_ROOT/%{_var}/lib/zypp
mkdir -p $RPM_BUILD_ROOT/%{_var}/log/zypp
mkdir -p $RPM_BUILD_ROOT/%{_var}/cache/zypp

%if 0%{?suse_version}
%suse_update_desktop_file -G "" -C "" package-manager
%endif

make -C po install DESTDIR=$RPM_BUILD_ROOT
# Create filelist with translations
cd ..
%{find_lang} zypp


%post
%run_ldconfig
if [ -f /var/cache/zypp/zypp.db ]; then rm /var/cache/zypp/zypp.db; fi

# convert old lock file to new
# TODO make this a separate file?
# TODO run the sript only when updating form pre-11.0 libzypp versions
LOCKSFILE=/etc/zypp/locks
OLDLOCKSFILE=/etc/zypp/locks.old

is_old(){
  # if no such file, exit with false (1 in bash)
  test -f ${LOCKSFILE} || return 1
  TEMP_FILE=`mktemp`
  cat ${LOCKSFILE} | sed '/^\#.*/ d;/.*:.*/d;/^[^[a-zA-Z\*?.0-9]*$/d' > ${TEMP_FILE}
  if [ -s ${TEMP_FILE} ]
  then
    RES=0
  else
    RES=1
  fi
  rm -f ${TEMP_FILE}
  return ${RES}
}

append_new_lock(){
  case "$#" in
    1 )
  echo "
solvable_name: $1
match_type: glob
" >> ${LOCKSFILE}
;;
    2 ) #TODO version
  echo "
solvable_name: $1
match_type: glob
version: $2
" >> ${LOCKSFILE}
;;
    3 ) #TODO version
  echo "
solvable_name: $1
match_type: glob
version: $2 $3
" >> ${LOCKSFILE}
  ;;
esac
}

die() {
  echo $1
  exit 1
}

if is_old ${LOCKSFILE}
  then
  mv -f ${LOCKSFILE} ${OLDLOCKSFILE} || die "cannot backup old locks"
  cat ${OLDLOCKSFILE}| sed "/^\#.*/d"| while read line
  do
    append_new_lock $line
  done
fi


%postun
%run_ldconfig

%clean
rm -rf "$RPM_BUILD_ROOT"

%files -f zypp.lang
%defattr(-,root,root)
%dir               /etc/zypp
%dir               /etc/zypp/repos.d
%dir               /etc/zypp/services.d
%config(noreplace) /etc/zypp/zypp.conf
%config(noreplace) /etc/zypp/systemCheck
%config(noreplace) %{_sysconfdir}/logrotate.d/zypp-history.lr
                   %{_usr}/lib/zypp
%dir               %{_var}/lib/zypp
%dir               %{_var}/log/zypp
%dir               %{_var}/cache/zypp
%dir               %{prefix}/lib/zypp
%{prefix}/share/zypp
%{prefix}/share/applications/package-manager.desktop
%{prefix}/share/icons/hicolor/scalable/apps/package-manager-icon.svg
%{prefix}/share/icons/hicolor/16x16/apps/package-manager-icon.png
%{prefix}/share/icons/hicolor/22x22/apps/package-manager-icon.png
%{prefix}/share/icons/hicolor/24x24/apps/package-manager-icon.png
%{prefix}/share/icons/hicolor/32x32/apps/package-manager-icon.png
%{prefix}/share/icons/hicolor/48x48/apps/package-manager-icon.png
%{prefix}/bin/*
%{_libdir}/libzypp*so.*
%doc %_mandir/man5/locks.5.*

%files devel
%defattr(-,root,root)
%{_libdir}/libzypp.so
%{_docdir}/%{name}
%{prefix}/include/zypp
%{prefix}/share/cmake/Modules/*
%{_libdir}/pkgconfig/libzypp.pc

%changelog
