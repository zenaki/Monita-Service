#include "save_db.h"

save_db::save_db()
{
}

void save_db::data_harian(QSqlDatabase db, QString tanggal, QString data){
    QString query;
    QSqlQuery q(db);

    query = "REPLACE INTO data_" + tanggal + " ( \
                id_titik_ukur, \
                waktu, \
                data_tunggal, \
                hour, \
                minute, \
                second \
            ) VALUES ";
    query = query + data + ";";
    qDebug() << query;

    q.prepare(query);

//    q->bindValue(":id_titik_ukur", id_titik_ukur);
//    q->bindValue(":waktu", waktu);
//    q->bindValue(":data_tunggal", data_tunggal);
//    q->bindValue(":hour", hour);
//    q->bindValue(":minute", minute);
//    q->bindValue(":second", second);

    q.exec();
}

void save_db::create_tabel_data_harian(QSqlDatabase db, QString tanggal){
    QString query;
    QSqlQuery q(db);

    query.clear();
    query = "CREATE TABLE if not exists data_" + tanggal + " (\
                id_titik_ukur INT(11) NOT NULL, \
                waktu bigint(17) NOT NULL, \
                data_tunggal float NOT NULL, \
                hour tinyint(4) NOT NULL, \
                minute tinyint(4) NOT NULL, \
                second tinyint(4) NOT NULL, \
                PRIMARY KEY (id_titik_ukur, waktu) \
    );";

    q.exec(query);
}
