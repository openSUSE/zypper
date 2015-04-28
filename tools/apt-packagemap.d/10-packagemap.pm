#included by zypper-aptitude

my %pkgmap=qw(
dnsmasq-base		dnsmasq
iputils-ping		iputils
libldap2-dev		openldap2-devel
libsasl2-dev		cyrus-sasl-devel
zlib1g-dev		zlib-devel
locate			findutils-locate
open-iscsi-utils	open-iscsi
pep8			python-pep8
python-cherrypy3	python-CherryPy
python-libvirt		libvirt-python
python-libxml2		libxml2-python
python-mysqldb		python-mysql
python-pysqlite2	python-pysqlite
vim-nox			vim
);

my %ospkgmap = (
suse => {qw(
	openssh-server		openssh
	pylint			python-pylint
	python-migrate		python-sqlalchemy-migrate
	)},
"fedora|centos|rhel" => {qw(
	libldap2-dev		openldap-devel
	locate			findutils
	open-iscsi-utils	iscsi-initiator-utils
	python-cherrypy3	python-cherrypy
	python-mysqldb		MySQL-python
	python-pysqlite2	python-apsw
	vim-nox			vim-minimal
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
