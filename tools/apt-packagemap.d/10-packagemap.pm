#included by zypper-aptitude

my %pkgmap=qw(
dnsmasq-base		dnsmasq
iputils-ping		iputils
libldap2-dev		openldap2-devel
libsasl2-dev		cyrus-sasl-devel
zlib1g-dev		zlib-devel
locate			findutils-locate
open-iscsi-utils	open-iscsi
openssh-server		openssh
pep8			python-pep8
pylint			python-pylint
python-cherrypy3	python-CherryPy
python-cheetah		python-Cheetah
python-gflags		python-python-gflags
python-libvirt		libvirt-python
python-libxml2		libxml2-python
python-migrate		python-sqlalchemy-migrate
python-mysqldb		python-mysql
python-pysqlite2	python-pysqlite
vim-nox			vim
);

foreach(@ARGV) {
	my $p=$pkgmap{$_};
	if($p) {$_=$p}
}
