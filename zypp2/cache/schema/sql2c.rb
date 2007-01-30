#!/usr/bin/ruby

class String
  def trim(margin=nil)
    s = self.dup
    # Remove initial blank line.
    s.sub!(/\A[ \t]*\n/, "")
    # Get rid of the margin, if it's specified.
    unless margin.nil?
      margin_re = Regexp.escape(margin || "")
      margin_re = /^[ \t]*#{margin_re} ?/
      s.gsub!(margin_re, "")
    end
    # Remove trailing whitespace on each line
    s.gsub!(/[ \t]+$/, "")
    s
  end
end

if ARGV.size != 1
  puts "Usage: sql2c.rb filename.sql"
  exit
end

filename = ARGV[0]

puts "char** getsql() {"
puts "  static char *sql[] = {"

File.open(filename, "r") do | f |
  f.each_line do |line|
    l = line.chomp.trim
    next if l == ""
    newline = "  \"#{l}\""
    lastchar = newline[newline.size - 2, 1]
#puts "#{lastchar}"
    (newline = newline + ",\n\n") if ( lastchar == ";")
    puts newline
  end
end

puts "  0};"

puts "  return sql;"
puts "}"
