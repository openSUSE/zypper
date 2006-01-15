#
# simple xml checker
#
#  better use '/usr/bin/xmllint' for larger files
#

require "rexml/document"
include REXML

if ARGV.size < 1
	STDERR.puts "Usage: check_xml.rb <packages.xml>"
	exit 1
end

x = Document.new( File.open( ARGV[0] ))
