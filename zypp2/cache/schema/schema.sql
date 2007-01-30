
DROP TABLE IF EXISTS db_info;
DROP TABLE IF EXISTS patch_packages_baseversions;
DROP TABLE IF EXISTS patch_packages;
DROP TABLE IF EXISTS delta_packages;
DROP TABLE IF EXISTS package_details;
DROP VIEW IF EXISTS packages;
DROP TABLE IF EXISTS patch_details;
DROP VIEW IF EXISTS patches;
DROP TABLE IF EXISTS pattern_details;
DROP VIEW IF EXISTS patterns;
DROP TABLE IF EXISTS product_details;
DROP VIEW IF EXISTS products;
DROP TABLE IF EXISTS message_details;
DROP VIEW IF EXISTS messages;
DROP TABLE IF EXISTS script_details;
DROP VIEW IF EXISTS scripts;
DROP TABLE IF EXISTS dependencies;
DROP TABLE IF EXISTS sources;
DROP TABLE IF EXISTS locks;
DROP TABLE IF EXISTS resolvables;


CREATE TABLE db_info (
  version INTEGER
);

CREATE TABLE resolvables (
    id INTEGER AUTO INCREMENT
  , name TEXT
  , version TEXT
  , release TEXT
  , epoch INTEGER
  , arch INTEGER
  , kind INTEGER
  , summary TEXT
  , description TEXT
  , insnotify TEXT
  , delnotify TEXT
  , license_to_confirm TEXT
  , vendor TEXT
  , size INTEGER
  , archive_size INTEGER
  , catalog INTEGER
  , catalog_media_nr INTEGER
  , install_only INTEGER
  , build_time INTEGER
  , install_time INTEGER
  , PRIMARY KEY (id)
);

CREATE TABLE dependencies (
    resolvable_id INTEGER REFERENCES resolvable(id)
  , dep_type INTEGER
  , name TEXT
  , version TEXT
  , release TEXT
  , epoch INTEGER
  , arch INTEGER
  , relation INTEGER
  , dep_target INTEGER
);

CREATE TABLE message_details (
    resolvable_id INTEGER  REFERENCES resolvable(id)
  , text TEXT
);

CREATE TABLE catalogs (
    id INTEGER
  , alias TEXT NOT NULL
  , url TEXT
  , description TEXT
  , enabled INTEGER
  , autorefresh INTEGER
  , type VARCHAR(20)
  , cache_dir TEXT
  , path TEXT
  , PRIMARY KEY (id)
);

CREATE TABLE patch_details (
    id INTEGER
  , resolvable_id INTEGER REFERENCES resolvables (id)
  , patch_id TEXT
  , timestamp INTEGER
  , category TEXT
  , reboot_needed INTEGER
  , affects_package_manager INTEGER
  , PRIMARY KEY (id)
);

CREATE TABLE pattern_details (
    id INTEGER
  , resolvable_id INTEGER REFERENCES resolvables (id)
  , user_default INTEGER
  , user_visible INTEGER
  , pattern_category TEXT
  , icon TEXT
  , script TEXT
  , pattern_order TEXT
  , PRIMARY KEY (id)
);

CREATE TABLE product_details (
    id INTEGER
  , resolvable_id INTEGER REFERENCES resolvables (id)
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
  , PRIMARY KEY (id)
);

CREATE TABLE script_details (
    id INTEGER
  , resolvable_id INTEGER REFERENCES resolvables (id)
  , do_script TEXT
  , undo_script TEXT
  , PRIMARY KEY (id)
  
);

CREATE TABLE package_details (
    resolvable_id INTEGER REFERENCES resolvables (id)
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

CREATE TABLE locks (
    id INTEGER
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
  , PRIMARY KEY (id)
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
    id INTEGER
  , package_id INTEGER REFERENCES packages(id)
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
    id INTEGER
  , package_id INTEGER REFERENCES packages(id)
  , media_nr INTEGER
  , location TEXT
  , checksum TEXT
  , download_size INTEGER
  , build_time INTEGER
);

CREATE TABLE patch_packages_baseversions (
    patch_package_id INTEGER REFERENCES patch_packages(id)
  , version TEXT
  , release TEXT
  , epoch INTEGER
);

CREATE INDEX dependency_resolvable ON dependencies (resolvable_id);
CREATE INDEX package_details_resolvable_id ON package_details (resolvable_id);
CREATE INDEX resolvable_catalog ON resolvables (catalog);

CREATE TRIGGER remove_catalogs
  AFTER DELETE ON catalogs
  BEGIN
    DELETE FROM resolvables WHERE catalog = old.id;
  END;

CREATE TRIGGER remove_patch_packages_baseversions
  AFTER DELETE ON patch_packages
  BEGIN
    DELETE FROM patch_packages_baseversions WHERE patch_package_id = old.id;
  END;

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
