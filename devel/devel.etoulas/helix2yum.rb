#!/usr/bin/ruby -w

require 'optparse'
require 'rexml/document'
include REXML


class Pkginfo
  attr_accessor :name, :op, :version
  def initialize name=nil, op=nil, version=nil
    @name    = name
    @op      = op
    @version = version
  end
  def to_s
    "name=#{@name} op=#{@op} version=#{@version}"
  end
end

class Provides < Pkginfo
  @@count = 0
  def initialize name=nil, op=nil, version=nil
    super
    @@count += 1
  end
  def Provides.count
    @@count
  end
  def to_s
    '[provides] ' + super
  end
end

class Conflicts < Pkginfo
  @@count = 0
  def initialize name=nil, op=nil, version=nil
    super
    @@count += 1
  end
  def Conflicts.count
    @@count
  end
  def to_s
    '[conflicts] ' + super
  end
end

class Requires < Pkginfo
  @@count = 0
  def initialize name=nil, op=nil, version=nil
    super
    @@count += 1
  end
  def Requires.count
    @@count
  end
  def to_s
    '[requires] ' + super
  end
end


# global defaults
show = false
of = nil
filename = nil


# parse command-line options
ARGV.options do |o|
  o.banner = 'Usage: ' + File.basename($0) + ' FILE [-s|-o YUMFILE]'
  o.separator 'Options:'
  o.on( '-s', '--show', 'output will be printed to stdout' ) { |s| show = true }
  o.on( '-o YUMFILE', '--outfile', 'specify the outputfile' ) { |out| of = out }
  o.separator '    without this parameter given, the outputfile is named <Helixfile>.yum.xml'
  o.separator ''
  o.on_tail( '-h', '--help', 'show this screen' ) { puts o; exit 0 }
end


# start exception-block
begin

ARGV.parse!

filename = ARGV[0] || raise( 'No Helixfile given.' )

if not File.exists?( filename )
  raise( 'File not found.' )
elsif not File.readable?( filename )
  raise( 'File not readable.' )
elsif File.zero?( filename )
  puts 'Nothing to do.'; exit 0
end

puts "[file] #{filename} (" + File.size(filename).to_s + " bytes)"
# use file to instanciate an XML-Object and close filehandle immediate
infile = File.open( filename )
input = Document.new infile

# without -o the output file is named like the inputfile with the ending .yum.xml
if not show and of.nil?
  outfile = filename.sub(/(^.*)(\.xml)/, '\1.yum.xml')
  outfile = File.new( outfile, 'w+' )
elsif not show and not of.nil?
  outfile = of
  outfile = File.new( outfile, 'w+' )
else
  outfile = nil
end


output = Document.new

pkg_name  = Array.new
md5sum    = Array.new
epoch     = Array.new
version   = Array.new
release   = Array.new
provides  = Array.new
requires  = Array.new
conflicts = Array.new

# get all needed elements
XPath.each( input, '//name'     ) { |elem| pkg_name.push elem.text }
XPath.each( input, '//md5sum'   ) { |elem| md5sum.push   elem.text }
XPath.each( input, '//epoch'    ) { |elem| epoch.push    elem.text }
XPath.each( input, '//version'  ) { |elem| version.push  elem.text }
XPath.each( input, '//release'  ) { |elem| release.push  elem.text }
# TODO provides
XPath.each( input, '//provides' ) do |elem|
  count_dep = 0
  #puts elem
  elem.elements.each('dep') do |dep|
    provides[count_dep] = Provides.new( dep.attributes['name'],
                                      dep.attributes['op'],
                                      dep.attributes['version'] )
    #puts dep.attributes['name']
    #print dep.attributes['op']
    #puts dep.attributes['version']
    count_dep += 1
  end
end
# TODO requires
#XPath.each( input, '//requires' ) { |elem| puts elem }
# TODO conflicts
#XPath.each( input, '//conflicts' ) { |elem| puts elem }


pkg_name.reverse!
md5sum.reverse!
epoch.reverse!
version.reverse!
release.reverse!
#provides.reverse!
#requires.reverse!
#conflicts.reverse!

    # debug
    puts provides

output << XMLDecl.new
output << Element.new( 'metadata' )
output.root.add_attribute( 'xmlns', 'http://linux.duke.edu/metadata/common' )
output.root.add_attribute( 'xmlns:rpm', 'http://linux.duke.edu/metadata/rpm' )
output.root.add_attribute( 'packages', pkg_name.length )


while !pkg_name.empty?
  pkg  = Element.new( 'package' )
   pkg.add_attribute( 'type', 'rpm' )
  name = Element.new( 'name' ).add_text pkg_name.pop
  arch = Element.new( 'arch' ).add_text 'i386'
  ver  = Element.new( 'version' )
   ver.add_attribute( 'epoch', epoch.pop )
   ver.add_attribute( 'ver', version.pop )
   ver.add_attribute( 'rel', release.pop )
  csum = Element.new( 'checksum' ).add_text md5sum.pop
   csum.add_attribute( 'pkgid', 'YES' )
   csum.add_attribute( 'type', 'md5sum' )

  pkg << name
  pkg << arch
  pkg << ver
#  pkg << csum   # not needed at the moment; helix uses md5, yum uses sha-1

  output.root << pkg
end


# write formated xml into file or stdout
if not outfile.nil?
  output.write( outfile, 0 )
else
  output.write( $stdout, 0 )
end
puts " "


rescue => exc
  STDERR.puts "E: #{exc.message}"
  exit 1
ensure
  infile.close
  outfile.close
end
