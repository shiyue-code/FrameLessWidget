#ifndef FLIOCNFONTHELP_H
#define FLIOCNFONTHELP_H

/**
 * fontawesome class
 **/
#include <QMap>
#include <QString>
#include <QIcon>
#include <QFont>

#define FLIcon FLIocnfontHelp::instance()

class FLIocnfontHelp
{
public:

    static FLIocnfontHelp *instance();

    QFont font();

    void GetIcon(QIcon &icon, const QString &name, int pointSize,
                 QIcon::Mode mode, QIcon::State state);
    void GetPix(QPixmap &icon, const QString &name, int pointSize);
    QChar GetChar(const QString &name);

private:
    bool Init();
    bool loadAwesomeFont();
    bool initIconMap();

private:
    FLIocnfontHelp();
    ~FLIocnfontHelp()=default;

    FLIocnfontHelp(const FLIocnfontHelp&)=delete;
    FLIocnfontHelp(FLIocnfontHelp &&)=delete;
    FLIocnfontHelp & operator= (const FLIocnfontHelp&)=delete;

private:
    QFont m_font;
    QMap<QString, QChar> m_mapIcon;
    bool m_vaild;
};

#endif // FLIOCNFONTHELP_H
