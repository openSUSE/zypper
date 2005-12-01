#!/usr/bin/ruby -w

require 'optparse'
require 'rexml/document'
include REXML

OUTFILE = 'foobar.xml'

# parse command-line options
ARGV.options do |o|
  o.banner = 'Usage: ' + File.basename($0) + ' FILE [-o YUMFILE]'
  o.separator 'Options:'
  o.on( '-s', '--show', 'output will be printed to stdout' ) { |s| $show = true }   # not good
  o.on( '-o YUMFILE', '--outfile', 'specify the outputfile' ) { |of| $of = of }
  o.separator '    without this parameter given, the outputfile is named <Helixfile>.yum.xml'
  o.separator ''
  o.on_tail( '-h', '--help', 'show this screen' ) { puts o; exit 0 }
end

# start exception-block
begin
ARGV.parse!

filename = ARGV[0] || raise( 'No Helixfile given.' )
puts "filename is #{filename}"
puts "show is #{$show}"
if true
end
puts "outfile is #{$of}"

if !File.exists?( filename )
  raise( 'File not found.' )
elsif !File.readable?( filename )
  raise( 'File not readable.' )
elsif File.zero?( filename )
  puts 'Nothing to do.'; exit 0
end

# puts "Opening file #{filename} (#{filename.size} bytes)"
printf( "-- Opening file %s (%d bytes) --\n", filename, filename.size )
infile = File.open( filename )

### use file to instanciate an XML-Object
input = Document.new infile

output = Document.new

pkg_name = Array.new
md5sum = Array.new
epoch = Array.new
version = Array.new
release = Array.new


XPath.each( input, '//name' ) { |elem| pkg_name.push elem.text }
XPath.each( input, '//md5sum' ) { |elem| md5sum.push elem.text }
XPath.each( input, '//epoch' ) { |elem| epoch.push elem.text }
XPath.each( input, '//version' ) { |elem| version.push elem.text }
XPath.each( input, '//release' ) { |elem| release.push elem.text }

pkg_name.reverse!
md5sum.reverse!
epoch.reverse!
version.reverse!
release.reverse!

output << XMLDecl.new
output << Element.new( 'metadata' )
output.root.add_attribute( 'xmlns', 'http://linux.duke.edu/metadata/common' )
output.root.add_attribute( 'xmlns:rpm', 'http://linux.duke.edu/metadata/rpm' )
output.root.add_attribute( 'packages', pkg_name.length )


while !pkg_name.empty?
  pkg = Element.new( 'package' )
  pkg.add_attribute( 'type', 'rpm' )
  name = Element.new( 'name' ).add_text pkg_name.pop
  arch = Element.new( 'arch' ).add_text 'i386'
  ver = Element.new( 'version' )
  ver.add_attribute( 'epoch', epoch.pop )
  ver.add_attribute( 'ver', version.pop )
  ver.add_attribute( 'rel', release.pop )
  csum = Element.new( 'checksum' ).add_text md5sum.pop
  csum.add_attribute( 'pkgid', 'YES' )
  csum.add_attribute( 'type', 'md5sum' )   # maybe it should be sha?

  pkg << name
  pkg << arch
  pkg << ver
  pkg << csum

  output.root << pkg
end

#output.write( $stdout, 0 )
puts " "

infile.close

rescue => exc
  STDERR.puts "E: #{exc.message}"
  exit 1
end