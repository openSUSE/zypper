
------------------------------------------------
-- The cleanup can be generated as:
-- cat schema.sql | grep "^CREATE" | awk '{print "DROP " $2 " IF EXISTS " $3 ";"}' | sort -r
------------------------------------------------

DROP TRIGGER IF EXISTS remove_resolvables;
DROP TRIGGER IF EXISTS remove_patch_packages_baseversions;
DROP TABLE IF EXISTS types;
DROP TABLE IF EXISTS text_attributes;
DROP TABLE IF EXISTS split_capabilities;
DROP TABLE IF EXISTS resolvables_repositories;
DROP TABLE IF EXISTS resolvables;
DROP TABLE IF EXISTS patch_packages_baseversions;
DROP TABLE IF EXISTS patch_packages;
DROP TABLE IF EXISTS other_capabilities;
DROP TABLE IF EXISTS numeric_attributes;
DROP TABLE IF EXISTS names;
DROP TABLE IF EXISTS named_capabilities;
DROP TABLE IF EXISTS modalias_capabilities;
DROP TABLE IF EXISTS locks;
DROP TABLE IF EXISTS hal_capabilities;
DROP TABLE IF EXISTS files;
DROP TABLE IF EXISTS file_names;
DROP TABLE IF EXISTS file_capabilities;
DROP TABLE IF EXISTS dir_names;
DROP TABLE IF EXISTS delta_packages;
DROP TABLE IF EXISTS db_info;
DROP TABLE IF EXISTS repositories;
DROP INDEX IF EXISTS types_class_name_index;
DROP INDEX IF EXISTS text_attributes_index;
DROP INDEX IF EXISTS numeric_attributes_index;
DROP INDEX IF EXISTS named_capabilities_name;

------------------------------------------------
-- version metadata, probably not needed, there
-- is pragma user_version
------------------------------------------------

CREATE TABLE db_info (
  version INTEGER
);

------------------------------------------------
-- Basic types like archs, attributes, languages
------------------------------------------------

CREATE TABLE types (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , class TEXT NOT NULL
  , name TEXT NOT NULL
);
CREATE INDEX types_class_name_index ON types(class, name);

------------------------------------------------
-- Knew repositories. They existed some day.
------------------------------------------------

CREATE TABLE repositories (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , alias TEXT NOT NULL UNIQUE
  , checksum TEXT DEFAULT NULL
  , timestamp INTEGER NOT NULL
);
CREATE INDEX repo_alias_index ON repositories(alias);

------------------------------------------------
-- Resolvable names
------------------------------------------------

CREATE TABLE names (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT UNIQUE
);

------------------------------------------------
-- File names table and normalized sub tables
------------------------------------------------

CREATE TABLE file_names (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT
);

CREATE TABLE dir_names (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT
);

CREATE TABLE files (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , dir_name_id INTEGER NOT NULL
  , file_name_id INTEGER NOT NULL
  , UNIQUE ( dir_name_id, file_name_id )
);

------------------------------------------------
-- Resolvables table
------------------------------------------------

-- Resolvables translated strings
CREATE TABLE text_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , weak_resolvable_id INTEGER NOT NULL
  , lang_id INTEGER REFERENCES types(id)
  , attr_id INTEGER REFERENCES types(id)
  , text TEXT
);
CREATE INDEX text_attributes_index ON text_attributes(weak_resolvable_id, lang_id, attr_id);

-- Resolvables numeric attributes
CREATE TABLE numeric_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , weak_resolvable_id INTEGER NOT NULL
  , attr_id INTEGER REFERENCES types(id)
  , value INTEGER NOT NULL
);
CREATE INDEX numeric_attributes_index ON numeric_attributes(weak_resolvable_id, attr_id);


CREATE TABLE resolvables (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT
  , version TEXT
  , release TEXT
  , epoch INTEGER
  , arch INTEGER REFERENCES types(id)
  , kind INTEGER REFERENCES types(id)
  , repository_id INTEGER REFERENCES repositories(id)
  , installed_size INTEGER
  , archive_size INTEGER
  , install_only INTEGER
  , build_time INTEGER
  , install_time INTEGER
  , shared_id INTEGER DEFAULT NULL
);
CREATE INDEX resolvable_repository_id ON resolvables(repository_id);

------------------------------------------------
-- Do we need those here?
------------------------------------------------

CREATE TABLE locks (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT
  , version TEXT
  , release TEXT
  , epoch INTEGER
  , arch INTEGER
  , relation INTEGER
  , repository TEXT
  , glob TEXT
  , importance INTEGER
  , importance_gteq INTEGER

);

CREATE TABLE delta_packages (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , media_nr INTEGER
  , location TEXT
  , checksum TEXT
  , download_size INTEGER
  , build_time INTEGER
  , baseversion_version TEXT
  , baseversion_release TEXT
  , baseversion_epoch INTEGER
  , baseversion_checksum TEXT
  , baseversion_build_time INTEGER
  , baseversion_sequence_info TEXT
  , repository_id INTEGER REFERENCES repositories(id)
);
CREATE INDEX delta_package_repository_id ON delta_packages(repository_id);

CREATE TABLE patch_packages (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , media_nr INTEGER
  , location TEXT
  , checksum TEXT
  , download_size INTEGER
  , build_time INTEGER
  , repository_id INTEGER REFERENCES repositories(id)
);
CREATE INDEX patch_package_repository_id ON patch_packages(repository_id);

CREATE TABLE patch_packages_baseversions (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , patch_package_id INTEGER REFERENCES patch_packages(id)
  , version TEXT
  , release TEXT
  , epoch INTEGER
);

------------------------------------------------
-- Capabilities
------------------------------------------------

CREATE TABLE named_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , name_id INTEGER REFERENCES names(id)
  , version TEXT
  , release TEXT
  , epoch INTEGER
  , relation INTEGER
);
CREATE INDEX named_capabilities_name ON named_capabilities(name_id);
CREATE INDEX named_capabilities_resolvable ON named_capabilities(resolvable_id);

CREATE TABLE modalias_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , name TEXT
  , value TEXT
  , relation INTEGER
);
CREATE INDEX modalias_capabilities_resolvable ON modalias_capabilities(resolvable_id);

CREATE TABLE hal_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , name TEXT
  , value TEXT
  , relation INTEGER
);

CREATE TABLE file_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , file_id INTEGER REFERENCES files(id)
);
CREATE INDEX file_capabilities_resolvable ON file_capabilities(resolvable_id);

CREATE TABLE other_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , value TEXT
);
CREATE INDEX other_capabilities_resolvable ON other_capabilities(resolvable_id);

CREATE TABLE split_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , name_id INTEGER REFERENCES names(id)
  , file_id INTEGER REFERENCES files(id)
);
CREATE INDEX split_capabilities_resolvable ON split_capabilities(resolvable_id);

------------------------------------------------
-- File list
------------------------------------------------

CREATE TABLE resolvable_files (
    resolvable_id INTEGER REFERENCES resolvables (id)
  , file_id INTEGER REFERENCES files (id)
  , PRIMARY KEY ( resolvable_id, file_id )
);

------------------------------------------------
-- Disk usage
------------------------------------------------

CREATE TABLE resolvable_disk_usage (
    resolvable_id INTEGER REFERENCES resolvables (id)
  , dir_name_id INTEGER REFERENCES dir_name (id)
  , files INTEGER
  , size INTEGER
  , PRIMARY KEY ( resolvable_id, dir_name_id )
);
CREATE INDEX disk_usage_resolvable_id ON resolvable_disk_usage(resolvable_id);
CREATE INDEX disk_usage_dir_name_id ON resolvable_disk_usage(dir_name_id);

------------------------------------------------
-- Cleanup
------------------------------------------------

CREATE TRIGGER remove_resolvables
  AFTER DELETE ON resolvables
  BEGIN
    DELETE FROM named_capabilities WHERE resolvable_id = old.id;
    DELETE FROM modalias_capabilities WHERE resolvable_id = old.id;
    DELETE FROM hal_capabilities WHERE resolvable_id = old.id;
    DELETE FROM file_capabilities WHERE resolvable_id = old.id;
    DELETE FROM split_capabilities WHERE resolvable_id = old.id;
    DELETE FROM other_capabilities WHERE resolvable_id = old.id;
    
    DELETE FROM text_attributes  WHERE weak_resolvable_id = old.id;
    DELETE FROM numeric_attributes  WHERE weak_resolvable_id = old.id;
    DELETE FROM resolvable_disk_usage WHERE resolvable_id = old.id;
    DELETE FROM resolvable_files WHERE resolvable_id = old.id;
  END;




------------------------------------------------
-- Associate resolvables and repositories
------------------------------------------------

-- FIXME do we want to allow same resolvable to
-- be listed twice in same source but different
-- medias? I think NOT.
CREATE TABLE resolvables_repositories (
    resolvable_id INTEGER REFERENCES resolvables (id)
  , repository_id    INTEGER REFERENCES repositories    (id)
  , repository_media_nr INTEGER
  , PRIMARY KEY ( resolvable_id, repository_id )
);

-- Auto clean repositories
CREATE TRIGGER remove_repositories
  AFTER DELETE ON repositories
  BEGIN
    DELETE FROM resolvables WHERE repository_id = old.id;
    DELETE FROM delta_packages WHERE repository_id = old.id;
    DELETE FROM patch_packages WHERE repository_id = old.id;
  END;

CREATE TRIGGER remove_patch_packages_baseversions
  AFTER DELETE ON patch_packages
  BEGIN
    DELETE FROM patch_packages_baseversions WHERE patch_package_id = old.id;
  END;


