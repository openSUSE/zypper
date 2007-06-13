-- what kinds of resolvables have been read?
select * from types where class = 'kind';

-- what languages have been encountered?
select * from types where class = 'lang';

-- what architectures have been read?
select * from types where class = 'arch';

-- what types of dependencies have been encountered?
select * from types where class = 'deptype';

-- ----------------------------------------------------------------------------

-- how many packages have been read?
select count(*) from resolvables r, types t where t.class = 'kind' and t.name = 'package' and t.id = r.kind;

-- print resolvable kind -> count table
select t.name, count(*) from resolvables r, types t where t.class = 'kind' and t.id = r.kind group by t.name;

-- what patches have been read? print id, name, and version
select r.id, r.name, r.version from resolvables r, types t where t.class = 'kind' and t.name = 'patch' and t.id = r.kind;

-- ----------------------------------------------------------------------------

-- print all text and numeric attributes of resolvable with id = 2
select a.weak_resolvable_id "res-id", t.class "attr-class", t.name "attr-name", a.text "value"
  from text_attributes a, types t
  where t.id = a.attr_id and a.weak_resolvable_id = 2
union
select a.weak_resolvable_id "res-id", t.class "attr-class", t.name "attr-name", a.value "value"
  from numeric_attributes a, types t
  where t.id = a.attr_id and a.weak_resolvable_id = 2
order by t.class

-- print all named dependencies of resolvable with id = 2
select dt.name "dtype", kt.name "kind", n.name "name", rt.name "rel", c.version "version"
  from named_capabilities c, types dt, types kt, types rt, names n
  where c.dependency_type = dt.id
    and c.refers_kind= kt.id
    and c.relation = rt.id
    and c.name_id = n.id
    and c.resolvable_id = 2;

-- print all file dependencies of resolvable with id = 2
select dt.name "dtype", kt.name "kind", (dname.name || '/' || fname.name) "file"
  from file_capabilities c, types dt, types kt, files f
    left outer join file_names fname on fname.id = f.file_name_id
    left outer join dir_names dname on dname.id = f.dir_name_id
  where c.dependency_type = dt.id
    and c.refers_kind= kt.id
    and c.file_id = f.id
    and c.resolvable_id = 2;

