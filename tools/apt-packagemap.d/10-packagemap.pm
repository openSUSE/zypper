#included by zypper-aptitude

my %pkgmap=qw(
debianutils		which
dnsmasq-base		dnsmasq
iputils-ping		iputils
libegl1-mesa		Mesa-libEGL1
libldap2-dev		openldap2-devel
libsasl2-dev		cyrus-sasl-devel
libsdl1.2-dev		libSDL-devel
zlib1g-dev		zlib-devel
locate			findutils
mesa-common-dev		Mesa-devel
open-iscsi-utils	open-iscsi
pep8			python-pep8
pylint3			python3-pylint
python-cherrypy3	python-cherrypy
python-libvirt		libvirt-python
python-libxml2		libxml2-python
python-mysqldb		python-mysql
python-pysqlite2	python-pysqlite
vim-nox			vim-minimal
);

my %ospkgmap = (
suse => {qw(
	locate			findutils-locate
	openssh-server		openssh
	pylint			python-pylint
	python-cherrypy3	python-CherryPy
	python-migrate		python-sqlalchemy-migrate
	vim-nox			vim
	)},
"fedora|centos|rhel" => {qw(
	libldap2-dev		openldap-devel
	open-iscsi-utils	iscsi-initiator-utils
	python-mysqldb		MySQL-python
	python-pysqlite2	python-apsw
	)},
);

foreach my $os (keys(%ospkgmap)) {
	if(os_is($os)) {
		my $map = $ospkgmap{$os};
		foreach my $k (keys(%$map)) { $pkgmap{$k}=$map->{$k} }
	}
}

foreach(@ARGV) {
	my $p=$pkgmap{$_};
	if($p) {$_=$p}
}
