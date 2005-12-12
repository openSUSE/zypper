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
    "name=#{@name} op=#{@op} version=#{@version}\n"
  end
end

class Provides < Dependency
  @@count = 0
  def initialize( name, op, version )
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
  def initialize( name, op, version )
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
  def initialize( name, op, version )
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

  def initialize( pkg )   # accepts a <package> section of Helix-XML
    @@count   += 1
    @name      = pkg.elements['name'] == nil ? raise( 'Missing package-name in Helix sourcefile.' ) : pkg.elements['name'].text
    @type      = 'rpm'
    @arch      = 'i386'
    @epoch     = validate_input( pkg, 'history/update/epoch',   '0'   )     # Epoch
    @ver       = validate_input( pkg, 'history/update/version', '1.0' )     # Version
    @rel       = validate_input( pkg, 'history/update/release', '1'   )     # Release
    @chksum    = validate_input( pkg, 'history/update/md5sum',  '0'   )     # Checksum
    @chktype   = 'md5sum'   # Checksum type; not needed for now; helix uses md5, yum uses sha-1
    @pkgid     = 'NO'       # YES if chksum exists
    @summary   = validate_input( pkg, 'summary', 'n/a' )
    @descr     = validate_input( pkg, 'description', 'n/a' )   # Package description
    # dependencies
    @provides  = Array.new
    @conflicts = Array.new
    @requires  = Array.new
    XPath.each( pkg, 'provides/dep'  ) do |p|
      add_provides(  p.attributes['name'], p.attributes['op'], p.attributes['version'] )
    end
    XPath.each( pkg, 'conflicts/dep' ) do |c|
      add_conflicts( c.attributes['name'], c.attributes['op'], c.attributes['version'] )
    end
    XPath.each( pkg, 'requires/dep'  ) do |r|
      add_requires(  r.attributes['name'], r.attributes['op'], r.attributes['version'] )
    end
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
    "[Package]  name=#{@name} \
    epoch=#{@epoch} version=#{@ver} release=#{@rel} \
    #{@chktype}=#{@chksum}
           summary:     #{@summary}
           description: #{@descr}\
           \n#{@provides}#{@conflicts}#{@requires}"
  end

  private
  def validate_input( pkg, xpath, default_val )
    pkg.elements[xpath] == nil || pkg.elements[xpath].text == nil ? default_val : pkg.elements[xpath].text
  end

end


# command-line options defaults
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


# get the first package
current = XPath.first( input, '/channel/subchannel/package' )
# and add it to  the packages-array - add every following package
while current != nil
  packages << Package.new( current )
  current = current.next_element
end


output << XMLDecl.new
output << Element.new( 'metadata' )
output.root.add_attribute( 'xmlns', 'http://linux.duke.edu/metadata/common' )
output.root.add_attribute( 'xmlns:rpm', 'http://linux.duke.edu/metadata/rpm' )
output.root.add_attribute( 'packages', Package.count )


# TODO
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
# #  pkg << csum   # not needed at the moment; helix uses md5, yum uses sha-1
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
puts packages
puts Package.count.to_s + ' packages processed.'

rescue => exc
  STDERR.puts "E: #{exc.message}"
  exit 1
ensure
  infile.close
end
