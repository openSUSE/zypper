#!/usr/bin/ruby -w

require 'rexml/document'
include REXML

FILENAME = 'test.xml'

if !File.exists?( FILENAME )
  puts 'File not found.'
  exit 1
elsif !File.readable?( FILENAME )
  puts 'File not readable.'
  exit 1
end

# puts "Opening file #{FILENAME} (#{FILENAME.size} bytes)"
printf( "Opening file %s (%d bytes)\n", FILENAME, FILENAME.size )
file = File.open( FILENAME )

# use file to instanciate an XML-Object
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

file.close
