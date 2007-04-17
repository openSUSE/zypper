#!/usr/bin/ruby

mode = :normal
File.new(ARGV[0]).each_line do |l |
  tag = l[0,5]
  if ( tag == "+Prv:")
    mode = :dep
    puts "+p"
    next
  end
  if ( tag == "+Req:" )
    mode = :dep
    puts "+r"
    next
  end
  if ( tag == "-Req:" )
    mode = :normal
    puts "-r"
    next
  end
  if ( tag == "-Prv:")
    mode = :normal
    puts "-p"
    next
  end

  if mode == :normal
    if ( l[0,6] == "=Pkg: " )
      puts l[6, l.size]
    end
  end
  
  if mode == :dep
    puts l
  end
  
  
end