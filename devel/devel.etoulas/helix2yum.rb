#!/usr/bin/ruby -w

require 'rexml/document'
include REXML

FILENAME = 'test.xml'
OUTFILE = 'foobar.xml'

if !File.exists?( FILENAME )
  puts 'File not found.'
  exit 1
elsif !File.readable?( FILENAME )
  puts 'File not readable.'
  exit 1
elsif File.zero?( FILENAME )
  puts 'Nothing to do'
  exit 0
end

# puts "Opening file #{FILENAME} (#{FILENAME.size} bytes)"
printf( "Opening file %s (%d bytes)\n", FILENAME, FILENAME.size )
file = File.open( FILENAME )

### use file to instanciate an XML-Object
doc = Document.new file

### Finds and returns the first node that matches the supplied xpath
# puts XPath.first( doc, '//name' )

### Iterates over nodes that match the given path, calling the supplied block with the match.
a = XPath.each( doc, '//name' ) { |elem| puts elem.text }
printf( "elements matching: %d\n", a.length )

### Returns an array of nodes matching a given XPath
#a = XPath.match( doc, '//name' )
#printf( "%d elements matching.\n", a.length )
#puts a   # <name>G</name> ...

# get Attributes
# doc.elements.each( 'channel/subchannel/package' ) { |element| puts element.attributes['name'] }

#===============================================================================

file2 = File.new( OUTFILE, 'w+' )
doc2 = Document.new

elem = Element.new "foo"

elem2 = elem.add_element "bar", { "attrib"=>"2" }
elem2.text = "this is my text"

elem2 = elem.add_element "hola"
elem2.text = "amigo"

doc2.add_element elem
doc2.write( file2, 0 )

file.close
file2.close
