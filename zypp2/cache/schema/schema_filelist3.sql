
DROP TABLE IF EXISTS file_entries;

CREATE TABLE file_entries (
    id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL
  , package_id INTEGER
  , dir TEXT UNIQUE
  , files TEXT
);
