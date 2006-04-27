#!/usr/bin/ruby
#
# patterns_to_xml.rb
#
# reads e.g. /mounts/dist/next-i386/suse/setup/descr/patterns
#
# run: ruby patterns_to_xml.rb patterns > patterns.xml
#
# Implementation note:
#  The output is generated via plain print/puts calls
#  The initial version used rexml but this was a memory hog
#  Since xml builds up the complete tree in memory before
#  actually writing it to a file, parsing a 6MB 'patterns'
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

class Pattern
	attr_accessor :name, :epoch, :version, :release, :architecture
	attr_accessor :summary, :description, :group
	attr_accessor :requires, :prerequires, :provides, :conflicts, :obsoletes, :recommends
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

		@channel.puts "<pattern>"

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
		add_dependencies "recommends", recommends

		@channel.puts "</pattern>"
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
	# parsepatterns
	# parse a patterns file
	# return number of patterns
	#

	def parse_patterns( filename, stream )
		pcount = 0
		pattern = nil

		STDERR.puts "Parsing #{filename}"

		begin	# rescue block
			buildfrom = nil					# src/nosrc pattern
			File.open( filename ) do |f|
				skip = false;
				loop do
					key, value = parse_key_value f
					break if key.nil?			# error
					next if value.nil?			# no key
					case key
						when "=Ver"
							raise "Wrong version '#{version}'" unless value = "4.0"
						when "=Pat"
							pcount += 1
							pattern.save unless pattern.nil?
							pattern = Pattern.new stream
							skip = false;
							pattern.name,pattern.epoch,pattern.version,pattern.release,pattern.architecture = parse_NEVRA value
							pattern.skip = true if pattern.architecture == "src" || pattern.architecture == "nosrc"
						when "=Sum"
							pattern.summary = value
						when "=Cat"
							pattern.group = value
						when "+Req"
							req = parse_dependencies f, "pattern", "-Req"
							next if req.nil?
							pattern.requires = [] if pattern.requires.nil?
							pattern.requires = pattern.requires + req
						when "+Prq"
							prq = parse_dependencies f, "package", "-Prq"
							next if prq.nil?
							pattern.requires = [] if pattern.requires.nil?
							pattern.requires = pattern.requires + prq
						when "+Prv"
							pattern.provides = parse_dependencies f, "pattern", "-Prv"
						when "+Con"
							pattern.conflicts = parse_dependencies f, "pattern", "-Con"
						when "+Obs"
							pattern.obsoletes = parse_dependencies f, "pattern", "-Obs"
						when "+Rec"
							rec = parse_dependencies f, "pattern", "-Rec"
							next if rec.nil?
							pattern.recommends = [] if pattern.recommends.nil?
							pattern.recommends = pattern.recommends + rec
						when "+Prc"
							prc = parse_dependencies f, "package", "-Prc"
							next if prc.nil?
							pattern.recommends = [] if pattern.recommends.nil?
							pattern.recommends = pattern.recommends + prc
						else
							next if key[0,5] = "=Sum."		# skip translations
							STDERR.puts "Unknown '#{key}'"

					end
				end # loop
				pattern.save unless pattern.nil?
			end # File.open()
		rescue
			STDERR.puts "Ooops!"
			raise
		end

		return pcount
	end
end


def usage
	STDERR.puts "Usage: patterns_to_xml.rb <patterns-file> [<patterns-file> ...]"
	exit 1
end

usage unless ARGV.size > 0

parser = Parser.new

stream = STDOUT
stream.puts "<channel><subchannel>"
ARGV.each do |arg|
	parser.parse_patterns arg, stream
end
stream.puts "</subchannel></channel>"
