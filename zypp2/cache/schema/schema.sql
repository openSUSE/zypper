
------------------------------------------------
-- The cleanup can be generated as:
-- cat schema.sql | grep "^CREATE" | awk '{print "DROP " $2 " IF EXISTS " $3 ";"}' | sort -r
------------------------------------------------

------------------------------------------------
-- version metadata, probably not needed, there
-- is pragma user_version
------------------------------------------------

DROP TABLE IF EXISTS db_info;
CREATE TABLE db_info (
  version INTEGER
);

------------------------------------------------
-- Knew catalogs. They existed some day.
------------------------------------------------

DROP TABLE IF EXISTS catalogs;
CREATE TABLE catalogs (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , url TEXT NOT NULL
  , path TEXT NOT NULL
  , checksum TEXT DEFAULT NULL
  , timestamp INTEGER NOT NULL
);

------------------------------------------------
-- Resolvable names
------------------------------------------------

DROP TABLE IF EXISTS names;
CREATE TABLE names (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT UNIQUE
);

------------------------------------------------
-- File names table and normalized sub tables
------------------------------------------------

DROP TABLE IF EXISTS file_names;
CREATE TABLE file_names (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT
);

DROP TABLE IF EXISTS dir_names;
CREATE TABLE dir_names (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT
);

DROP TABLE IF EXISTS files;
CREATE TABLE files (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , dir_name_id INTEGER NOT NULL
  , file_name_id INTEGER NOT NULL
  , UNIQUE ( dir_name_id, file_name_id )
);

------------------------------------------------
-- File names table and normalized sub tables
------------------------------------------------

DROP TABLE IF EXISTS translated_texts;
CREATE TABLE translated_texts (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , text_id INTEGER NOT NULL
  , lang_id INTEGER NOT NULL
  , text TEXT

);

DROP TABLE IF EXISTS resolvable_texts;
CREATE TABLE resolvable_texts (
    resolvable_id INTEGER NOT NULL
  , text_id INTEGER NOT NULL
  , lang_id INTEGER NOT NULL
  , field_id INTEGER NOT NULL
);

------------------------------------------------
-- Resolvables table
------------------------------------------------

DROP TABLE IF EXISTS resolvables;
CREATE TABLE resolvables (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT
  , version TEXT
  , release TEXT
  , epoch INTEGER
  , arch INTEGER
  , kind INTEGER
  , summary_text_id INTEGER
  , description_text_id INTEGER
  , insnotify TEXT
  , delnotify TEXT
  , license_to_confirm TEXT
  , vendor TEXT
  , installed_size INTEGER
  , archive_size INTEGER
  , install_only INTEGER
  , build_time INTEGER
  , install_time INTEGER
  , catalog_id INTEGER REFERENCES catalogs(id)
);

DROP TABLE IF EXISTS message_details;
CREATE TABLE message_details (
    resolvable_id INTEGER  REFERENCES resolvables(id)
  , text TEXT
);

DROP TABLE IF EXISTS patch_details;
CREATE TABLE patch_details (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , patch_id TEXT
  , timestamp INTEGER
  , category TEXT
  , reboot_needed INTEGER
  , affects_package_manager INTEGER

);

DROP TABLE IF EXISTS pattern_details;
CREATE TABLE pattern_details (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , user_default INTEGER
  , user_visible INTEGER
  , pattern_category TEXT
  , icon TEXT
  , script TEXT
  , pattern_order TEXT

);

DROP TABLE IF EXISTS product_details;
CREATE TABLE product_details (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , category TEXT
  , vendor TEXT
  , release_notes_url TEXT
  , update_urls TEXT
  , extra_urls TEXT
  , optional_urls TEXT
  , flags TEXT
  , short_name TEXT
  , long_name TEXT
  , distribution_name TEXT
  , distribution_edition TEXT

);

DROP TABLE IF EXISTS script_details;
CREATE TABLE script_details (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , do_script TEXT
  , undo_script TEXT


);

DROP TABLE IF EXISTS package_details;
CREATE TABLE package_details (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , checksum TEXT
  , changelog TEXT
  , buildhost TEXT
  , distribution TEXT
  , license TEXT
  , packager TEXT
  , package_group TEXT
  , url TEXT
  , os TEXT
  , prein TEXT
  , postin TEXT
  , preun TEXT
  , postun TEXT
  , source_size INTEGER
  , authors TEXT
  , filenames TEXT
  , location TEXT
);
CREATE INDEX package_details_resolvable_id ON package_details (resolvable_id);

------------------------------------------------
-- Do we need those here?
------------------------------------------------

DROP TABLE IF EXISTS locks;
CREATE TABLE locks (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT
  , version TEXT
  , release TEXT
  , epoch INTEGER
  , arch INTEGER
  , relation INTEGER
  , catalog TEXT
  , glob TEXT
  , importance INTEGER
  , importance_gteq INTEGER

);

CREATE VIEW messages
  AS SELECT * FROM resolvables, message_details
  WHERE resolvables.id = message_details.resolvable_id;

CREATE VIEW packages
  AS SELECT * FROM resolvables, package_details
  WHERE resolvables.id = package_details.resolvable_id;

CREATE VIEW patches
  AS SELECT * FROM resolvables, patch_details
  WHERE resolvables.id = patch_details.resolvable_id;

CREATE VIEW patterns AS SELECT * FROM resolvables, pattern_details
  WHERE resolvables.id = pattern_details.resolvable_id;

CREATE VIEW products AS
  SELECT * FROM resolvables, product_details
  WHERE resolvables.id = product_details.resolvable_id;

CREATE VIEW scripts AS
  SELECT * FROM resolvables, script_details
  WHERE resolvables.id = script_details.resolvable_id;

DROP TABLE IF EXISTS delta_packages;
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

);

DROP TABLE IF EXISTS patch_packages;
CREATE TABLE patch_packages (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , media_nr INTEGER
  , location TEXT
  , checksum TEXT
  , download_size INTEGER
  , build_time INTEGER

);

CREATE TRIGGER remove_resolvables
  AFTER DELETE ON resolvables
  BEGIN
    DELETE FROM package_details WHERE resolvable_id = old.id;
    DELETE FROM patch_details WHERE resolvable_id = old.id;
    DELETE FROM pattern_details WHERE resolvable_id = old.id;
    DELETE FROM product_details WHERE resolvable_id = old.id;
    DELETE FROM message_details WHERE resolvable_id = old.id;
    DELETE FROM script_details WHERE resolvable_id = old.id;
    DELETE FROM dependencies WHERE resolvable_id = old.id;
    DELETE FROM files WHERE resolvable_id = old.id;
    DELETE FROM delta_packages WHERE package_id = old.id;
    DELETE FROM patch_packages WHERE package_id = old.id;
  END;

DROP TABLE IF EXISTS patch_packages_baseversions;
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

DROP TABLE IF EXISTS named_capabilities;
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

DROP TABLE IF EXISTS modalias_capabilities;
CREATE TABLE modalias_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , name TEXT
  , value TEXT
  , relation INTEGER
);

DROP TABLE IF EXISTS hal_capabilities;
CREATE TABLE hal_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , name TEXT
  , value TEXT
  , relation INTEGER
);

DROP TABLE IF EXISTS file_capabilities;
CREATE TABLE file_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , file_id INTEGER REFERENCES files(id)
);

DROP TABLE IF EXISTS other_capabilities;
CREATE TABLE other_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , value TEXT
);

DROP TABLE IF EXISTS split_capabilities;
CREATE TABLE split_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , name_id INTEGER REFERENCES names(id)
  , file_id INTEGER REFERENCES files(id)
);

------------------------------------------------
-- Associate resolvables and catalogs
------------------------------------------------

-- FIXME do we want to allow same resolvable to
-- be listed twice in same source but different
-- medias? I think NOT.
DROP TABLE IF EXISTS resolvables_catalogs;
CREATE TABLE resolvables_catalogs (
    resolvable_id INTEGER REFERENCES resolvables (id)
  , catalog_id    INTEGER REFERENCES catalogs    (id)
  , catalog_media_nr INTEGER
  , PRIMARY KEY ( resolvable_id, catalog_id )
);

-- FIX this trigger
--CREATE TRIGGER remove_catalogs
--  AFTER DELETE ON catalogs
--  BEGIN
--    DELETE FROM resolvables WHERE catalog = old.id;
--  END;

CREATE TRIGGER remove_patch_packages_baseversions
  AFTER DELETE ON patch_packages
  BEGIN
    DELETE FROM patch_packages_baseversions WHERE patch_package_id = old.id;
  END;


