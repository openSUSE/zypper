# File : Makefile.PL
use ExtUtils::MakeMaker;
WriteMakefile(
   'NAME'    => 'rpdbtozypp',                # Name of package
   'LIBS'    => '-lm -lzypp -lpdbtozypp',                    # Name of custom libraries
   'OBJECT'  => 'PdbToZypp_wrap.o /usr/lib/perl5/5.8.8/i586-linux-thread-multi/zypp.so'  # Object files
);
