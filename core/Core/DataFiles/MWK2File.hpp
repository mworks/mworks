//
//  MWK2File.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 9/7/18.
//

#ifndef MWK2File_hpp
#define MWK2File_hpp

#include <string>

#include <boost/noncopyable.hpp>
#define MSGPACK_DISABLE_LEGACY_NIL
#include <msgpack.hpp>
#include <sqlite3.h>

#include "DataFileReader.hpp"
#include "Event.h"


BEGIN_NAMESPACE_MW


class MWK2File : boost::noncopyable {
    
public:
    ~MWK2File();
    
protected:
    struct StatementDeleter {
        void operator()(sqlite3_stmt *stmt) const;
    };
    using StatementPtr = std::unique_ptr<sqlite3_stmt, StatementDeleter>;
    
    MWK2File() :
        conn(nullptr)
    { }
    
    int prepareStatement(const std::string &sql, StatementPtr &stmtPtr);
    
    sqlite3 *conn;
    
};


class MWK2Writer : public MWK2File {
    
public:
    explicit MWK2Writer(const std::string &filename, std::size_t pageSize = 16384);
    
    void writeEvent(int code, MWTime time, const Datum &data);
    void writeEvent(const boost::shared_ptr<Event> &event);
    void writeEvents(const std::vector<boost::shared_ptr<Event>> &events);
    
private:
    static void _concatEventData(sqlite3_context *context, int numValues, sqlite3_value **values);
    
    void concatEventData(sqlite3_context *context, int numValues, sqlite3_value **values);
    int bindDatum(sqlite3_stmt *stmt, int index, const Datum &datum);
    int evaluateStatement(sqlite3_stmt *stmt);
    
    StatementPtr insertStatement;
    StatementPtr beginTransactionStatement;
    StatementPtr rollbackTransactionStatement;
    StatementPtr commitTransactionStatement;
    
    msgpack::sbuffer buffer;
    msgpack::packer<decltype(buffer)> packer;
    
};


class MWK2Reader : public MWK2File, public DataFileReader {
    
public:
    explicit MWK2Reader(const std::string &filename);
    
    std::size_t getNumEvents() override;
    MWTime getTimeMin() override;
    MWTime getTimeMax() override;
    
    void selectEvents(const std::unordered_set<int> &codes = {},
                      MWTime timeMin = MIN_MWORKS_TIME(),
                      MWTime timeMax = MAX_MWORKS_TIME()) override;
    bool nextEvent(int &code, MWTime &time, Datum &data) override;
    
private:
    static void eventDataCount(sqlite3_context *context, int numValues, sqlite3_value **values);
    
    void unpackNext(Datum &data);
    
    StatementPtr selectEventCountStatement;
    StatementPtr selectMinTimeStatement;
    StatementPtr selectMaxTimeStatement;
    StatementPtr selectEventsStatement;
    
    int lastCode;
    MWTime lastTime;
    msgpack::unpacker unpacker;
    
};


END_NAMESPACE_MW


#endif /* MWK2File_hpp */
