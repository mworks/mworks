//
//  MWK2File.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/7/18.
//

#include "MWK2File.hpp"

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


const std::string createTableSQL
(R"(
 PRAGMA locking_mode = EXCLUSIVE;
 PRAGMA page_size = %1%;
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


END_NAMESPACE()


MWK2File::~MWK2File() {
    auto result = sqlite3_close(conn);  // Safe to pass NULL
    if (SQLITE_OK != result) {
        logSQLError(result, "Cannot close data file");
    }
}


void MWK2File::StatementDeleter::operator()(sqlite3_stmt *stmt) const {
    // Ignore the return value, as it indicates whether the most recent evaluation
    // of the statement succeeded, not whether the statement was finalized successfully
    (void)sqlite3_finalize(stmt);
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


MWK2Writer::MWK2Writer(const std::string &filename, std::size_t pageSize) :
    packer(buffer)
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
                                                       -1,
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
        SQLITE_OK != (result = bindDatum(insertStatement.get(), dataColumn + 1, data)))
    {
        throwSQLError(result, "Cannot bind parameters to SQL INSERT statement");
    }
    
    result = evaluateStatement(insertStatement.get());
    if (SQLITE_DONE != result) {
        throwSQLError(result, "Cannot execute SQL INSERT statement");
    }
}


void MWK2Writer::writeEvent(const boost::shared_ptr<Event> &event) {
    writeEvent(event->getEventCode(), event->getTime(), event->getData());
}


void MWK2Writer::writeEvents(const std::vector<boost::shared_ptr<Event>> &events) {
    auto result = evaluateStatement(beginTransactionStatement.get());
    if (SQLITE_DONE != result) {
        throwSQLError(result, "Cannot begin SQL transaction");
    }
    
    try {
        for (auto &event : events) {
            writeEvent(event);
        }
    } catch (const SimpleException &) {
        result = evaluateStatement(rollbackTransactionStatement.get());
        if (SQLITE_DONE != result) {
            logSQLError(result, "Cannot rollback SQL transaction");
        }
        throw;
    }
    
    result = evaluateStatement(commitTransactionStatement.get());
    if (SQLITE_DONE != result) {
        throwSQLError(result, "Cannot commit SQL transaction");
    }
}


void MWK2Writer::_concatEventData(sqlite3_context *context, int numValues, sqlite3_value **values) {
    MWK2Writer &writer = *static_cast<MWK2Writer *>(sqlite3_user_data(context));
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
    buffer.clear();
    
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
                
            case SQLITE_BLOB:
                // Value is already msgpack encoded, so just copy it
                buffer.write(reinterpret_cast<const char *>(sqlite3_value_blob(value)), sqlite3_value_bytes(value));
                break;
                
            case SQLITE_NULL:
                [[clang::fallthrough]];
                
            default:
                packer.pack_nil();
                break;
        }
    }
    
    sqlite3_result_blob64(context,
                          buffer.data(),
                          buffer.size(),
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
                return sqlite3_bind_text64(stmt,
                                           index,
                                           str.data(),
                                           str.size(),
                                           SQLITE_TRANSIENT,
                                           SQLITE_UTF8);
            }
            buffer.clear();
            packer.pack_bin(str.size());
            packer.pack_bin_body(str.data(), str.size());
            break;
        }
            
        case M_LIST:
            buffer.clear();
            packer.pack(datum.getList());
            break;
            
        case M_DICTIONARY:
            buffer.clear();
            packer.pack(datum.getDict());
            break;
            
        case M_UNDEFINED:
            [[clang::fallthrough]];
            
        default:
            return sqlite3_bind_null(stmt, index);
    }
    
    return sqlite3_bind_blob64(stmt,
                               index,
                               buffer.data(),
                               buffer.size(),
                               SQLITE_TRANSIENT);
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
    lastTime(-1)
{
    // Try opening the file to ensure it exists
    auto fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Cannot open data file", strerror(errno));
    }
    (void)fclose(fp);
    
    int result;
    if (SQLITE_OK != (result = sqlite3_open_v2(filename.c_str(),
                                               &conn,
                                               SQLITE_OPEN_READONLY,
                                               nullptr)) ||
        SQLITE_OK != (result = sqlite3_create_function(conn,
                                                       "event_data_count",
                                                       1,
                                                       SQLITE_UTF8 | SQLITE_DETERMINISTIC,
                                                       nullptr,
                                                       eventDataCount,
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
    
    if (unpacker.nonparsed_size() > 0) {
        code = lastCode;
        time = lastTime;
        unpackNext(data);
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
            auto blob = sqlite3_column_blob(selectEventsStatement.get(), dataColumn);
            auto size = sqlite3_column_bytes(selectEventsStatement.get(), dataColumn);
            unpacker.reserve_buffer(size);
            std::memcpy(unpacker.buffer(), blob, size);
            unpacker.buffer_consumed(size);
            unpackNext(data);
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


void MWK2Reader::eventDataCount(sqlite3_context *context, int numValues, sqlite3_value **values) {
    try {
        auto value = values[0];
        sqlite3_int64 result = 0;
        if (sqlite3_value_type(value) != SQLITE_BLOB) {
            result = 1;
        } else {
            auto blob = sqlite3_value_blob(value);
            auto size = sqlite3_value_bytes(value);
            std::size_t offset = 0;
            while (offset < size) {
                msgpack::unpack(reinterpret_cast<const char *>(blob), size, offset);  // Discard result
                result += 1;
            }
        }
        sqlite3_result_int64(context, result);
    } catch (const std::bad_alloc &) {
        sqlite3_result_error_nomem(context);
    } catch (const std::exception &e) {  // All msgpack exceptions derive from std::exception
        sqlite3_result_error(context, e.what(), -1);
    } catch (...) {
        sqlite3_result_error(context, "An unknown error occurred", -1);
    }
}


void MWK2Reader::unpackNext(Datum &data) {
    msgpack::object_handle handle;
    try {
        if (!unpacker.next(handle)) {
            throw std::runtime_error("incomplete event data");
        }
        data = handle.get().as<Datum>();
    } catch (const std::exception &) {  // All msgpack exceptions derive from std::exception
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Data file contains invalid or corrupt event data");
    }
}


END_NAMESPACE_MW
