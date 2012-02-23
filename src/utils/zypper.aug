(* Lens for parsing Zypper configuration file.

About: License
  This file is licensed under the GPLv2+, like the rest of Zypper.
*)

module ZYpper =
  autoload xfm

  (* ****************( primitives )*************** *)
  (* These are taken from the official util.aug *)

  (*
  Variable: eol
    Delete end of line, including optional trailing whitespace
  *)
  let eol = del /[ \t]*\n/ "\n"

  (*
  Variable: del_str
    Delete a string and default to it

  Parameters:
     s:string - the string to delete and default to
  *)
  let del_str (s:string) = del s s

  (*
  Variable: del_opt_ws
    Delete optional whitespace
  *)
  let del_opt_ws = del /[ \t]*/

  (* Matches an empty line and creates a node for it *)
  let empty = [ eol ]

  (* Deletes optional whitespace and stores the rest 'till the end of line *)
  let store_to_eol = del_opt_ws " " . store /(.*[^ \t\n])?/

  (*
    Keyword regex.
    Allows alphanumericals and '.' and '_'. Must start with a letter
    and end with a letter or number.
  *)
  let kw_re = /[a-zA-Z][a-zA-Z0-9\._]*[a-zA-Z0-9]/

  (* ****************( section )*************** *)

  (* Matches one line of ## description and creates a node for it *)
  let description = [ label "description" . del /##/ "##" . store_to_eol? . eol ]

  (* Matches '#' and whitespace, creates a 'commented' note for it. *)
  let commented  = [ label "commented" . del /#[ \t]*/ "# " . store_to_eol? . eol ]

  (* Matches key=value, creates a new node out of key and stores the value *)
  let kv = [ del_opt_ws "" . key kw_re . del /[ \t]*=[ \t]*/ " = " . store /[^ \t\n]([^\n]*[^ \t\n])?/ . eol ]

  (* An option consists of ## description, # commented lines and an optionall key=value pair. *)
  let option = [ seq "option" . description* . commented* . kv? ]

  (* ****************( section )*************** *)

  (* Matches section [title] and creates a new tree node out of it *)
  let section_title = del_str "[" . key /[^] \t\n\/]+/ . del_str "]" . eol

  (* Section with it's contests *)
  let section = [ section_title . (option | empty)* ]

  (* Optional comments in the anonymous section (start of the file). *)
  let section_anon = [ label "anon" . ( description | empty )+ ]


  (* The lens matching and mapping the whole file *)
  let lns = section_anon? . section+

  (*
    Filter for the xfm transformation.
    This is for system-wide zypper.conf only. ~/.zypper.conf must be
    read on-demand only.
  *)
  let filter = (incl "/etc/zypp/zypper.conf")

  (* Transfrom files matching 'filter' using lens 'lns' *)
  let xfm = transform lns filter
