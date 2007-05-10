
------------------------------------------------
-- The cleanup can be generated as:
-- cat schema.sql | grep "^CREATE" | awk '{print "DROP " $2 " IF EXISTS " $3 ";"}' | sort -r
------------------------------------------------

DROP VIEW IF EXISTS scripts;
DROP VIEW IF EXISTS products;
DROP VIEW IF EXISTS patterns;
DROP VIEW IF EXISTS patches;
DROP VIEW IF EXISTS packages;
DROP VIEW IF EXISTS messages;
DROP TRIGGER IF EXISTS remove_resolvables;
DROP TRIGGER IF EXISTS remove_patch_packages_baseversions;
DROP TABLE IF EXISTS split_capabilities;
DROP TABLE IF EXISTS script_ttext_attributes;
DROP TABLE IF EXISTS script_text_attributes;
DROP TABLE IF EXISTS script_script_ttext_attributes;
DROP TABLE IF EXISTS script_script_text_attributes;
DROP TABLE IF EXISTS script_details;
DROP TABLE IF EXISTS resolvable_ttext_attributes;
DROP TABLE IF EXISTS resolvable_text_attributes;
DROP TABLE IF EXISTS resolvables_catalogs;
DROP TABLE IF EXISTS resolvables;
DROP TABLE IF EXISTS resolvable_resolvable_ttext_attributes;
DROP TABLE IF EXISTS resolvable_resolvable_text_attributes;
DROP TABLE IF EXISTS product_ttext_attributes;
DROP TABLE IF EXISTS product_text_attributes;
DROP TABLE IF EXISTS product_product_ttext_attributes;
DROP TABLE IF EXISTS product_product_text_attributes;
DROP TABLE IF EXISTS product_details;
DROP TABLE IF EXISTS pattern_details;
DROP TABLE IF EXISTS patch_text_attributes;
DROP TABLE IF EXISTS patch_patch_text_attributes;
DROP TABLE IF EXISTS patch_packages_baseversions;
DROP TABLE IF EXISTS patch_packages;
DROP TABLE IF EXISTS patch_details;
DROP TABLE IF EXISTS package_ttext_attributes;
DROP TABLE IF EXISTS package_text_attributes;
DROP TABLE IF EXISTS package_package_ttext_attributes;
DROP TABLE IF EXISTS package_package_text_attributes;
DROP TABLE IF EXISTS package_details;
DROP TABLE IF EXISTS other_capabilities;
DROP TABLE IF EXISTS names;
DROP TABLE IF EXISTS named_capabilities;
DROP TABLE IF EXISTS modalias_capabilities;
DROP TABLE IF EXISTS message_ttext_attributes;
DROP TABLE IF EXISTS message_message_ttext_attributes;
DROP TABLE IF EXISTS message_details;
DROP TABLE IF EXISTS locks;
DROP TABLE IF EXISTS hal_capabilities;
DROP TABLE IF EXISTS files;
DROP TABLE IF EXISTS file_names;
DROP TABLE IF EXISTS file_capabilities;
DROP TABLE IF EXISTS dir_names;
DROP TABLE IF EXISTS delta_packages;
DROP TABLE IF EXISTS db_info;
DROP TABLE IF EXISTS catalogs;
DROP INDEX IF EXISTS package_details_resolvable_id;
DROP INDEX IF EXISTS named_capabilities_name;

------------------------------------------------
-- version metadata, probably not needed, there
-- is pragma user_version
------------------------------------------------

CREATE TABLE db_info (
  version INTEGER
);

------------------------------------------------
-- Knew catalogs. They existed some day.
------------------------------------------------

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

CREATE TABLE resolvables (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT
  , version TEXT
  , release TEXT
  , epoch INTEGER
  , arch INTEGER
  , kind INTEGER
  , catalog_id INTEGER REFERENCES catalogs(id)
  ,  installed_size INTEGER
  , archive_size INTEGER
  , install_only INTEGER
  , build_time INTEGER
  , install_time INTEGER
);

-- Resolvables translated strings
CREATE TABLE resolvable_ttext_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , lang_code INTEGER
  , summary TEXT
  , description TEXT
  , insnotify TEXT
  , delnotify TEXT
  , license_to_confirm
);

-- Association between ttext attributes and resolvables
CREATE TABLE resolvable_resolvable_ttext_attributes (
    resolvable_id INTEGER REFERENCES resolvable(id)
  , ttext_attribute_id INTEGER REFERENCES resolvable_ttext_attributes(id)
  , PRIMARY KEY ( resolvable_id, ttext_attribute_id )
);

-- Resolvables untranslated strings
CREATE TABLE resolvable_text_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
);

-- Association between text attributes and resolvables
CREATE TABLE resolvable_resolvable_text_attributes (
    resolvable_id INTEGER REFERENCES resolvable(id)
  , text_attribute_id INTEGER REFERENCES text_attribute(id)
  , PRIMARY KEY ( resolvable_id, text_attribute_id )
);

------------------------------------------------
-- Resolvables kind details
------------------------------------------------

CREATE TABLE message_details (
    resolvable_id INTEGER  REFERENCES resolvables(id)
  , text TEXT
);

-- messages translated strings
CREATE TABLE message_ttext_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , lang_code INTEGER
  , summary TEXT
  , description TEXT
  , insnotify TEXT
  , delnotify TEXT
  , license_to_confirm
);

-- Association between ttext attributes and messages
CREATE TABLE message_message_ttext_attributes (
    message_id INTEGER REFERENCES message(id)
  , ttext_attribute_id INTEGER REFERENCES message_ttext_attributes(id)
  , PRIMARY KEY ( message_id, ttext_attribute_id )
);

CREATE TABLE patch_details (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , patch_id TEXT
  , timestamp INTEGER
  , category TEXT
  , reboot_needed INTEGER
  , affects_package_manager INTEGER

);

-- patchs translated strings
CREATE TABLE patch_text_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , patch_id TEXT
  , category TEXT
);

-- Association between ttext attributes and patchs
CREATE TABLE patch_patch_text_attributes (
    patch_id INTEGER REFERENCES patch(id)
  , text_attribute_id INTEGER REFERENCES patch_text_attributes(id)
  , PRIMARY KEY ( patch_id, text_attribute_id )
);

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

CREATE TABLE product_details (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
);

-- products translated strings
CREATE TABLE product_ttext_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , lang_code INTEGER
  , short_name TEXT
);

-- Association between ttext attributes and products
CREATE TABLE product_product_ttext_attributes (
    product_id INTEGER REFERENCES product(id)
  , ttext_attribute_id INTEGER REFERENCES product_ttext_attributes(id)
  , PRIMARY KEY ( product_id, ttext_attribute_id )
);

-- products translated strings
CREATE TABLE product_text_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , category TEXT
  , distribution_name TEXT
  , distribution_edition TEXT
  , vendor TEXT
  , release_notes_url TEXT
  , update_urls TEXT
  , extra_urls TEXT
  , optional_urls TEXT
  , flags TEXT
  , short_name TEXT
);

-- Association between ttext attributes and products
CREATE TABLE product_product_text_attributes (
    product_id INTEGER REFERENCES product(id)
  , text_attribute_id INTEGER REFERENCES product_text_attributes(id)
  , PRIMARY KEY ( product_id, text_attribute_id )
);

CREATE TABLE script_details (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
);

-- scripts translated strings
CREATE TABLE script_ttext_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , lang_code INTEGER
);

-- Association between ttext attributes and scripts
CREATE TABLE script_script_ttext_attributes (
    script_id INTEGER REFERENCES script(id)
  , ttext_attribute_id INTEGER REFERENCES script_ttext_attributes(id)
  , PRIMARY KEY ( script_id, ttext_attribute_id )
);

-- scripts translated strings
CREATE TABLE script_text_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , do_script TEXT
  , undo_script TEXT
);

-- Association between ttext attributes and scripts
CREATE TABLE script_script_text_attributes (
    script_id INTEGER REFERENCES script(id)
  , text_attribute_id INTEGER REFERENCES script_text_attributes(id)
  , PRIMARY KEY ( script_id, text_attribute_id )
);

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

-- packages translated strings
CREATE TABLE package_ttext_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , lang_code INTEGER
  , short_name TEXT
);

-- Association between ttext attributes and packages
CREATE TABLE package_package_ttext_attributes (
    package_id INTEGER REFERENCES package(id)
  , ttext_attribute_id INTEGER REFERENCES package_ttext_attributes(id)
  , PRIMARY KEY ( package_id, ttext_attribute_id )
);

-- packages translated strings
CREATE TABLE package_text_attributes (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , category TEXT
  , distribution_name TEXT
  , distribution_edition TEXT
  , vendor TEXT
  , release_notes_url TEXT
  , update_urls TEXT
  , extra_urls TEXT
  , optional_urls TEXT
  , flags TEXT
  , short_name TEXT
);

-- Association between ttext attributes and packages
CREATE TABLE package_package_text_attributes (
    package_id INTEGER REFERENCES package(id)
  , text_attribute_id INTEGER REFERENCES package_text_attributes(id)
  , PRIMARY KEY ( package_id, text_attribute_id )
);


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

CREATE TABLE modalias_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , name TEXT
  , value TEXT
  , relation INTEGER
);

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

CREATE TABLE other_capabilities (
   id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , resolvable_id INTEGER REFERENCES resolvables(id)
  , dependency_type INTEGER
  , refers_kind INTEGER
  , value TEXT
);

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


