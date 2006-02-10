#!/usr/bin/ruby
#
# selections_to_xml.rb
#
# reads e.g. /mounts/dist/next-i386/suse/setup/descr/selections
#
# run: ruby selections_to_xml.rb selections > selections.xml
#
# Implementation note:
#  The output is generated via plain print/puts calls
#  The initial version used rexml but this was a memory hog
#  Since xml builds up the complete tree in memory before
#  actually writing it to a file, parsing a 6MB 'selections'
#  file produced a too large in-memory xml tree :-(
#

class Dependency
	attr_accessor :name, :op, :epoch, :version, :release, :pre, :kind

	# parse "name op e:v-r"
	#  evra = epoch, version, release
	def initialize str, kind, pre
		@pre = pre
		@kind = kind

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
		return "<dep kind='#{@kind}' name='#{@name}' op='#{@op}' version='#{version}' release='#{release}'>"
	end

	def to_xml
		out = "<dep kind='#{@kind}' name='#{@name}'"
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

class Selection
	attr_accessor :name, :epoch, :version, :release, :architecture
	attr_accessor :summary, :description, :group
	attr_accessor :requires, :prerequires, :provides, :conflicts, :obsoletes, :recommends, :packages
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

		@channel.puts "<selection>"

		add_element "name", name
		add_element "summary", summary
		add_element "description", description
		add_element "section", "system"				#group.gsub( %r{/}, "_" )

		@channel.puts "  <history>"
		@channel.puts "    <update>"

		add_element "hid", "12345", "      "
		add_element "epoch", epoch, "      "
		add_element "version", version, "      "
		add_element "release", release, "      "
		add_element "arch", architecture, "      "

		@channel.puts "    </update>"
		@channel.puts "  </history>"

		if (!prerequires.nil? && !requires.nil?)
			add_dependencies "requires", (prerequires + requires)
		else
			add_dependencies "requires", requires
		end
		add_dependencies "provides", provides
		add_dependencies "conflicts", conflicts
		add_dependencies "obsoletes", obsoletes
		if (recommends.nil?)
			add_dependencies "recommends", packages	unless packages.nil?
		else
			if (packages.nil?)
				add_dependencies "recommends", recommends
			else
				add_dependencies "recommends", (recommends + packages)
			end
		end
		@channel.puts "</selection>"
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

	def parse_dependencies( f, kind, endkey, is_pre = false )
		values = parse_array f, endkey
		return if values.nil?
		deps = Array.new
		values.each { |dep|
			deps << Dependency.new( dep, kind, is_pre ) unless (dep[0,7] == "rpmlib(")
		}
		return deps
	end

	#
	# parseSelections
	# parse a selections file
	# return number of selections
	#

	def parse_selections( filename, stream )
		pcount = 0
		selection = nil

		STDERR.puts "Parsing #{filename}"

		begin	# rescue block
			buildfrom = nil					# src/nosrc selection
			File.open( filename ) do |f|
				skip = false;
				loop do
					key, value = parse_key_value f
					break if key.nil?			# error
					next if value.nil?			# no key
					case key
						when "=Ver"
							raise "Wrong version '#{version}'" unless value = "4.0"
						when "=Sel"
							pcount += 1
							selection.save unless selection.nil?
							selection = Selection.new stream
							skip = false;
							selection.name,selection.epoch,selection.version,selection.release,selection.architecture = parse_NEVRA value
							selection.skip = true if selection.architecture == "src" || selection.architecture == "nosrc"
						when "=Sum"
							selection.summary = value
						when "=Cat"
							selection.group = value
						when "+Req"
							selection.requires = parse_dependencies f, "selection", "-Req"
						when "+Prv"
							selection.provides = parse_dependencies f, "selection", "-Prv"
						when "+Con"
							selection.conflicts = parse_dependencies f, "selection", "-Con"
						when "+Obs"
							selection.obsoletes = parse_dependencies f, "selection", "-Obs"
						when "+Rec"
							selection.recommends = parse_dependencies f, "selection", "-Rec"
						when "+Ins"
							selection.packages = parse_dependencies f, "package", "-Ins"
						else
							next if key[0,5] = "=Sum."		# skip translations
							STDERR.puts "Unknown '#{key}'"

					end
				end # loop
				selection.save unless selection.nil?
			end # File.open()
		rescue
			STDERR.puts "Ooops!"
			raise
		end

		return pcount
	end
end


def usage
	STDERR.puts "Usage: selections_to_xml.rb <selections-file> [<selections-file> ...]"
	exit 1
end

usage unless ARGV.size > 0

parser = Parser.new

stream = STDOUT
stream.puts "<channel><subchannel>"
ARGV.each do |arg|
	parser.parse_selections arg, stream
end
stream.puts "</subchannel></channel>"
