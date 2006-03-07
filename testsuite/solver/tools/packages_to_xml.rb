#!/usr/bin/ruby
#
# packages_to_xml.rb
#
# reads e.g. /mounts/dist/next-i386/suse/setup/descr/packages
#
# run: ruby packages_to_xml.rb packages > packages.xml
#
# Implementation note:
#  The output is generated via plain print/puts calls
#  The initial version used rexml but this was a memory hog
#  Since xml builds up the complete tree in memory before
#  actually writing it to a file, parsing a 6MB 'packages'
#  file produced a too large in-memory xml tree :-(
#
require 'fileutils'

class Dependency
	attr_accessor :name, :op, :epoch, :version, :release, :pre

	# parse "name op e:v-r"
	#  evra = epoch, version, release
	def initialize str, pre
		@pre = pre

		splitted = str.split " "
		if (splitted.size < 2)						# just a name
			@name = str
			return
		end
		@name = splitted[0]

		raise( "bad dependency [#{str}]" ) unless splitted.size == 3	# ensure <op> and <evra>

		@op = splitted[1]
		evr = splitted[2].split "-"					# check if <version>-<release>

		if evr.size < 2							# just <version>
			@version = splitted[2]
			return
		end

		ev = evr[0].split ":"						# check if <epoch>:<version>
		if ev.size < 2
			@version = evr[0]
		else
			@epoch = ev[0]
			@version = ev[1]
		end
	end

	def inspect
		return "<dep name='#{@name}' op='#{@op}' version='#{version}' release='#{release}'>"
	end

	def to_xml
		out = "<dep name='#{@name}'"
		unless @op.nil?
			case @op
				when "=": op = @op
				when "<": op = "&lt;"
				when ">": op = "&gt;"
				when "<=": op = "&lt;="
				when ">=": op = "&gt;="
				else
					STDERR.puts "Unkown dep operation '#{@op}'"
			end
			out = out + " op='#{op}' version='#{@version}'" unless op.nil?
			out = out + " release='#{@release}" unless @release.nil?
		end
		out += " pre='1'" if @pre
		return out + " />"
	end
end

class Package
	attr_accessor :name, :epoch, :version, :release, :architecture
	attr_accessor :summary, :description, :group, :rpmsize, :installsize
	attr_accessor :requires, :prerequires, :provides, :conflicts, :obsoletes, :suggests, :enhances
	attr_accessor :location
	attr_accessor :skip

	def initialize( stream )
		@channel = stream
	end

	def add_element name, value, indent = "  "
		@channel.puts "#{indent}<#{name}>#{value}</#{name}>" unless value.nil?
	end

	def add_dependencies name, deps
		return if deps.nil?
		@channel.puts "  <#{name}>"
		deps.each { |dep|
		    @channel.puts "    #{dep.to_xml}"
		}
		@channel.puts "  </#{name}>"
	end

	# save as xml to channel
	def save
		return if skip

		@channel.puts "<package>"

		add_element "name", name
		add_element "summary", summary
		add_element "description", description
		add_element "section", "system"				#group.gsub( %r{/}, "_" )
		add_element "location", location

		@channel.puts "  <history>"
		@channel.puts "    <update>"

		add_element "hid", "12345", "      "
		add_element "epoch", epoch, "      "
		add_element "version", version, "      "
		add_element "release", release, "      "
		add_element "arch", architecture, "      "

		@channel.puts "    </update>"
		@channel.puts "  </history>"

		add_dependencies "requires", (prerequires + requires)
		add_dependencies "provides", provides
		add_dependencies "conflicts", conflicts
		add_dependencies "obsoletes", obsoletes
		add_dependencies "suggests", suggests
		add_dependencies "enhances", enhances

		@channel.puts "</package>"
	end

end


class Parser

	#
	# parse key, value from file f
	#
	def parse_key_value(f)
		l = ""
		loop do
			return nil if f.eof?
			l = f.gets.chomp
			next if l.size == 0
			case l[0,1]
				when "#": next			# skip comment lines
				when "+":  break
				when "-":  break
				when "=":  break
				else return l
			end
		end
		key,value = l.split /:/
		value.strip! unless value.nil?
		return key, value if key[0,1] == "="
		return key, ""
	end


	#
	# parse_NEVRA - parse name, epoch, version, release, architecture from string
	#
	def parse_NEVRA( str )
		name,version,release,arch = str.split " "
		if version.include? ":"		# an epoch ?
			epoch,version = version.split ":"
		else
			epoch = nil
		end
#STDERR.print "#{str}                                            "
#STDERR.print "\r"
#STDERR.flush
		return name,epoch,version,release,arch
	end

	#
	# parse strings from f until endkey is encountered
	#

	def parse_array ( f, endkey )
		values = Array.new
		loop do
			key, value = parse_key_value f
			raise "EOF during dependency parse" if key.nil?			# error
			break if key == endkey
			values << key
		end
		return nil if values.empty?
		return values
	end

	#
	# parse dependencies from f until endkey is encountered
	#

	def parse_dependencies( f, endkey, is_pre = false )
		values = parse_array f, endkey
		return if values.nil?
		deps = Array.new
		values.each { |dep|
			deps << Dependency.new( dep, is_pre ) unless (dep[0,7] == "rpmlib(")
		}
		return deps
	end

	#
	# parsePackages
	# parse a packages file
	# return number of packages
	#

	def parse_packages( filename, stream, packageFile )
		pcount = 0
		package = nil
		packageList = []
		packageFileExist = false
		
		if File.file?(packageFile) then
			IO.foreach(packageFile) {|line| 
				line = line.rstrip
				packageList.push(line) }
			packageFileExist = true
		end

		stream.puts "<channel><subchannel>"

		STDERR.puts "Parsing #{filename}"

		begin	# rescue block
			buildfrom = nil					# src/nosrc package
			File.open( filename ) do |f|
				if f.gets.chomp != "=Ver: 2.0"
					raise "Wrong version"
				end
				skip = false;
				loop do
					key, value = parse_key_value f
					break if key.nil?			# error
					next if value.nil?			# no key
					case key
						when "=Pkg"
							pcount += 1
							if !package.nil?  then
								writeEntry = false
								if packageFileExist then
									# Searching for matched packages
									packageList.each do | packName |
										fit = packName.index(package.name)
										writeEntry = true if fit != nil
									end
								else
									writeEntry = true	
								end
								#saving
								package.save unless !writeEntry
							end

							package = Package.new stream
							skip = false;
							package.name,package.epoch,package.version,package.release,package.architecture = parse_NEVRA value
							package.skip = true if package.architecture == "src" || package.architecture == "nosrc"

						when "=Grp"
							package.group = value
						when "=Siz"
							package.rpmsize, package.installsize = value.split " "
						when "=Src"
							# name,epoch,version,release,architecture = parse_NEVRA value
						when "=Lic"
							# package.license = get_license value
						when "=Loc"
							package.location = value.split(" ")[0]
						when "=Tim"
							# package.buildtime
						when "=Shr"
							# package.shared
						when "+Aut"
							parse_array f, "-Aut"
						when "+Req"
							package.requires = parse_dependencies f, "-Req"
						when "+Prq"
							package.prerequires = parse_dependencies f, "-Prq", true
						when "+Prv"
							package.provides = parse_dependencies f, "-Prv"
						when "+Con"
							package.conflicts = parse_dependencies f, "-Con"
						when "+Obs"
							package.obsoletes = parse_dependencies f, "-Obs"
						when "+Sug"
							package.suggests = parse_dependencies f, "-Sug"
						when "+Enh"
							package.enhances = parse_dependencies f, "-Enh"
						else
							STDERR.puts "Unknown '#{key}'"

					end
				end # loop
				if !package.nil? then
					writeEntry = false
					if File.file?(packageFile) then
						# Searching for matched packages
						packageList.each do | packName |
							fit = packName.index(package.name)
							writeEntry = true if fit != nil
						end
					else
						writeEntry = true	
					end
					#saving
					package.save unless !writeEntry
				end
			end # File.open()
		rescue
			STDERR.puts "Ooops!"
			raise
		end
		stream.puts "</subchannel></channel>"

		return pcount
	end
end


def usage
	STDERR.puts "Usage: packages_to_xml.rb <packages-file> [<package-list-file>]"
	STDERR.puts "<package-list-file> is a file containing package names for which <packages-file>"
	STDERR.puts "will be scanned."
	exit 1
end

usage unless ARGV.size == 1 || ARGV.size == 2

parser = Parser.new
if ARGV.size == 1 then
	parser.parse_packages ARGV[0], STDOUT, ""
else
	parser.parse_packages ARGV[0], STDOUT, ARGV[1]
end