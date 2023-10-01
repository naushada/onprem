#ifndef __mongodbc_cc__
#define __mongodbc_cc__

#include <vector>
#include <sstream>

#include "mongodbc.h"

MongodbClient::MongodbClient()
{
    mInstance = nullptr;
    mURI.clear();
    mdbName.clear();
    mMongoConnPool = nullptr;
}

MongodbClient::MongodbClient(std::string uri_str)
{
    mInstance = nullptr;
    mMongoConnPool = nullptr;
    mURI = uri_str;

    do {

        mInstance = std::make_unique<mongocxx::v_noabi::instance>();
        if(nullptr == mInstance) {
            break;
        }

        /* pool of connections*/
        //std::string poolUri(uri_str);

        /* reference: http://mongocxx.org/mongocxx-v3/connection-pools/ */
        //poolUri += "/?minPoolSize=10&maxPoolSize=" + std::to_string(poolSize);
        //poolUri += "&minPoolSize=10&maxPoolSize=" + std::to_string(poolSize);
        //poolUri += "&maxPoolSize=" + std::to_string();

        mongocxx::uri uri(uri_str.c_str());
        mMongoConnPool = std::make_unique<mongocxx::pool>(uri);

        if(nullptr == mMongoConnPool) {
            break;
        }

        set_database(uri.database());

    }while(0);
}

MongodbClient::~MongodbClient()
{

    mInstance.reset(nullptr);
    mMongoConnPool.reset(nullptr);

}

bool MongodbClient::update_collection(std::string collectionName, std::string match, std::string document)
{
    bsoncxx::document::value toUpdate = bsoncxx::from_json(document.c_str());
    bsoncxx::document::value filter = bsoncxx::from_json(match.c_str());

    auto conn = mMongoConnPool->acquire();
    if(!conn) {
        return(false);
    }

    mongocxx::database dbInst = conn->database(get_database().c_str());
    auto collection = dbInst.collection(collectionName.c_str());

    mongocxx::options::bulk_write bulk_opt;
    mongocxx::write_concern wc;
    bulk_opt.ordered(false);
    wc.acknowledge_level(mongocxx::write_concern::level::k_default);
    bulk_opt.write_concern(wc);
    auto bulk = collection.create_bulk_write(bulk_opt);

    mongocxx::model::update_many upd(filter.view(), toUpdate.view());
    bulk.append(upd);

    auto result = bulk.execute();
    std::int32_t cnt = 0;
    if(result) {
        cnt = result->matched_count();
        if(!cnt) {
            return(false);
        }
        return(true);
    }
    
    return(false);
}

bool MongodbClient::delete_document(std::string collectionName, std::string doc)
{
    bsoncxx::document::value filter = bsoncxx::from_json(doc.c_str());
    auto conn = mMongoConnPool->acquire();

    if(!conn) {
        return(false);
    }

    mongocxx::database dbInst = conn->database(get_database().c_str());
    if(!dbInst) {
        return(false);
    }
    auto collection = dbInst.collection(collectionName.c_str());

    mongocxx::options::bulk_write bulk_opt;
    mongocxx::write_concern wc;
    bulk_opt.ordered(false);
    wc.acknowledge_level(mongocxx::write_concern::level::k_default);
    bulk_opt.write_concern(wc);
    auto bulk = collection.create_bulk_write(bulk_opt);

    mongocxx::model::delete_many del(filter.view());
    bulk.append(del);

    auto result = bulk.execute();
    std::int32_t cnt = 0;

    if(result) {
        cnt = result->deleted_count();
    }

    if(result) {
        return(true);
    }

    return(false);
}


bool MongodbClient::delete_documents(std::string collectionName, std::string doc)
{
    bsoncxx::document::value filter = bsoncxx::from_json(doc.c_str());
    auto conn = mMongoConnPool->acquire();

    if(!conn) {
        return(false);
    }

    mongocxx::database dbInst = conn->database(get_database().c_str());
    if(!dbInst) {
        return(false);
    }
    
    auto collection = dbInst.collection(collectionName.c_str());

    mongocxx::options::bulk_write bulk_opt;
    mongocxx::write_concern wc;
    bulk_opt.ordered(false);
    wc.acknowledge_level(mongocxx::write_concern::level::k_default);
    bulk_opt.write_concern(wc);
    auto bulk = collection.create_bulk_write(bulk_opt);

    mongocxx::model::delete_many del(filter.view());
    bulk.append(del);

    auto result = bulk.execute();
    std::int32_t cnt = 0;

    if(result) {
        cnt = result->deleted_count();
    }

    if(result) {
        return(true);
    }

    return(false);
}

std::string MongodbClient::get_document(std::string collectionName, std::string query, std::string fieldProjection)
{
    std::string json_object;
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());

    auto conn = mMongoConnPool->acquire();
    if(!conn) {
        return(std::string());
    }

    mongocxx::database dbInst = conn->database(get_database().c_str());

    if(!dbInst) {
        return(std::string());
    }

    auto collection = dbInst.collection(collectionName.c_str());

    mongocxx::options::find opts{};
    using namespace std::literals::chrono_literals;
    std::chrono::milliseconds ms(5000);

    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(fieldProjection.c_str());
    opts.max_time(ms).no_cursor_timeout(false).projection(outputProjection);

    auto cursor = collection.find(filter.view(), opts);
    //mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), opts);

    mongocxx::cursor::iterator iter = cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }

    return(std::string(bsoncxx::to_json(*iter).c_str()));
}

std::string MongodbClient::get_documents(std::string collectionName, std::string query, std::string fieldProjection)
{
    std::string json_object;
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());

    auto conn = mMongoConnPool->acquire();
    if(!conn) {
        return(std::string());
    }

    mongocxx::database dbInst = conn->database(get_database().c_str());
    if(!dbInst) {
        return(std::string());
    }

    auto collection = dbInst.collection(collectionName.c_str());

    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(fieldProjection.c_str());

    using namespace std::literals::chrono_literals;
    mongocxx::options::find opts{};
    std::chrono::milliseconds ms(5000);

    opts.max_time(ms).no_cursor_timeout(false).projection(outputProjection);

    mongocxx::cursor cursor = collection.find(filter.view(), opts);
    mongocxx::cursor::iterator iter = cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }

    std::stringstream result("");
    result << "[";

    for(; iter != cursor.end(); ++iter) {
        result << bsoncxx::to_json(*iter).c_str()
               << ",";
    }

    result.seekp(-1, std::ios_base::end);
    result << "]";

    return(std::string(result.str()));
}

std::string MongodbClient::get_documents(std::string collectionName, std::string projection)
{
    auto conn = mMongoConnPool->acquire();
    if(!conn) {
        return(std::string());
    }

    mongocxx::database dbInst = conn->database(get_database().c_str());
    if(!dbInst) {
        return(std::string());
    }

    auto collection = dbInst.collection(collectionName.c_str());
    bsoncxx::document::view_or_value filter;

    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(projection.c_str());

    using namespace std::literals::chrono_literals;
    mongocxx::options::find opts{};
    std::chrono::milliseconds ms(5000);

    opts.max_time(ms).no_cursor_timeout(false).projection(outputProjection);
    mongocxx::cursor cursor = collection.find({}, opts);
    mongocxx::cursor::iterator iter = cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }

    std::stringstream result("");
    result << "[";

    for(; iter != cursor.end(); ++iter) {
        result << bsoncxx::to_json(*iter).c_str()
               << ",";
    }

    result.seekp(-1, std::ios_base::end);
    result << "]";

    return(std::string(result.str()));
}

/**
 * @brief this member function insert singlr document in a collection and returns the inserted OID
 * 
 * @param dbName 
 * @param collectionName 
 * @param doc 
 * @return std::string 
 */
std::string MongodbClient::create_document(std::string dbName, std::string collectionName, std::string doc)
{
    bsoncxx::document::value document = bsoncxx::from_json(doc.c_str());
    
    auto conn = mMongoConnPool->acquire();
    if(!conn) {
        return(std::string());
    }

    mongocxx::database dbInst = conn->database(dbName.c_str());
    if(!dbInst) {
        return(std::string());
    }

    auto collection = dbInst.collection(collectionName.c_str());
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection.insert_one(document.view());

    if(result) {
        bsoncxx::oid oid = result->inserted_id().get_oid().value;
        std::string JobID = oid.to_string();
        return(JobID);
    }

    return(std::string());
}

/**
 * @brief this member function insert singlr document in a collection and returns the inserted OID
 * 
 * @param dbName 
 * @param collectionName 
 * @param doc 
 * @return std::string 
 */
std::string MongodbClient::create_document(std::string collectionName, std::string doc)
{
    bsoncxx::document::value document = bsoncxx::from_json(doc.c_str());
    
    auto conn = mMongoConnPool->acquire();
    if(!conn) {
        return(std::string());
    }

    mongocxx::database dbInst = conn->database(get_database().c_str());
    if(!dbInst) {
        return(std::string());
    }

    auto collection = dbInst.collection(collectionName.c_str());
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection.insert_one(document.view());

    if(result) {
        bsoncxx::oid oid = result->inserted_id().get_oid().value;
        std::string JobID = oid.to_string();
        return(JobID);
    }

    return(std::string());
}

std::string MongodbClient::get_byOID(std::string coll, std::string projection, std::string oid)
{
    auto conn = mMongoConnPool->acquire();
    mongocxx::database dbInst = conn->database(get_database().c_str());
    auto collection = dbInst.collection(coll.c_str());

    std::string query("{\"_id\" : {\"$oid\": \"");
    query += oid + "\"}}";

    //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [Worker:%t] %M %N:%l The query is %s\n"), query.c_str()));
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
    mongocxx::options::find opts{};
    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(projection.c_str());
    auto resultFormat = opts.projection(outputProjection);
    mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), {});
    mongocxx::cursor::iterator iter = cursor.begin();
    bsoncxx::document::view res = *cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }

    std::stringstream rsp("");
    rsp << bsoncxx::to_json(*iter);
    return(rsp.str());
}

std::string MongodbClient::get_documentList(std::string collectionName, std::string query, std::string fieldProjection)
{
    std::string json_object;
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
    auto conn = mMongoConnPool->acquire();
    if(!conn) {
        return(std::string());
    }

    mongocxx::database dbInst = conn->database(get_database().c_str());
    if(!dbInst) {
        return(std::string());
    }

    auto collection = dbInst.collection(collectionName.c_str());

    using namespace std::literals::chrono_literals;
    std::chrono::milliseconds ms = 5s;
    mongocxx::options::find opts{};
    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(fieldProjection.c_str());
    opts.max_time(ms).no_cursor_timeout(false).projection(outputProjection);
    mongocxx::v_noabi::cursor cursor = std::move(collection.find(filter.view(), opts));
    mongocxx::cursor::iterator iter = cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }

    std::stringstream result("");
    result << "[";

    for (auto&& doc : cursor) {
        result << bsoncxx::to_json(doc).c_str()
               << ",";
    }
    result.seekp(-1, std::ios_base::end);
    result << "]";
    return(std::string(result.str()));
}

/**
 * @brief This member function insert the multiple documents in a collection for a given uri.
 * 
 * @param collectionName 
 * @param doc 
 * @return std::int32_t 
 */
std::int32_t MongodbClient::create_bulk_document(std::string dbName, std::string collectionName, std::string doc)
{
    std::int32_t cnt = 0;
    mongocxx::options::bulk_write bulk_opt;
    mongocxx::write_concern wc;
    bulk_opt.ordered(false);
    wc.acknowledge_level(mongocxx::write_concern::level::k_default);
    bulk_opt.write_concern(wc);

    bsoncxx::document::value new_shipment = bsoncxx::from_json(doc.c_str());
    auto conn = mMongoConnPool->acquire();
    if(!conn) {
        return(cnt);
    }

    mongocxx::database dbInst = conn->database(dbName.c_str());
    if(!dbInst) {
        return(cnt);
    }

    auto collection = dbInst.collection(collectionName.c_str());

    auto bulk = collection.create_bulk_write(bulk_opt);

    bsoncxx::document::view dock_view = new_shipment.view();
    auto iter = dock_view.begin();
    
    for(; iter != dock_view.end(); ++iter) {
        bsoncxx::document::element elm = *iter;
        mongocxx::model::insert_one insert_op(elm.get_document().value);
        bulk.append(insert_op);
    }

    auto result = bulk.execute();

    if(result) {
        cnt = result->inserted_count();
    }

    return(cnt);
}

std::string MongodbClient::upload_file(std::string fileName, std::string& content, std::uint32_t len) {
    std::cout << "\nline: " << __LINE__ << " entring upload_file " << std::endl;
    auto conn = mMongoConnPool->acquire();
    if(!conn) {
        return(std::string());
    }

    mongocxx::database db = conn->database(get_database().c_str());
    if(!db) {
        return(std::string());
    }

    auto bucket = db.gridfs_bucket();
    auto uploader = bucket.open_upload_stream(fileName);
    std::cout << "line: " << __LINE__ << " after open_upload_stream " << std::endl;
    std::cout << "line: " << __LINE__ << " filename: " << fileName << " content.length(): " << len << std::endl;
    uploader.write((std::uint8_t *)content.data(), len);
    auto result = uploader.close();
    bsoncxx::types::bson_value::view id = result.id();
    //std::cout << "line: " << __LINE__ << " file_id: " << id.data() << std::endl;
    //return(bsoncxx::string::to_string());

}

std::string MongodbClient::download_file(std::string fileId) {

}

/** 
 * 
*/
std::string MongodbClient::get_documentEx(std::string collectionName, std::string query, std::string fieldProjection)
{
    std::string json_object;
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
    mongocxx::uri uri(get_uri());
    mongocxx::client conn(uri);

    if(!conn) {
        return(std::string());
    }

    mongocxx::database dbInst = conn.database(uri.database().c_str());
    if(!dbInst) {
        return(std::string());
    }

    auto collection = dbInst.collection(collectionName.c_str());

    mongocxx::options::find opts{};
    using namespace std::literals::chrono_literals;
    std::chrono::milliseconds ms = 5s;
    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(fieldProjection.c_str());
    opts.max_time(ms).no_cursor_timeout(false).projection(outputProjection);
    auto cursor = collection.find(filter.view(), opts);
    //mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), opts);

    mongocxx::cursor::iterator iter = cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }

    return(std::string(bsoncxx::to_json(*iter).c_str()));
}

std::string MongodbClient::get_documentsEx(std::string collectionName, std::string query, std::string fieldProjection)
{
    std::string json_object;
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());

    mongocxx::uri uri(get_uri());
    mongocxx::client conn(uri);

    if(!conn) {
        return(std::string());
    }

    mongocxx::database dbInst = conn.database(uri.database().c_str());
    if(!dbInst) {
        return(std::string());
    }

    auto collection = dbInst.collection(collectionName.c_str());

    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(fieldProjection.c_str());

    using namespace std::literals::chrono_literals;
    mongocxx::options::find opts{};
    std::chrono::milliseconds ms = 5s;

    opts.max_time(ms).no_cursor_timeout(false).projection(outputProjection);
    mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), opts);
    mongocxx::cursor::iterator iter = cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }

    std::stringstream result("");
    result << "[";

    for(; iter != cursor.end(); ++iter) {
        result << bsoncxx::to_json(*iter).c_str()
               << ",";
    }

    result.seekp(-1, std::ios_base::end);
    result << "]";

    return(std::string(result.str()));
}

bool MongodbClient::update_collectionEx(std::string collectionName, std::string match, std::string document)
{
    bsoncxx::document::value toUpdate = bsoncxx::from_json(document.c_str());
    bsoncxx::document::value filter = bsoncxx::from_json(match.c_str());

    mongocxx::uri uri(get_uri());
    mongocxx::client conn(uri);
    if(!conn) {
        return(false);
    }

    mongocxx::database dbInst = conn.database(uri.database().c_str());
    auto collection = dbInst.collection(collectionName.c_str());

    mongocxx::options::bulk_write bulk_opt;
    mongocxx::write_concern wc;
    bulk_opt.ordered(false);
    wc.acknowledge_level(mongocxx::write_concern::level::k_default);
    bulk_opt.write_concern(wc);
    auto bulk = collection.create_bulk_write(bulk_opt);

    mongocxx::model::update_many upd(filter.view(), toUpdate.view());
    bulk.append(upd);

    auto result = bulk.execute();
    std::int32_t cnt = 0;
    if(result) {
        cnt = result->matched_count();
        if(!cnt) {
            return(false);
        }
        return(true);
    }

    return(false);
}

bool MongodbClient::delete_documentEx(std::string collectionName, std::string doc)
{
    bsoncxx::document::value filter = bsoncxx::from_json(doc.c_str());
    mongocxx::uri uri(get_uri());
    mongocxx::client conn(uri);

    if(!conn) {
        return(false);
    }

    mongocxx::database dbInst = conn.database(uri.database().c_str());
    if(!dbInst) {
        return(false);
    }
    auto collection = dbInst.collection(collectionName.c_str());

    mongocxx::options::bulk_write bulk_opt;
    mongocxx::write_concern wc;
    bulk_opt.ordered(false);
    wc.acknowledge_level(mongocxx::write_concern::level::k_default);
    bulk_opt.write_concern(wc);
    auto bulk = collection.create_bulk_write(bulk_opt);

    mongocxx::model::delete_many del(filter.view());
    bulk.append(del);

    auto result = bulk.execute();
    std::int32_t cnt = 0;

    if(result) {
        cnt = result->deleted_count();
    }

    if(result) {
        return(true);
    } else {
        return(false);
    }
}

std::string MongodbClient::create_documentEx(std::string collectionName, std::string doc)
{
    bsoncxx::document::value document = bsoncxx::from_json(doc.c_str());
    
    mongocxx::uri uri(get_uri());
    mongocxx::client conn(uri);
    if(!conn) {
        return(std::string());
    }

    mongocxx::database dbInst = conn.database(uri.database().c_str());
    if(!dbInst) {
        return(std::string());
    }

    auto collection = dbInst.collection(collectionName.c_str());
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection.insert_one(document.view());

    if(result) {
        bsoncxx::oid oid = result->inserted_id().get_oid().value;
        std::string JobID = oid.to_string();
        return(JobID);
    }

    return(std::string());
}

std::string MongodbClient::get_documentListEx(std::string collectionName, std::string query, std::string fieldProjection)
{
    std::string json_object;
    bsoncxx::document::value filter = bsoncxx::from_json(query.c_str());
    mongocxx::uri uri(get_uri());
    mongocxx::client conn(uri);

    if(!conn) {
        return(std::string());
    }

    mongocxx::database dbInst = conn.database(uri.database().c_str());
    if(!dbInst) {
        return(std::string());
    }

    auto collection = dbInst.collection(collectionName.c_str());

    using namespace std::literals::chrono_literals;
    std::chrono::milliseconds ms = 5s;
    mongocxx::options::find opts{};
    bsoncxx::document::view_or_value outputProjection = bsoncxx::from_json(fieldProjection.c_str());
    opts.max_time(ms).no_cursor_timeout(false).projection(outputProjection);
    mongocxx::v_noabi::cursor cursor = collection.find(filter.view(), opts);
    mongocxx::cursor::iterator iter = cursor.begin();

    if(iter == cursor.end()) {
        return(std::string());
    }

    std::stringstream result("");
    result << "[";

    for (auto&& doc : cursor) {
        result << bsoncxx::to_json(doc).c_str()
               << ",";
    }
    result.seekp(-1, std::ios_base::end);
    result << "]";
    return(std::string(result.str()));
}

std::int32_t MongodbClient::create_bulk_documentEx(std::string collectionName, std::string doc)
{
    std::int32_t cnt = 0;
    mongocxx::options::bulk_write bulk_opt;
    mongocxx::write_concern wc;
    bulk_opt.ordered(false);
    wc.acknowledge_level(mongocxx::write_concern::level::k_default);
    bulk_opt.write_concern(wc);

    bsoncxx::document::value new_shipment = bsoncxx::from_json(doc.c_str());
    mongocxx::uri uri(get_uri());
    mongocxx::client conn(uri);

    if(!conn) {
        return(cnt);
    }

    mongocxx::database dbInst = conn.database(uri.database().c_str());
    if(!dbInst) {
        return(cnt);
    }

    auto collection = dbInst.collection(collectionName.c_str());

    auto bulk = collection.create_bulk_write(bulk_opt);

    bsoncxx::document::view dock_view = new_shipment.view();
    auto iter = dock_view.begin();
    
    for(; iter != dock_view.end(); ++iter) {
        bsoncxx::document::element elm = *iter;
        mongocxx::model::insert_one insert_op(elm.get_document().value);
        bulk.append(insert_op);
    }

    auto result = bulk.execute();

    if(result) {
        cnt = result->inserted_count();
    }

    return(cnt);
}
/*
std::int32_t MongodbClient::update_bulk_shipment(std::string bulkShipment)
{

}
*/

std::uint32_t MongodbClient::from_json_array_to_vector(const std::string json_obj, const std::string key, std::vector<std::string>& vec_out)
{
  bsoncxx::document::value doc_val = bsoncxx::from_json(json_obj.c_str());
  bsoncxx::document::view doc = doc_val.view();

  auto it = doc.find(key);
  if(it == doc.end()) {
    return(1);    
  }

  bsoncxx::document::element elm_value = *it;
  if(elm_value && bsoncxx::type::k_array == elm_value.type()) {
     bsoncxx::array::view to(elm_value.get_array().value);
     for(bsoncxx::array::element elm : to) {
        if(bsoncxx::type::k_utf8 == elm.type()) {
            std::string tmp(elm.get_utf8().value.data(), elm.get_utf8().value.length());
            vec_out.push_back(tmp);
        }
     }
  } else {
    vec_out.clear();
  }
  
  return(0);

#if 0
  auto tt = it->get_document().value;
  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l the length is  %d value %s\n"), tt.length(), tt.data()));  
  reference_no = tt["reference"].get_utf8().value.to_string();
  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l the reference is %s\n"), reference_no.c_str()));  

  bsoncxx::document::element elm_value = doc["TrackingNumber"];

  //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l the type is %d\n"), elm_value.type()));
  if(elm_value && elm_value.type() == bsoncxx::type::k_utf8) {
    //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l the length is %d\n"), elm_value.get_utf8().value.length()));
    std::string res(elm_value.get_utf8().value.data(), elm_value.get_utf8().value.length());
    return(res);
  }
  return(std::string());
  #endif
}

std::uint32_t MongodbClient::from_json_element_to_string(const std::string json_obj, const std::string key, std::string& str_out)
{

  bsoncxx::document::value doc_val = bsoncxx::from_json(json_obj.c_str());
  bsoncxx::document::view doc = doc_val.view();

  auto it = doc.find(key);
  if(it == doc.end()) {
    //ACE_ERROR((LM_ERROR, ACE_TEXT("%D [Worker:%t] %M %N:%l The element:%s not found in json document\n"), key.c_str()));
    str_out.clear();
    return(1);    
  }

  bsoncxx::document::element elm_value = *it;
  if(elm_value && bsoncxx::type::k_utf8 == elm_value.type()) {
      std::string elm(elm_value.get_utf8().value.data(), elm_value.get_utf8().value.length());
      str_out = elm;
  } else {
    str_out.clear();
  }
  
  return(0);
}

std::uint32_t MongodbClient::from_json_object_to_map(const std::string json_obj, const std::string key, std::vector<std::tuple<std::string, std::string>>& out)
{
    bsoncxx::document::value doc_val = bsoncxx::from_json(json_obj.c_str());
    bsoncxx::document::view doc = doc_val.view();

    auto it = doc.find(key);
    if(it == doc.end()) {
        //ACE_ERROR((LM_ERROR, ACE_TEXT("%D [Worker:%t] %M %N:%l the element:%s not found in the json document\n"), key.c_str()));
        return(1);    
    }

    bsoncxx::document::element elm_value = *it;
    if(elm_value && bsoncxx::type::k_array == elm_value.type()) {
       bsoncxx::array::view to(elm_value.get_array().value);
       for(bsoncxx::array::element elm : to) {
          if(bsoncxx::type::k_document == elm.type()) {
              /* get the document now */
              auto doc = elm.get_document().value;
              //ACE_DEBUG((LM_DEBUG, ACE_TEXT("%D [worker:%t] %M %N:%l file-name:%s file-content:%s\n"), doc["file-name"].get_utf8().value.data(), 
              //           doc["file-content"].get_utf8().value.data()));
              #if 0
              for(auto it = doc.begin(); it != doc.end(); ++it) {
                  out.push_back(std::make_tuple(doc["file-name"].get_utf8().value.data(), doc["file-content"].get_utf8().value.data()));
              }
              #endif
              out.push_back(std::make_tuple(doc["file-name"].get_utf8().value.data(), doc["file-content"].get_utf8().value.data()));
          }
          #if 0
          if(bsoncxx::type::k_utf8 == elm.type()) {
              std::string tmp(elm.get_utf8().value.data(), elm.get_utf8().value.length());
              vec_out.push_back(tmp);
          }
          #endif
       }
    } else {
        out.clear();
    }
  
    return(0);
}

#endif /* __mongodbc_cc__*/
