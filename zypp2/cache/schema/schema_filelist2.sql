
DROP TABLE IF EXISTS files;
DROP TABLE IF EXISTS dir;

CREATE TABLE dirs (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , name TEXT UNIQUE
);

CREATE TABLE files (
     dir_id INTEGER NOT NULL REFERENCES dir(id)
  ,  package_id INTEGER NOT NULL
  , name TEXT
  , PRIMARY KEY (package_id, dir_id, name)
);

