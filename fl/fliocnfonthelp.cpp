#include <QLoggingCategory>
#include <QPainter>
#include <QFontMetrics>
#include <QFontDatabase>

#include "fliocnfonthelp.h"

const QString awesomFontPath = ":/awesome-font/awesomFont/fontawesom-free-5.14.0-desktop/otfs/Font Awesome 5 Free-Solid-900.otf";

FLIocnfontHelp *FLIocnfontHelp::instance()
{
    static FLIocnfontHelp iconfont;

    return &iconfont;
}

QFont FLIocnfontHelp::font()
{
    return m_font;
}

void FLIocnfontHelp::GetIcon(QIcon &icon, const QString &name, int pointSize,
                              QIcon::Mode mode, QIcon::State state)
{
    QPixmap pix;

    GetPix(pix, name, pointSize);
    icon.addPixmap(pix, mode, state);
}

void FLIocnfontHelp::GetPix(QPixmap &pix, const QString &name, int pointSize)
{
    if(!m_mapIcon.count(name))
        return;

    QPainter painter(&pix);
    QFont ft(m_font);
    ft.setPointSize(pointSize);

    painter.setFont(ft);
    QFontMetrics fm(ft);

    int pixelsWide = fm.horizontalAdvance(m_mapIcon[name]);
    int pixelsHigh = fm.height();
    painter.drawText(0, 0, pixelsWide, pixelsHigh, Qt::AlignCenter, m_mapIcon[name]);
}

QChar FLIocnfontHelp::GetChar(const QString &name)
{
    if(!m_mapIcon.count(name))
        return *name.data();
    return m_mapIcon[name];
}

bool FLIocnfontHelp::Init()
{
    return initIconMap() && loadAwesomeFont();
}

bool FLIocnfontHelp::loadAwesomeFont()
{
    int nIndex = QFontDatabase::addApplicationFont(awesomFontPath);
    if (nIndex != -1)
    {
        QStringList strList(QFontDatabase::applicationFontFamilies(nIndex));
        if (strList.count() > 0)
        {
            m_font.setPointSize(20);
            m_font = strList.at(0);
            return true;
        }
    }
    return false;
}

bool FLIocnfontHelp::initIconMap()
{
    m_mapIcon = {
        {"ad", 0xf641}, {"address-book", 0xf2b9}, {"address-card", 0xf2bb}, {"ad", 0xf042},
        {"air-freshener", 0xf5d0}, {"align-center", 0xf037}, {"align-justify", 0xf039}, {"align-left", 0xf036},
        {"align-right", 0x038}, {"allergies", 0xf461}, {"ambulance", 0x0f9}, {"american-sign-language-interpreting", 0xf2a3},
        {"anchor", 0xf13d}, {"angle-double-down", 0xf103}, {"aangle-double-left", 0xf100}, {"angle-double-right", 0xf101},
        {"window-close", 0xf410}, {"window-maximize", 0xf2d0}, {"window-minimize", 0xf2d1}, {"window-restore", 0xf2d2},
    };



    return true;
}

FLIocnfontHelp::FLIocnfontHelp() :
    m_vaild(false)
{
    m_vaild = Init();
    if(!m_vaild)
        qWarning() << "FLT icon font initial fail";
}
