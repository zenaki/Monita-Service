#include "get_db.h"

get_db::get_db()
{
}

bool get_db::check_table_is_available(QSqlDatabase db, QString table_name) {
    QString query;
    QSqlQuery q(db);

    query.clear();
    query = "SELECT * FROM " + table_name + ";";

    q.prepare(query);

    return q.exec();
}

QStringList get_db::get_config(QSqlDatabase db) {
    db.open();
    QSqlQuery q(db);
    QStringList list; int count;
    q.prepare("SELECT ip_address, port, slave_id, function_code, start_address, num_of_coils FROM config where id_config = 1 and status = 1");
    if(!q.exec()){
        return list;
    }
    else{
        do {
            for (int i = 0; i < 6; i++) {
                if (!q.isNull(i)) {
                    list.insert(count, q.value(i).toString());
                    count++;
                }
            }
        } while(q.next());
    }
    db.close();
    return list;
}
