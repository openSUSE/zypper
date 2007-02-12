
DROP TABLE IF EXISTS files;


CREATE TABLE files (
    package_id INTEGER NOT NULL
  , name TEXT UNIQUE
  , PRIMARY KEY (package_id, name)
);

CREATE INDEX files_name ON files (name);
