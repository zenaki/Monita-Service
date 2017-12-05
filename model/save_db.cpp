#include "save_db.h"

save_db::save_db()
{
}

void save_db::data_harian(QSqlDatabase db, QString tb_name, QString data, QString type, int debug){
    QString query;
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

    query = "REPLACE INTO " + tb_name + " ( \
                titik_ukur_id, \
                value, \
                epochtime \
            ) VALUES ";
    query = query + data + ";";
    log.write(type, query, debug);
    q.prepare(query);
    if (q.exec()) {
        log.write(type, "Berhasil Tulis Ke Database", debug);
    } else {
        log.write(type, "Gagal Tulis Ke Database", debug);
    }
}

void save_db::create_tabel_data_harian(QSqlDatabase db, QString tb_name, QString type, int debug){
    QString query;
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

    query.clear();
    query = "CREATE TABLE if not exists " + tb_name + " (\
                titik_ukur_id INT(11) NOT NULL, \
                value FLOAT NULL, \
                epochtime INT(11) NOT NULL, \
                PRIMARY KEY (titik_ukur_id, epochtime) \
    );";
    log.write(type, query, debug);
    q.prepare(query);
    q.exec(query);
}

void save_db::create_table_data_punya_skywave(QSqlDatabase db, QString tb_name, QString type, int debug)
{
    QString query;
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

    query.clear();
    query = "CREATE TABLE if not exists " + tb_name + " ("
                "id_titik_ukur INT NOT NULL, "
                "value FLOAT NOT NULL, "
                "id_trip INT NULL DEFAULT NULL, "
                "epochtime INT NOT NULL, "
                "data_time DATETIME NOT NULL, "
                "flag_data INT, "
                "PRIMARY KEY (id_titik_ukur, data_time)"
            ")";
    log.write(type, query, debug);
    q.prepare(query);
    q.exec(query);
}

void save_db::data_punya_skywave(QSqlDatabase db, QString tb_name, QString data, QString type, int debug)
{
    QString query;
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

    query = "REPLACE INTO " + tb_name + " ( \
                id_titik_ukur, \
                value, \
                id_trip, \
                epochtime, \
                data_time, \
                flag_data \
            ) VALUES ";
    query = query + data + ";";
//    qDebug() << query;
    log.write(type, query, debug);
    q.prepare(query);
    q.exec();
}

void save_db::update_multiple_punya_skywave(
        QSqlDatabase db,
        QString tb_name,
        QString col_target,
        QString col_clause,
        QString value_clause,
        QString data,
        QString type,
        int debug)
{
    QString query;
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

    query = "UPDATE " + tb_name + " "
            "SET " + col_target + " = (case "
//            "   when user_role = 'student' then '622057' "
//            "   when user_role = 'assistant' then '2913659' "
//            "   when user_role = 'admin' then '6160230' "
                + data +
            "end ) "
            "WHERE " + col_clause + " in (" + value_clause + ");";
//    qDebug() << query;
    log.write(type, query, debug);
    q.prepare(query);
    q.exec();
}

void save_db::delete_last_utc_punya_skywave(QSqlDatabase db, QString tb_name, QString last_utc, QString type, int debug)
{
    QString query;
    QSqlQuery q(QSqlDatabase::database(db.connectionName()));

    query = "DELETE FROM " + tb_name + " "
            " WHERE epochtime <= " + last_utc + ";";

//    qDebug() << query;
    log.write(type, query, debug);
    q.prepare(query);
    q.exec();
}

//void save_db::create_tabel_data_skywave(QSqlQuery *q)
//{
//    QString query;

//    query.clear();
//    query.sprintf("CREATE TABLE if not exists data ("
//                  "id_titik_ukur INT NOT NULL, "
//                  "value FLOAT NOT NULL, "
//                  "id_trip INT NULL DEFAULT NULL, "
//                  "epochtime INT NOT NULL, "
//                  "data_time DATETIME NOT NULL, "
//                  "flag_data INT, "
//                  "PRIMARY KEY (id_titik_ukur, data_time)"
//                  ")");
//    q->exec(query);
//}

//void save_db::create_tabel_data_harian_skywave(QSqlQuery *q, int index){
//    QString query;

//    query.clear();
//    query.sprintf("CREATE TABLE if not exists data_%d ("
//                  "id_titik_ukur INT NOT NULL, "
//                  "value FLOAT NOT NULL, "
//                  "id_trip INT NULL DEFAULT NULL, "
//                  "epochtime INT NOT NULL, "
//                  "data_time DATETIME NOT NULL, "
//                  "flag_data INT, "
//                  "PRIMARY KEY (id_titik_ukur, data_time)"
//                  ")", index);
//    q->exec(query);
//}

//void save_db::data_skywave(QSqlQuery *q, float value, int id_tu, int id_trip, int epochtime, QString data_time, int flag){
//    //printf("insert id_tu : %d --> value : %.2f\n", id_tu, value);

//    QString query;
//    query.sprintf("REPLACE INTO data ("
//                  "id_titik_ukur, "
//                  "value, "
//                  "id_trip, "
//                  "epochtime, "
//                  "data_time, "
//                  "flag_data"
//                  ") VALUES ("
//                  "%d, "
//                  "%.2f, %d, %d, '%s', %d)",
//                  id_tu, value, id_trip, epochtime, data_time.toUtf8().data(), flag);

//    printf("%s\n", query.toUtf8().data());

//    q->prepare(query);
//    q->exec();


//    /*
//    q->prepare("REPLACE INTO data(id_titik_ukur, value, id_trip, epochtime, data_time) VALUES(:id_titik_ukur, :value, :id_trip, :epochtime, :data_time)");

//    q->bindValue(":id_titik_ukur", id_tu);
//    q->bindValue(":value", value);
//    q->bindValue(":id_trip", id_trip);
//    q->bindValue(":epochtime", epochtime);
//    q->bindValue(":data_time", data_time.toUtf8().data());
//    */
//}

//void save_db::data_harian_skywave(QSqlQuery *q, float value, int id_tu, int id_trip, int epochtime, QString data_time, int index, int flag){
//    QString query;
//    query.sprintf("REPLACE INTO data_%d(id_titik_ukur, value, id_trip, epochtime, data_time, flag_data) VALUES(:id_titik_ukur, :value, :id_trip, :epochtime, :data_time, :flag_data)", index);

//    q->prepare(query);

//    q->bindValue(":id_titik_ukur", id_tu);
//    q->bindValue(":value", value);
//    q->bindValue(":id_trip", id_trip);
//    q->bindValue(":epochtime", epochtime);
//    q->bindValue(":data_time", data_time.toUtf8().data());
//    q->bindValue(":flag_data", flag);

//    q->exec();
//}

//void save_db::update_next_utc_skywave(QSqlDatabase db, QString next_utc, int id_ship){
//    QString query;
//    query.sprintf("UPDATE ship SET nextutc = '%s' where id_ship = %d", next_utc.toUtf8().data(), id_ship);

//    QSqlQuery q(db);
//    q.first();
//    q.exec(query);
//    q.finish();
//}

//void save_db::update_next_utc_gateway_skywave(QSqlDatabase db, QString next_utc, int id_gateway){
//    QString query;
//    query.sprintf("UPDATE gateway set next_utc = '%s' where id = %d", next_utc.toUtf8().data(), id_gateway);

//    QSqlQuery q(db);
//    q.first();
//    q.exec(query);
//    q.finish();
//}
