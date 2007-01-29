
CREATE TABLE db_info (
  version INTEGER
);

CREATE TABLE resolvables (
    id INTEGER
  , name TEXT NOT NULL
  , version TEXT
  , release TEXT
  , epoch INTEGER
  , arch INTEGER
  , installed_size INTEGER
  , catalog TEXT
  , installed INTEGER
  , local INTEGER
  , status INTEGER
  , category TEXT
  , license TEXT
  , kind INTEGER
  , PRIMARY KEY (id)
);

CREATE TABLE dependencies (
    resolvable_id INTEGER
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
    resolvable_id INTEGER
  , content TEXT
);

CREATE TABLE sources (
    id INTEGER
  , alias TEXT NOT NULL
  , url TEXT
  , description TEXT
  , enabled INTEGER
  , autorefresh INTEGER
  , type VARCHAR(20)
  , cachedir TEXT
  , path TEXT
  , PRIMARY KEY (id)
);

CREATE TABLE patch_details (
    resolvable_id INTEGER REFERENCES resolvables (id)
  , patch_id TEXT
  , creation_time INTEGER
  , reboot INTEGER
  , restart INTEGER
  , interactive INTEGER
  , summary TEXT
  , description TEXT
  , id INTEGER
);

CREATE TABLE pattern_details (
    resolvable_id INTEGER REFERENCES resolvables (id)
  , summary TEXT
  , description TEXT
  , id INTEGER
);

CREATE TABLE product_details (
    resolvable_id INTEGER REFERENCES resolvables (id)
  , summary TEXT
  , description TEXT
  , id INTEGER
);

CREATE TABLE script_details (
    resolvable_id INTEGER REFERENCES resolvables (id)
  , do_script TEXT
  , undo_script TEXT
  , id INTEGER
);

CREATE TABLE package_details (
    resolvable_id INTEGER REFERENCES resolvables (id)
  , group TEXT
  , summary TEXT
  , description TEXT
  , url TEXT
  , filename TEXT
  , signature_filename TEXT
  , file_size INTEGER
  , install_only INTEGER
  , media_nr INTEGER
  , id INTEGER
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
CREATE INDEX resolvable_key ON resolvables (key);

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
