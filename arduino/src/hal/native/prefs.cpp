#include "hal/prefs.h"

#include <sqlite3.h>
#include <cstring>
#include <stdexcept>

namespace HAL {

static sqlite3 *db = nullptr;

static void Exec(const char *sql)
{
    char *error = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error) != SQLITE_OK) {
        std::string msg = error ? error : sqlite3_errmsg(db);
        sqlite3_free(error);
        throw std::runtime_error(msg);
    }
}

Prefs::Prefs(const char *name, bool readOnly)
    : name{name}
    , readOnly{readOnly}
{
    if (!db) {
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

        if (sqlite3_open_v2("prefs.db", &db, flags, nullptr) != SQLITE_OK) {
            std::string msg = sqlite3_errmsg(db);
            sqlite3_close(db);
            db = nullptr;
            throw std::runtime_error(msg);
        }
    }

    if (!readOnly) {
        Exec(
            "CREATE TABLE IF NOT EXISTS prefs ("
            "namespace TEXT NOT NULL,"
            "key TEXT NOT NULL,"
            "value BLOB NOT NULL,"
            "PRIMARY KEY(namespace, key)"
            ")"
        );
    }
}

Prefs::~Prefs()
{
    // Database is shared between Prefs instances.
}

static bool ReadValue(
    const char *ns,
    const char *key,
    void *data,
    size_t size,
    size_t *actualSize = nullptr)
{
    sqlite3_stmt *stmt = nullptr;

    sqlite3_prepare_v2(
        db,
        "SELECT value FROM prefs WHERE namespace=? AND key=?",
        -1, &stmt, nullptr
    );

    sqlite3_bind_text(stmt, 1, ns, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, key, -1, SQLITE_TRANSIENT);

    bool found = false;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int len = sqlite3_column_bytes(stmt, 0);
        const void *blob = sqlite3_column_blob(stmt, 0);

        if (actualSize)
            *actualSize = len;

        if (data && size)
            memcpy(data, blob, std::min(size, (size_t)len));

        found = true;
    }

    sqlite3_finalize(stmt);
    return found;
}

static void WriteValue(
    const char *ns,
    const char *key,
    const void *data,
    size_t size)
{
    sqlite3_stmt *stmt = nullptr;

    sqlite3_prepare_v2(
        db,
        "INSERT INTO prefs(namespace,key,value) VALUES(?,?,?) "
        "ON CONFLICT(namespace,key) DO UPDATE SET value=excluded.value",
        -1, &stmt, nullptr
    );

    sqlite3_bind_text(stmt, 1, ns, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, key, -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 3, data, size, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

uint8_t Prefs::GetU8(const char *key, uint8_t def)
{
    uint8_t val;
    return ReadValue(name, key, &val, sizeof(val))
        ? val : def;
}

uint16_t Prefs::GetU16(const char *key, uint16_t def)
{
    uint16_t val;
    return ReadValue(name, key, &val, sizeof(val))
        ? val : def;
}

void Prefs::GetBytes(const char *key, uint8_t *data, size_t size)
{
    ReadValue(name, key, data, size);
}

size_t Prefs::GetBytesLength(const char *key)
{
    size_t size = 0;
    ReadValue(name, key, nullptr, 0, &size);
    return size;
}

std::string Prefs::GetString(const char *key)
{
    size_t size = GetBytesLength(key);
    if (!size)
        return {};

    std::string val(size, '\0');
    ReadValue(name, key, val.data(), size);
    return val;
}

void Prefs::PutU8(const char *key, uint8_t val)
{
    WriteValue(name, key, &val, sizeof(val));
}

void Prefs::PutU16(const char *key, uint16_t val)
{
    WriteValue(name, key, &val, sizeof(val));
}

void Prefs::PutBytes(const char *key, const uint8_t *data, size_t size)
{
    WriteValue(name, key, data, size);
}

void Prefs::PutString(const char *key, const char *val)
{
    WriteValue(name, key, val, strlen(val));
}

} // namespace HAL
