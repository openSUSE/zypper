foreach(@ARGV) {
        if(m/^python-/) {
		# find out correct upper/lower case
		my $xml=`zypper --xmlout search $_`;
		if($xml=~m/ name="((?:python-)?$_)" summary=/i) {
			$_=$1;
		}
	}
}
