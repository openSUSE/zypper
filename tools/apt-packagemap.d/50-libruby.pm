foreach(@ARGV) {
        s/^lib(.*)-ruby$/rubygem-$1/
}
