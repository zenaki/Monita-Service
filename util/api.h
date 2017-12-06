#ifndef API_H
#define API_H

#include <action.h>

class api : public qttp::Action
{
public:
    QStringList API;
    api();

    const char* getName() const
    {
      return "desktop-action";
    }

    void onGet(qttp::HttpData& data);

//signals:
//    void getParameter(QStringList data);
};

#endif // API_H
