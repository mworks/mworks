//
//  MWK2File.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/7/18.
//

#include "MWK2File.hpp"

#include <compression.h>

#include "MessagePackAdaptors.hpp"


BEGIN_NAMESPACE_MW


BEGIN_NAMESPACE()


inline void logSQLError(int error, const std::string &description) {
    merror(M_FILE_MESSAGE_DOMAIN, "%s: %s (%d)", description.c_str(), sqlite3_errstr(error), error);
}


[[noreturn]] inline void throwSQLError(int error, const std::string &description) {
    throw SimpleException(M_FILE_MESSAGE_DOMAIN,
                          boost::format("%s: %s (%d)") % description % sqlite3_errstr(error) % error);
}


//
// cstawarz (2020/01/06):
//
// Setting SQLite's "synchronous" flag to "OFF" [1] may seem dangerous, but I believe it's the correct choice
// for MWorks.
//
// First, when the flag is set to "FULL" or "NORMAL", high levels of non-MWorks disk I/O severely reduce SQLite's
// write performance.  This can lead to long delays (on the order of hours) in MWorks events being written to disk.
// If the user force quits MWServer before the backlog is cleared, they can lose data [2].  With "synchronous" set
// to "OFF", these backups don't occur.
//
// Second, the calls to fsync that SQLite makes when "synchronous" is "NORMAL" or higher don't appear to provide
// the resilience against OS crashes or power loss that the SQLite docs suggest.  The fsync man page [3] states this
// explicitly and says that an application must use F_FULLFSYNC to be certain that its data is actually written to
// disk.  While SQLite does support using F_FULLFSYNC [4], enabling this kills MWorks' write performance even under
// moderate disk I/O loads.
//
// Given the clear potential for data loss due to write backlogs and the dubious benefits of fsync for data security,
// setting "synchronous" to "OFF" seems like our best option.
//
// [1] https://sqlite.org/pragma.html#pragma_synchronous
// [2] https://mworks.tenderapp.com/discussions/problems/413
// [3] https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/fsync.2.html
// [4] https://sqlite.org/pragma.html#pragma_fullfsync
//
const std::string createTableSQL
(R"(
 PRAGMA locking_mode = EXCLUSIVE;
 PRAGMA page_size = %1%;
 PRAGMA synchronous = OFF;
 CREATE TABLE events (
   code INTEGER,
   time INTEGER,
   data /*no type affinity*/,
   PRIMARY KEY (code, time)
 ) WITHOUT ROWID;
 )");


const std::string insertSQL
(R"(
 INSERT INTO events VALUES (?, ?, ?)
 ON CONFLICT (code, time) DO UPDATE
 SET data = concat_event_data(data, excluded.data)
 )");


const std::string selectSQL
(R"(
 SELECT code, time, data FROM events
 )");


constexpr int codeColumn = 0;
constexpr int timeColumn = 1;
constexpr int dataColumn = 2;


constexpr int compressedTextTypeCode = 1;
constexpr int compressedMsgPackStreamTypeCode = 2;


END_NAMESPACE()


void MWK2File::StatementDeleter::operator()(sqlite3_stmt *stmt) const {
    // Ignore the return value, as it indicates whether the most recent evaluation
    // of the statement succeeded, not whether the statement was finalized successfully
    (void)sqlite3_finalize(stmt);
}


MWK2File::MWK2File() :
    conn(nullptr),
    scratchBuffer(std::max(compression_encode_scratch_buffer_size(COMPRESSION_ZLIB),
                           compression_decode_scratch_buffer_size(COMPRESSION_ZLIB)))
{ }


MWK2File::~MWK2File() {
    auto result = sqlite3_close(conn);  // Safe to pass NULL
    if (SQLITE_OK != result) {
        logSQLError(result, "Cannot close data file");
    }
}


int MWK2File::prepareStatement(const std::string &sql, StatementPtr &stmtPtr) {
    sqlite3_stmt *stmt = nullptr;
    auto result = sqlite3_prepare_v3(conn,
                                     sql.c_str(),
                                     sql.size(),
                                     SQLITE_PREPARE_PERSISTENT,
                                     &stmt,
                                     nullptr);
    if (stmt) {
        stmtPtr.reset(stmt);
    }
    return result;
}


static bool alwaysReferencePackedData(msgpack::type::object_type type, std::size_t length, void *userData) {
    return true;
}


std::tuple<int, std::size_t> MWK2File::unpack(const char *data,
                                              std::size_t size,
                                              std::size_t &offset,
                                              bool tryDecompression,
                                              Datum *valuePtr)
{
    try {
        auto handle = msgpack::unpack(data, size, offset, alwaysReferencePackedData);
        auto &object = handle.get();
        
        if (tryDecompression &&
            offset == size &&
            object.type == msgpack::type::object_type::EXT)
        {
            auto &extObj = object.via.ext;
            auto extType = extObj.type();
            
            if (extType == compressedTextTypeCode || extType == compressedMsgPackStreamTypeCode) {
                if (compressionBuffer.size() < 2 * extObj.size) {
                    compressionBuffer.resize(2 * extObj.size);
                }
                
                std::size_t decompressedSize;
                do {
                    decompressedSize = compression_decode_buffer(compressionBuffer.data(),
                                                                 compressionBuffer.size(),
                                                                 reinterpret_cast<const std::uint8_t *>(extObj.data()),
                                                                 extObj.size,
                                                                 scratchBuffer.data(),
                                                                 COMPRESSION_ZLIB);
                    if (decompressedSize < compressionBuffer.size()) {
                        break;
                    }
                    // If the decompressed size equals the size of the compression buffer, then there wasn't
                    // enough space to decompress all the data.  Double the buffer size and try again.
                    compressionBuffer.resize(2 * compressionBuffer.size());
                } while (true);
                
                if (decompressedSize > 0) {
                    return std::make_tuple(extType, decompressedSize);
                }
            }
        }
        
        if (valuePtr) {
            *valuePtr = object.as<Datum>();
        }
        
        return std::make_tuple(0, 0);
    } catch (const std::exception &) {  // All msgpack exceptions derive from std::exception
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Data file contains invalid or corrupt event data");
    }
}


MWK2Writer::MWK2Writer(const std::string &filename, std::size_t pageSize) :
    packer(packingBuffer)
{
    // Try creating the file to ensure it doesn't already exist
    auto fp = fopen(filename.c_str(), "wxb");
    if (!fp) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Cannot create data file", strerror(errno));
    }
    (void)fclose(fp);
    
    // Open database connection, create "concat_event_data" function, create table,
    // and prepare SQL statements
    int result;
    if (SQLITE_OK != (result = sqlite3_open_v2(filename.c_str(),
                                               &conn,
                                               SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                                               nullptr)) ||
        SQLITE_OK != (result = sqlite3_create_function(conn,
                                                       "concat_event_data",
                                                       2,
                                                       SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                                                       this,
                                                       _concatEventData,
                                                       nullptr,
                                                       nullptr)) ||
        SQLITE_OK != (result = sqlite3_exec(conn,
                                            (boost::format(createTableSQL) % pageSize).str().c_str(),
                                            nullptr,
                                            nullptr,
                                            nullptr)) ||
        SQLITE_OK != (result = prepareStatement(insertSQL, insertStatement)) ||
        SQLITE_OK != (result = prepareStatement("BEGIN TRANSACTION", beginTransactionStatement)) ||
        SQLITE_OK != (result = prepareStatement("ROLLBACK TRANSACTION", rollbackTransactionStatement)) ||
        SQLITE_OK != (result = prepareStatement("COMMIT TRANSACTION", commitTransactionStatement)))
    {
        throwSQLError(result, "Cannot create data file");
    }
}


void MWK2Writer::writeEvent(int code, MWTime time, const Datum &data) {
    int result;
    
    if (SQLITE_OK != (result = sqlite3_bind_int(insertStatement.get(), codeColumn + 1, code)) ||
        SQLITE_OK != (result = sqlite3_bind_int64(insertStatement.get(), timeColumn + 1, time)) ||
        SQLITE_OK != (result = bindDatum(insertStatement.get(), dataColumn + 1, data)) ||
        SQLITE_DONE != (result = evaluateStatement(insertStatement.get())))
    {
        if (!sqlite3_get_autocommit(conn)) {
            // Roll back the active transaction
            auto rollbackResult = evaluateStatement(rollbackTransactionStatement.get());
            if (SQLITE_DONE != rollbackResult) {
                logSQLError(rollbackResult, "Cannot rollback SQL transaction");
            }
        }
        throwSQLError(result, "Cannot write event to data file");
    }
}


void MWK2Writer::writeEvent(const boost::shared_ptr<Event> &event) {
    writeEvent(event->getEventCode(), event->getTime(), event->getData());
}


void MWK2Writer::writeEvents(const std::vector<boost::shared_ptr<Event>> &events) {
    beginTransaction();
    for (auto &event : events) {
        writeEvent(event);
    }
    commitTransaction();
}


void MWK2Writer::beginTransaction() {
    auto result = evaluateStatement(beginTransactionStatement.get());
    if (SQLITE_DONE != result) {
        throwSQLError(result, "Cannot begin SQL transaction");
    }
}


void MWK2Writer::commitTransaction() {
    auto result = evaluateStatement(commitTransactionStatement.get());
    if (SQLITE_DONE != result) {
        throwSQLError(result, "Cannot commit SQL transaction");
    }
}


void MWK2Writer::_concatEventData(sqlite3_context *context, int numValues, sqlite3_value **values) {
    auto &writer = *static_cast<MWK2Writer *>(sqlite3_user_data(context));
    try {
        writer.concatEventData(context, numValues, values);
    } catch (const std::bad_alloc &) {
        sqlite3_result_error_nomem(context);
    } catch (const std::exception &e) {
        sqlite3_result_error(context, e.what(), -1);
    } catch (...) {
        sqlite3_result_error(context, "An unknown error occurred", -1);
    }
}


void MWK2Writer::concatEventData(sqlite3_context *context, int numValues, sqlite3_value **values) {
    packingBuffer.clear();
    
    for (int index = 0; index < numValues; index++) {
        auto value = values[index];
        
        switch (sqlite3_value_type(value)) {
            case SQLITE_INTEGER:
                packer.pack(sqlite3_value_int64(value));
                break;
                
            case SQLITE_FLOAT:
                packer.pack(sqlite3_value_double(value));
                break;
                
            case SQLITE_TEXT: {
                auto text = sqlite3_value_text(value);
                auto size = sqlite3_value_bytes(value);
                packer.pack_str(size);
                packer.pack_str_body(reinterpret_cast<const char *>(text), size);
                break;
            }
                
            case SQLITE_BLOB: {
                auto blob = static_cast<const char *>(sqlite3_value_blob(value));
                auto size = sqlite3_value_bytes(value);
                
                int extType;
                std::size_t decompressedSize;
                std::size_t offset = 0;
                std::tie(extType, decompressedSize) = unpack(blob, size, offset, true);
                
                switch (extType) {
                    case compressedTextTypeCode:
                        packer.pack_str(decompressedSize);
                        packer.pack_str_body(reinterpret_cast<const char *>(compressionBuffer.data()),
                                             decompressedSize);
                        break;
                        
                    case compressedMsgPackStreamTypeCode:
                        blob = reinterpret_cast<const char *>(compressionBuffer.data());
                        size = decompressedSize;
                        [[clang::fallthrough]];
                        
                    default:
                        // Value is already msgpack encoded, so just copy it
                        packingBuffer.write(blob, size);
                        break;
                }
                
                break;
            }
                
            case SQLITE_NULL:
                [[clang::fallthrough]];
                
            default:
                packer.pack_nil();
                break;
        }
    }
    
    tryCompression(packingBuffer.data(), packingBuffer.size(), compressedMsgPackStreamTypeCode);
    
    sqlite3_result_blob64(context,
                          packingBuffer.data(),
                          packingBuffer.size(),
                          SQLITE_TRANSIENT);
}


int MWK2Writer::bindDatum(sqlite3_stmt *stmt, int index, const Datum &datum) {
    switch (datum.getDataType()) {
        case M_BOOLEAN:
            [[clang::fallthrough]];
            
        case M_INTEGER:
            return sqlite3_bind_int64(stmt, index, datum.getInteger());
            
        case M_FLOAT:
            return sqlite3_bind_double(stmt, index, datum.getFloat());
            
        case M_STRING: {
            auto &str = datum.getString();
            if (datum.stringIsCString()) {
                if (datum.isCompressible() && tryCompression(str.data(), str.size(), compressedTextTypeCode)) {
                    return sqlite3_bind_blob64(stmt,
                                               index,
                                               packingBuffer.data(),
                                               packingBuffer.size(),
                                               SQLITE_TRANSIENT);
                }
                return sqlite3_bind_text64(stmt,
                                           index,
                                           str.data(),
                                           str.size(),
                                           SQLITE_TRANSIENT,
                                           SQLITE_UTF8);
            }
            packingBuffer.clear();
            packer.pack_bin(str.size());
            packer.pack_bin_body(str.data(), str.size());
            break;
        }
            
        case M_LIST:
            packingBuffer.clear();
            packer.pack(datum.getList());
            break;
            
        case M_DICTIONARY:
            packingBuffer.clear();
            packer.pack(datum.getDict());
            break;
            
        case M_UNDEFINED:
            [[clang::fallthrough]];
            
        default:
            return sqlite3_bind_null(stmt, index);
    }
    
    if (datum.isCompressible()) {
        tryCompression(packingBuffer.data(), packingBuffer.size(), compressedMsgPackStreamTypeCode);
    }
    
    return sqlite3_bind_blob64(stmt,
                               index,
                               packingBuffer.data(),
                               packingBuffer.size(),
                               SQLITE_TRANSIENT);
}


bool MWK2Writer::tryCompression(const char *data, std::size_t size, int extTypeCode) {
    // Always resize compressionBuffer to size of data, since compression is pointless
    // if the compressed data is larger than the original
    compressionBuffer.resize(size);
    auto compressedSize = compression_encode_buffer(compressionBuffer.data(),
                                                    compressionBuffer.size(),
                                                    reinterpret_cast<const std::uint8_t *>(data),
                                                    size,
                                                    scratchBuffer.data(),
                                                    COMPRESSION_ZLIB);
    auto shouldCompress = (compressedSize > 0 && compressedSize < size);
    if (shouldCompress) {
        packingBuffer.clear();
        packer.pack_ext(compressedSize, extTypeCode);
        packer.pack_ext_body(reinterpret_cast<const char *>(compressionBuffer.data()), compressedSize);
    }
    return (shouldCompress && packingBuffer.size() < size);
}


int MWK2Writer::evaluateStatement(sqlite3_stmt *stmt) {
    auto result = sqlite3_step(stmt);
    // Reset after evaluating, because this will cause automatically started transactions
    // to be committed
    (void)sqlite3_reset(stmt);
    return result;
}


MWK2Reader::MWK2Reader(const std::string &filename) :
    lastCode(-1),
    lastTime(-1),
    unpackingBufferOffset(0)
{
    // Try opening the file to ensure it exists
    auto fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Cannot open data file", strerror(errno));
    }
    (void)fclose(fp);
    
    // Open database connection, create "event_data_count" function, and prepare SQL statements
    int result;
    if (SQLITE_OK != (result = sqlite3_open_v2(filename.c_str(),
                                               &conn,
                                               SQLITE_OPEN_READWRITE,
                                               nullptr)) ||
        SQLITE_OK != (result = sqlite3_create_function(conn,
                                                       "event_data_count",
                                                       1,
                                                       SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                                                       this,
                                                       _eventDataCount,
                                                       nullptr,
                                                       nullptr)) ||
        SQLITE_OK != (result = prepareStatement("SELECT sum(event_data_count(data)) FROM events", selectEventCountStatement)) ||
        SQLITE_OK != (result = prepareStatement("SELECT min(time) FROM events", selectMinTimeStatement)) ||
        SQLITE_OK != (result = prepareStatement("SELECT max(time) FROM events", selectMaxTimeStatement)) ||
        // Try preparing the select events statement to confirm that all the expected columns are present
        SQLITE_OK != (result = prepareStatement(selectSQL, selectEventsStatement)))
    {
        throwSQLError(result, "Cannot open data file");
    }
}


static inline sqlite3_int64 evaluateAggregateValueStatement(sqlite3_stmt *stmt, const std::string &failureMsg) {
    (void)sqlite3_reset(stmt);
    auto result = sqlite3_step(stmt);
    if (SQLITE_ROW != result) {
        throwSQLError(result, failureMsg);
    }
    return sqlite3_column_int64(stmt, 0);
}


std::size_t MWK2Reader::getNumEvents() {
    return evaluateAggregateValueStatement(selectEventCountStatement.get(),
                                           "Cannot determine number of events in data file");
}


MWTime MWK2Reader::getTimeMin() {
    return evaluateAggregateValueStatement(selectMinTimeStatement.get(),
                                           "Cannot determine minimum event time in data file");
}


MWTime MWK2Reader::getTimeMax() {
    return evaluateAggregateValueStatement(selectMaxTimeStatement.get(),
                                           "Cannot determine maximum event time in data file");
}


void MWK2Reader::selectEvents(const std::unordered_set<int> &codes, MWTime timeMin, MWTime timeMax) {
    if (timeMin > timeMax) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN,
                              "Minimum event time must be less than or equal to maximum event time");
    }
    
    std::vector<std::string> whereParts;
    std::vector<sqlite3_int64> whereParams;
    
    if (!codes.empty()) {
        std::string codeInSQL = "code IN (?";
        auto iter = codes.begin();
        whereParams.push_back(*(iter++));
        for (; iter != codes.end(); iter++) {
            codeInSQL += ", ?";
            whereParams.push_back(*iter);
        }
        codeInSQL += ")";
        whereParts.emplace_back(codeInSQL);
    }
    
    if (timeMin != MIN_MWORKS_TIME() && timeMax != MAX_MWORKS_TIME()) {
        whereParts.emplace_back("time BETWEEN ? AND ?");
        whereParams.push_back(timeMin);
        whereParams.push_back(timeMax);
    } else if (timeMin != MIN_MWORKS_TIME()) {
        whereParts.emplace_back("time >= ?");
        whereParams.push_back(timeMin);
    } else if (timeMax != MAX_MWORKS_TIME()) {
        whereParts.emplace_back("time <= ?");
        whereParams.push_back(timeMax);
    }
    
    auto sql = selectSQL;
    if (!whereParts.empty()) {
        sql += " WHERE " + whereParts.front();
        for (auto iter = whereParts.begin() + 1; iter != whereParts.end(); iter++) {
            sql += " AND " + *iter;
        }
    }
    
    int result;
    if (SQLITE_OK != (result = prepareStatement(sql, selectEventsStatement))) {
        throwSQLError(result, "Cannot prepare SQL SELECT statement");
    }
    
    for (std::size_t i = 0; i < whereParams.size(); i++) {
        if (SQLITE_OK != (result = sqlite3_bind_int64(selectEventsStatement.get(), i + 1, whereParams.at(i)))) {
            throwSQLError(result, "Cannot bind parameters to SQL SELECT statement");
        }
    }
}


bool MWK2Reader::nextEvent(int &code, MWTime &time, Datum &data) {
    if (!selectEventsStatement) {
        // No selection
        return false;
    }
    
    if (unpackingBufferOffset < unpackingBuffer.size()) {
        code = lastCode;
        time = lastTime;
        unpack(unpackingBuffer.data(), unpackingBuffer.size(), unpackingBufferOffset, false, &data);
        return true;
    }
    
    auto result = sqlite3_step(selectEventsStatement.get());
    if (SQLITE_DONE == result) {
        // No more rows in selection
        selectEventsStatement.reset();
        return false;
    }
    
    if (SQLITE_ROW != result) {
        throwSQLError(result, "Cannot read next event from data file");
    }
    
    code = sqlite3_column_int(selectEventsStatement.get(), codeColumn);
    time = sqlite3_column_int64(selectEventsStatement.get(), timeColumn);
    
    switch (sqlite3_column_type(selectEventsStatement.get(), dataColumn)) {
        case SQLITE_INTEGER:
            data = Datum(sqlite3_column_int64(selectEventsStatement.get(), dataColumn));
            break;
            
        case SQLITE_FLOAT:
            data = Datum(sqlite3_column_double(selectEventsStatement.get(), dataColumn));
            break;
            
        case SQLITE_TEXT: {
            auto text = sqlite3_column_text(selectEventsStatement.get(), dataColumn);
            auto size = sqlite3_column_bytes(selectEventsStatement.get(), dataColumn);
            data = Datum(reinterpret_cast<const char *>(text), size);
            break;
        }
            
        case SQLITE_BLOB: {
            auto blob = static_cast<const char *>(sqlite3_column_blob(selectEventsStatement.get(), dataColumn));
            auto size = sqlite3_column_bytes(selectEventsStatement.get(), dataColumn);
            
            int extType;
            std::size_t decompressedSize;
            std::size_t offset = 0;
            std::tie(extType, decompressedSize) = unpack(blob, size, offset, true, &data);
            
            switch (extType) {
                case compressedTextTypeCode:
                    data = Datum(reinterpret_cast<const char *>(compressionBuffer.data()), decompressedSize);
                    break;
                    
                case compressedMsgPackStreamTypeCode:
                    blob = reinterpret_cast<const char *>(compressionBuffer.data());
                    size = decompressedSize;
                    offset = 0;
                    unpack(blob, size, offset, false, &data);
                    break;
                    
                default:
                    break;
            }
            
            if (offset < size) {
                // Store remainder of data for unpacking in a subsequent iteration
                unpackingBuffer.assign(blob+offset, blob+size);
                unpackingBufferOffset = 0;
            }
            
            break;
        }
            
        case SQLITE_NULL:
            [[clang::fallthrough]];
            
        default:
            data = Datum();
            break;
    }
    
    lastCode = code;
    lastTime = time;
    
    return true;
}


void MWK2Reader::_eventDataCount(sqlite3_context *context, int numValues, sqlite3_value **values) {
    auto &reader = *static_cast<MWK2Reader *>(sqlite3_user_data(context));
    try {
        reader.eventDataCount(context, numValues, values);
    } catch (const std::bad_alloc &) {
        sqlite3_result_error_nomem(context);
    } catch (const std::exception &e) {
        sqlite3_result_error(context, e.what(), -1);
    } catch (...) {
        sqlite3_result_error(context, "An unknown error occurred", -1);
    }
}


void MWK2Reader::eventDataCount(sqlite3_context *context, int numValues, sqlite3_value **values) {
    sqlite3_int64 result = 0;
    
    for (int index = 0; index < numValues; index++) {
        result += 1;
        
        auto value = values[index];
        if (sqlite3_value_type(value) == SQLITE_BLOB) {
            auto blob = static_cast<const char *>(sqlite3_value_blob(value));
            auto size = sqlite3_value_bytes(value);
            
            int extType;
            std::size_t decompressedSize;
            std::size_t offset = 0;
            std::tie(extType, decompressedSize) = unpack(blob, size, offset, true);
            
            if (extType != compressedTextTypeCode) {
                if (extType == compressedMsgPackStreamTypeCode) {
                    blob = reinterpret_cast<const char *>(compressionBuffer.data());
                    size = decompressedSize;
                    offset = 0;
                    unpack(blob, size, offset, false);
                }
                while (offset < size) {
                    unpack(blob, size, offset, false);
                    result += 1;
                }
            }
        }
    }
    
    sqlite3_result_int64(context, result);
}


END_NAMESPACE_MW
