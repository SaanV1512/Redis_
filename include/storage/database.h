#ifndef DATABASE_H
#define DATABASE_H

void db_init();
void db_set(const char* key, const char* value);
char* db_get(const char* key);
void db_del(const char* key);
void db_free();

#endif