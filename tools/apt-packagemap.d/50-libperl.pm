foreach(@ARGV) {
        if(s/^lib(.*)-perl$/perl-$1/) {
		# find out correct upper/lower case
		my $xml=`zypper --xmlout search $_`;
		if($xml=~m/ name="($_)" summary=/i) {
			$_=$1;
		}
	}
}
