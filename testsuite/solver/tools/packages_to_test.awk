BEGIN   { FS=" "
	  print "<?xml version=\"1.0\"?>"
	  print "<test>";
	  print "<setup arch=\"i586\">";	/* or "x86_64" */
	  print "<channel name=\"pkg\" file=\"packages.xml\"/>"
	  print "</setup>"
	 }

/^=Pkg:/	{  print "<trial>";
	   printf ("<install channel=\"pkg\" name=\"%s\" />\n", $2);
	   print "</trial>";
	}

END	{ print "</test>";
	}
