#!/usr/bin/ruby -w

###
#
# ToDos:
# - find out wether the operator != exists in Yum-dependencies or not
# - read and write checksum and type correct and set the flag to YES or NO
#
###
require 'optparse'
require 'rexml/document'
include REXML


class Dependency
  attr_accessor :name, :op, :version
  def initialize( dep_array )
    @name    = dep_array[0]
    @op      = dep_array[1]
    @version = dep_array[2]
  end
  def to_s
    "name=#{@name} op=#{@op} version=#{@version}\n"
  end
end

class Provides < Dependency
  @@count = 0
  def initialize( p_array )
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
  def initialize( c_array )
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
  def initialize( r_array )
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
    @chktype   = 'md5sum'   # Checksum type; not needed for now; helix uses md5, yum uses sha-1 TODO
    @pkgid     = 'NO'       # YES if chksum exists TODO
    @summary   = validate_input( pkg, 'summary', 'n/a' )
    @descr     = validate_input( pkg, 'description', 'n/a' )   # Package description
    # dependencies
    @provides  = Array.new
    @conflicts = Array.new
    @requires  = Array.new
    XPath.each( pkg, 'provides/dep'  ) { |p| @provides << Provides.new( conv_dependency(p) ) }
    XPath.each( pkg, 'conflicts/dep' ) { |c| @conflicts << Conflicts.new( conv_dependency(c) ) }
    XPath.each( pkg, 'requires/dep'  ) { |r| @requires << Requires.new( conv_dependency(r) ) }
  end

  def Package.count
    @@count
  end

  def Package.to_yum
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

  def conv_dependency( dep )
      dep_array = Array.new(3)
      dep_array[0] = dep.attributes['name']
      dep_array[2] = dep.attributes['version']
      case dep.attributes['op']
        when '='
          dep_array[1] = 'EQ'
        when '!='
          dep_array[1] = 'TODO'  # Have to check if this operator exists in yum TODO
        when '&gt;'
          dep_array[1] = 'GT'
        when '&ge;'
          dep_array[1] = 'GE'
        when '&lt;'
          dep_array[1] = 'LT'
        when '&le;'
          dep_array[1] = 'LE'
        else
          dep_array[1,2] = nil
        end
      dep_array
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

puts Package.count.to_s + ' packages processed.'


output << XMLDecl.new( '1.0', 'UTF-8' )
output << Element.new( 'metadata' )
output.root.add_namespace( 'http://linux.duke.edu/metadata/common' )
output.root.add_namespace( 'rpm', 'http://linux.duke.edu/metadata/rpm' )
output.root.add_attribute( 'packages', Package.count )


# generate the Yum document
packages.each do |p|
  pkg  = Element.new( 'package' )
   pkg.add_attribute( 'type', p.type )
  name = Element.new( 'name' ).add_text p.name
  arch = Element.new( 'arch' ).add_text p.arch
  ver  = Element.new( 'version' )
   ver.add_attribute( 'epoch', p.epoch )
   ver.add_attribute( 'ver', p.ver )
   ver.add_attribute( 'rel', p.rel )
  csum = Element.new( 'checksum' ).add_text p.chksum
   csum.add_attribute( 'pkgid', p.pkgid )
   csum.add_attribute( 'type', p.chktype )
  sum  = Element.new( 'summary' ).add_text p.summary
  desc = Element.new( 'description' ).add_text p.descr
  pker = Element.new( 'packager' ).add_text 'SUSE LINUX Products GmbH. &lt;http://bugzilla.novell.com&gt;'
  url  = Element.new( 'url' )
  time = Element.new( 'time' )
   time.add_attribute( 'file', 'TIMESTAMP' )
   time.add_attribute( 'build', 'TIMESTAMP' )
  size = Element.new( 'size' )
   size.add_attribute( 'package', 'SIZE' )
   size.add_attribute( 'installed', 'SIZE' )
   size.add_attribute( 'archive', 'SIZE' )
  loc  = Element.new( 'location' )
   loc.add_attribute( 'href', 'PATH/TO/PACKAGE-VERSION.rpm' )
  form = Element.new( 'format' )
    licence = Element.new( 'rpm:license' ).add_text 'GPL'
    vendor  = Element.new( 'rpm:vendor' ) .add_text 'Novell, Inc.'
    group = Element.new( 'rpm:group' )    .add_text 'SYSTEM/PACKAGEMANAGER'
    bhost = Element.new( 'rpm:buildhost' ).add_text 'BUILDHOST'
    srpm = Element.new( 'rpm:sourcerpm' ) .add_text 'PACKAGE-VERSION.src.rpm'
    hrange = Element.new( 'rpm:header-range' )
     hrange.add_attribute( 'start', 'VALUE' )
     hrange.add_attribute( 'end', 'VALUE' )

  # dependencies
   # provides
    prov = Element.new( 'rpm:provides' )
    p.provides.each do |provides|
       prov.add_element( 'rpm:entry', {
                                       'name'=>provides.name,
                                       'flags'=>provides.op,
                                       'ver'=>provides.version
                                      }
                       )
    end
   # conflicts
    conf = Element.new( 'rpm:conflicts' )
    p.conflicts.each do |conflicts|
      conf.add_element( 'rpm:entry', {
                                      'name'=>conflicts.name,
                                      'flags'=>conflicts.op,
                                      'ver'=>conflicts.version
                                     }
                      )
    end
   # requires
    requ = Element.new( 'rpm:requires' )
    p.requires.each do |requires|
      requ.add_element( 'rpm:entry', {
                                      'name'=>requires.name,
                                      'flags'=>requires.op,
                                      'ver'=>requires.version
                                     }
                      )
    end

    file = Element.new( 'file' ).add_text '/PATH/FILE'

    form << licence
    form << vendor
    form << group
    form << bhost
    form << srpm
    form << hrange
    form << prov
    form << conf
    form << requ
    form << file

  pkg << name
  pkg << arch
  pkg << ver
  pkg << csum
  pkg << sum
  pkg << desc
  pkg << pker
  pkg << url
  pkg << time
  pkg << size
  pkg << loc
  pkg << form

  output.root << pkg
end

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
  puts ''
end
#puts packages

# rescue => exc
#   STDERR.puts "E: #{exc.message}"
#   exit 1
ensure
  infile.close
end
