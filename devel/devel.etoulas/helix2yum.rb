#!/usr/bin/ruby -w

require 'rexml/document'
include REXML

FILENAME = 'helix.xml'
OUTFILE = 'foobar.xml'

if !File.exists?( FILENAME )
  puts 'File not found.'
  exit 1
elsif !File.readable?( FILENAME )
  puts 'File not readable.'
  exit 1
elsif File.zero?( FILENAME )
  puts 'Nothing to do.'
  exit 0
end

# puts "Opening file #{FILENAME} (#{FILENAME.size} bytes)"
printf( "-- Opening file %s (%d bytes) --\n", FILENAME, FILENAME.size )
infile = File.open( FILENAME )

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

output.write( $stdout, 0 )
puts " "

infile.close

### provides
#input.elements.each("*/*/package") { |elem| puts elem.elements["provides"] }

### Finds and returns the first node that matches the supplied xpath
# puts XPath.first( doc, '//name' )

### Iterates over nodes that match the given path, calling the supplied block with the match.
#a = XPath.each( input, '//name' ) { |elem| puts elem.text }

### Returns an array of nodes matching a given XPath
#a = XPath.match( doc, '//name' )
#printf( "%d elements matching.\n", a.length )
#puts a   # <name>G</name> ...

# get Attributes
# doc.elements.each( 'channel/subchannel/package' ) { |element| puts element.attributes['name'] }

#===============================================================================

# file2 = File.new( OUTFILE, 'w+' )
# doc2 = Document.new
# 
# elem = Element.new "foo"
# 
# elem2 = elem.add_element "bar", { "attrib"=>"2" }
# elem2.text = "this is my text"
# 
# elem2 = elem.add_element "hola"
# elem2.text = "amigo"
# 
# doc2.add_element elem
# doc2.write( file2, 0 )
# 
# file2.close
