#!/usr/bin/ruby -w

require 'optparse'
require 'rexml/document'
include REXML


class Dependency
  attr_accessor :name, :op, :version
  def initialize( name=nil, op=nil, version=nil )
    @name    = name
    @op      = op
    @version = version
  end
  def to_s
    "<name>=#{@name} <op>=#{@op} <version>=#{@version}\n"
  end
end

class Provides < Dependency
  @@count = 0
  def initialize( name=nil, op=nil, version=nil )
    super
    @@count += 1
  end
  def Provides.count
    @@count
  end
  def to_s
    '      [p]  ' + super
  end
end

class Conflicts < Dependency
  @@count = 0
  def initialize( name=nil, op=nil, version=nil )
    super
    @@count += 1
  end
  def Conflicts.count
    @@count
  end
  def to_s
    '      [c]  ' + super
  end
end

class Requires < Dependency
  @@count = 0
  def initialize( name=nil, op=nil, version=nil )
    super
    @@count += 1
  end
  def Requires.count
    @@count
  end
  def to_s
    '      [r]  ' + super
  end
end

class Package
  attr_accessor :type, :name, :arch, :epoch, :ver, :rel, :chksum, :chktype, :pkgid, :summary, :descr, :provides, :conflicts, :requires

  @@count = 0

  def initialize( name )
    @name      = name
    @type      = 'rpm'
    @arch      = 'i386'
    @epoch     = 0
    @ver       = 1       # Version
    @rel       = 1       # Release
    @chksum    = 0       # Checksum
    @chktype   = 'md5'   # Checksum type; not needed for now; helix uses md5, yum uses sha-1
    @pkgid     = 'NO'    # YES if chksum exists
    @summary   = 'Summary n/a'
    @descr     = 'Description n/a'   # Package description
    @provides  = Array.new
    @conflicts = Array.new
    @requires  = Array.new
    @@count   += 1
  end

  def Package.count
    @@count
  end

  def add_provides( name, op, version )
    @provides << Provides.new( name, op, version )
  end

  def add_conflicts( name, op, version )
    @conflicts << Conflicts.new( name, op, version )
  end

  def add_requires( name, op, version )
    @requires << Requires.new( name, op, version )
  end

  def to_s
    "[Package]  <name>=#{@name}\n#{@provides}#{@conflicts}#{@requires}"
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

packages  = Array.new


# get all needed elements
current = XPath.first( input, '/channel/subchannel/package' )
while current != nil
  pkg = Package.new( current.elements['name'].text )

  # get dependencies
  XPath.each( current, 'provides/dep'  ) do |p|
    pkg.add_provides(  p.attributes['name'], p.attributes['op'], p.attributes['version'] )
  end
  XPath.each( current, 'conflicts/dep' ) do |c|
    pkg.add_conflicts( c.attributes['name'], c.attributes['op'], c.attributes['version'] )
  end
  XPath.each( current, 'requires/dep'  ) do |r|
    pkg.add_requires(  r.attributes['name'], r.attributes['op'], r.attributes['version'] )
  end

  # add current package to packages-array
  packages << pkg

  current = current.next_element
end

puts packages

#XPath.each( input, '//md5sum'   ) { |elem| md5sum.push   elem.text }
#XPath.each( input, '//epoch'    ) { |elem| epoch.push    elem.text }
#XPath.each( input, '//version'  ) { |elem| version.push  elem.text }
#XPath.each( input, '//release'  ) { |elem| release.push  elem.text }


output << XMLDecl.new
output << Element.new( 'metadata' )
output.root.add_attribute( 'xmlns', 'http://linux.duke.edu/metadata/common' )
output.root.add_attribute( 'xmlns:rpm', 'http://linux.duke.edu/metadata/rpm' )
output.root.add_attribute( 'packages', Package.count )


# while !pkg_name.empty?
#   pkg  = Element.new( 'package' )
#    pkg.add_attribute( 'type', 'rpm' )
#   name = Element.new( 'name' ).add_text pkg_name.pop
#   arch = Element.new( 'arch' ).add_text 'i386'
#   ver  = Element.new( 'version' )
#    ver.add_attribute( 'epoch', epoch.pop )
#    ver.add_attribute( 'ver', version.pop )
#    ver.add_attribute( 'rel', release.pop )
#   csum = Element.new( 'checksum' ).add_text md5sum.pop
#    csum.add_attribute( 'pkgid', 'YES' )
#    csum.add_attribute( 'type', 'md5sum' )
#
#   pkg << name
#   pkg << arch
#   pkg << ver
# #  pkg << csum   
#
#   output.root << pkg
# end


# write formated xml into file or stdout
if not outfile.nil?
  output.write( outfile, 0 )
  outfile.close
else
  output.write( $stdout, 0 )
end
puts ';-)'


rescue => exc
  STDERR.puts "E: #{exc.message}"
  exit 1
ensure
  infile.close
end
